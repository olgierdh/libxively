#include "csv_layer.h"
#include "xively.h"

// xi
#include "xi_macros.h"
#include "xi_debug.h"
#include "xi_coroutine.h"
#include "xi_generator.h"
#include "xi_stated_csv_decode_value_state.h"

#include "csv_layer_data.h"
#include "layer_api.h"
#include "http_layer_input.h"
#include "layer_helpers.h"


/** \brief holds pattern for parsing and constructing timestamp */
static const char* const CSV_TIMESTAMP_PATTERN = "%04d-%02d-%02dT%02d:%02d:%02d.%06dZ";

//

inline static int csv_encode_value(
      char* buffer
    , size_t buffer_size
    , const xi_datapoint_t* p )
{
    // PRECONDITION
    assert( buffer != 0 );
    assert( buffer_size != 0 );
    assert( p != 0 );

    switch( p->value_type )
    {
        case XI_VALUE_TYPE_I32:
            return snprintf( buffer, buffer_size, "%d", p->value.i32_value );
        case XI_VALUE_TYPE_F32:
            return snprintf( buffer, buffer_size, "%f", p->value.f32_value );
        case XI_VALUE_TYPE_STR:
            return snprintf( buffer, buffer_size, "%s", p->value.str_value );
        default:
            return -1;
    }
}

typedef enum
{
    XI_STATE_INITIAL = 0,
    XI_STATE_MINUS,
    XI_STATE_NUMBER,
    XI_STATE_FLOAT,
    XI_STATE_DOT,
    XI_STATE_STRING,
    XI_STATES_NO
} xi_dfa_state_t;

typedef enum
{
    XI_CHAR_UNKNOWN = 0,
    XI_CHAR_NUMBER,
    XI_CHAR_LETTER,
    XI_CHAR_DOT,
    XI_CHAR_SPACE,
    XI_CHAR_NEWLINE,
    XI_CHAR_TAB,
    XI_CHAR_MINUS,
    XI_CHARS_NO
} xi_char_type_t;

inline static xi_char_type_t csv_classify_char( char c )
{
    switch( c )
    {
        case 13:
        case 11:
            return XI_CHAR_NEWLINE;
        case 9:
            return XI_CHAR_TAB;
        case 32:
            return XI_CHAR_SPACE;
        case 33: case 34: case 35: case 36: case 37: case 38: case 39:
        case 40: case 41: case 42: case 43: case 44:
            return XI_CHAR_UNKNOWN;
        case 45:
            return XI_CHAR_MINUS;
        case 46:
            return XI_CHAR_DOT;
        case 47:
            return XI_CHAR_UNKNOWN;
        case 48: case 49: case 50: case 51: case 52: case 53: case 54:
        case 55: case 56:
        case 57:
            return XI_CHAR_NUMBER;
        case 58: case 59: case 60: case 61: case 62: case 63:
        case 64:
            return XI_CHAR_UNKNOWN;
        case 65: case 66: case 67: case 68: case 69: case 70: case 71:
        case 72: case 73: case 74: case 75: case 76: case 77: case 78:
        case 79: case 80: case 81: case 82: case 83: case 84: case 85:
        case 86: case 87: case 88: case 89:
        case 90:
            return XI_CHAR_LETTER;
        case 91: case 92: case 93: case 94: case 95:
        case 96:
            return XI_CHAR_UNKNOWN;
        case 97: case 98: case 99: case 100: case 101: case 102: case 103:
        case 104: case 105: case 106: case 107: case 108: case 109: case 110:
        case 111: case 112: case 113: case 114: case 115: case 116: case 117:
        case 118: case 119: case 120: case 121:
        case 122:
            return XI_CHAR_LETTER;
        case 123:
        case 124:
        case 125:
            return XI_CHAR_UNKNOWN;
        default:
            return XI_CHAR_UNKNOWN;
    }
}

// the transition function
static const short states[][6][2] =
{
      // state initial                          // state minus                            // state number                           // state float                            // state dot                              // string
    { { XI_CHAR_UNKNOWN   , XI_STATE_STRING  }, { XI_CHAR_UNKNOWN   , XI_STATE_STRING  }, { XI_CHAR_UNKNOWN   , XI_STATE_STRING  }, { XI_CHAR_UNKNOWN   , XI_STATE_STRING  }, { XI_CHAR_UNKNOWN   , XI_STATE_STRING  }, { XI_CHAR_UNKNOWN   , XI_STATE_STRING  } },
    { { XI_CHAR_NUMBER    , XI_STATE_NUMBER  }, { XI_CHAR_NUMBER    , XI_STATE_NUMBER  }, { XI_CHAR_NUMBER    , XI_STATE_NUMBER  }, { XI_CHAR_NUMBER    , XI_STATE_FLOAT   }, { XI_CHAR_NUMBER    , XI_STATE_FLOAT   }, { XI_CHAR_NUMBER    , XI_STATE_STRING  } },
    { { XI_CHAR_LETTER    , XI_STATE_STRING  }, { XI_CHAR_LETTER    , XI_STATE_STRING  }, { XI_CHAR_LETTER    , XI_STATE_STRING  }, { XI_CHAR_LETTER    , XI_STATE_STRING  }, { XI_CHAR_LETTER    , XI_STATE_STRING  }, { XI_CHAR_LETTER    , XI_STATE_STRING  } },
    { { XI_CHAR_DOT       , XI_STATE_DOT     }, { XI_CHAR_DOT       , XI_STATE_DOT     }, { XI_CHAR_DOT       , XI_STATE_DOT     }, { XI_CHAR_DOT       , XI_STATE_STRING  }, { XI_CHAR_DOT       , XI_STATE_STRING  }, { XI_CHAR_DOT       , XI_STATE_STRING  } },
    { { XI_CHAR_SPACE     , XI_STATE_STRING  }, { XI_CHAR_SPACE     , XI_STATE_STRING  }, { XI_CHAR_SPACE     , XI_STATE_STRING  }, { XI_CHAR_SPACE     , XI_STATE_STRING  }, { XI_CHAR_SPACE     , XI_STATE_STRING  }, { XI_CHAR_SPACE     , XI_STATE_STRING  } },
    { { XI_CHAR_NEWLINE   , XI_STATE_INITIAL }, { XI_CHAR_NEWLINE   , XI_STATE_INITIAL }, { XI_CHAR_NEWLINE   , XI_STATE_INITIAL }, { XI_CHAR_NEWLINE   , XI_STATE_INITIAL }, { XI_CHAR_NEWLINE   , XI_STATE_INITIAL }, { XI_CHAR_NEWLINE   , XI_STATE_INITIAL } },
    { { XI_CHAR_TAB       , XI_STATE_STRING  }, { XI_CHAR_TAB       , XI_STATE_STRING  }, { XI_CHAR_TAB       , XI_STATE_STRING  }, { XI_CHAR_TAB       , XI_STATE_STRING  }, { XI_CHAR_TAB       , XI_STATE_STRING  }, { XI_CHAR_TAB       , XI_STATE_STRING  } },
    { { XI_CHAR_MINUS     , XI_STATE_MINUS   }, { XI_CHAR_MINUS     , XI_STATE_STRING  }, { XI_CHAR_MINUS     , XI_STATE_STRING  }, { XI_CHAR_MINUS     , XI_STATE_STRING  }, { XI_CHAR_MINUS     , XI_STATE_STRING  }, { XI_CHAR_MINUS     , XI_STATE_STRING  } }
};

// @TODO
// static const short accepting_states[] = { XI_STATE_ };

char xi_stated_csv_decode_value(
          xi_stated_csv_decode_value_state_t* st
        , const const_data_descriptor_t* source
        , xi_datapoint_t* p
        , layer_hint_t hint )
{
    // unused
    XI_UNUSED( hint );

    // PRECONDITION
    assert( st != 0 );
    assert( source != 0 );
    assert( p != 0 );

    // if not the first run jump into the proper label
    if( st->state != XI_STATE_INITIAL )
    {
        goto data_ready;
    }

    // secure the output buffer
    XI_GUARD_EOS( p->value.str_value, XI_VALUE_STRING_MAX_SIZE );

    // clean the counter
    st->counter = 0;

    char    c   = source->data_ptr[ ( st->sp )++ ];
    st->state   = XI_STATE_INITIAL;

    // main processing loop
    while( c != '\n' && c !='\0' && c!='\r' )
    {
        if( st->counter >= XI_VALUE_STRING_MAX_SIZE - 1 )
        {
            xi_set_err( XI_DATAPOINT_VALUE_BUFFER_OVERFLOW );
            return 0;
        }

        xi_char_type_t ct = csv_classify_char( c );
        st->state = states[ ct ][ st->state ][ 1 ];

        switch( st->state )
        {
            case XI_STATE_MINUS:
            case XI_STATE_NUMBER:
            case XI_STATE_FLOAT:
            case XI_STATE_DOT:
            case XI_STATE_STRING:
                p->value.str_value[ st->counter ] = c;
                break;
        }

        // this is where we shall need to jump for more data
        if( st->sp == source->real_size )
        {
            // need more data
            return 0;
data_ready:
            st->sp = 0; // reset the counter
        }

        //
        ++( st->counter );
        c = source->data_ptr[ ( st->sp )++ ];
    }

    // set the guard
    p->value.str_value[ st->counter ] = '\0';

    // update of the state for loose states...
    switch( st->state )
    {
        case XI_STATE_MINUS:
        case XI_STATE_DOT:
        case XI_STATE_INITIAL:
            st->state = XI_STATE_STRING;
            break;
    }

    switch( st->state )
    {
        case XI_STATE_NUMBER:
            p->value.i32_value  = atoi( p->value.str_value );
            p->value_type       = XI_VALUE_TYPE_I32;
            break;
        case XI_STATE_FLOAT:
            p->value.f32_value  = ( float ) atof( p->value.str_value );
            p->value_type       = XI_VALUE_TYPE_F32;
            break;
        case XI_STATE_STRING:
        default:
            p->value_type       = XI_VALUE_TYPE_STR;
    }

    return 1;
}

/**
 * @brief csv_layer_data_generator_datapoint generates the data related to the datapoint
 * @param input
 * @param state
 * @return
 */
const void* csv_layer_data_generator_datapoint(
          const void* input
        , short* state )
{
    // we expect input to be datapoint
    const xi_datapoint_t* dp = ( xi_datapoint_t* ) input;

    ENABLE_GENERATOR();
    BEGIN_CORO( *state )

        // if there is a timestamp encode it
        if( dp->timestamp.timestamp != 0 )
        {
            xi_time_t stamp = dp->timestamp.timestamp;
            struct xi_tm* gmtinfo = xi_gmtime( &stamp );

            static char buffer[ 32 ] = { '\0' };

            snprintf( buffer, 32
                , CSV_TIMESTAMP_PATTERN
                , gmtinfo->tm_year + 1900
                , gmtinfo->tm_mon + 1
                , gmtinfo->tm_mday
                , gmtinfo->tm_hour
                , gmtinfo->tm_min
                , gmtinfo->tm_sec
                , ( int ) dp->timestamp.micro );

            gen_ptr_text( *state, buffer );
            gen_static_text( *state, "," );
        }

        // value
        {
            static char buffer[ 32 ] = { '\0' };
            csv_encode_value( buffer, sizeof( buffer ), dp );
            gen_ptr_text( *state, buffer );
        }

        // end of line
        gen_static_text_and_exit( *state, "\n" );

    END_CORO()
}

/**
 * @brief csv_layer_data_generator_datastream_update
 * @param input
 * @param state
 * @return
 */
const void* csv_layer_data_generator_datastream_update(
          const void* input
        , short* state )
{
    XI_UNUSED( input );

    ENABLE_GENERATOR();
    BEGIN_CORO( *state )

        gen_static_text( *state, "sub_test0" );
        gen_static_text( *state, "sub_test1" );
        gen_static_text( *state, "sub_test2" );
        gen_static_text( *state, "sub_test3" );

        call_sub_gen( *state, input, csv_layer_data_generator_datapoint );

        gen_static_text( *state, "sub_test4" );
        gen_static_text_and_exit( *state, "sub_test5" );

    END_CORO()
}


/**
 * \brief  see the layer_interface for details
 */
const void* csv_layer_data_generator_datastream_get(
          const void* input
        , short* state )
{
    XI_UNUSED( input );

    ENABLE_GENERATOR();
    BEGIN_CORO( *state )

        gen_static_text( *state, "test0" );
        gen_static_text( *state, "test1" );
        gen_static_text( *state, "test2" );

        call_sub_gen( *state, input, csv_layer_data_generator_datastream_update );

        gen_static_text( *state, "test3" );
        gen_static_text( *state, "test4" );
        gen_static_text_and_exit( *state, "test5" );

    END_CORO()
}


/**
 * @brief csv_layer_parse_datastream helper function that parses the one level of the data which is the datastream itself
 *        this function suppose to parse the timestamp and the value and save it within the proper datastream field
 *
 * @param context
 * @param data
 * @param hint
 * @return
 */
layer_state_t csv_layer_parse_datastream(
        layer_connectivity_t* context
      , const_data_descriptor_t* data
      , const layer_hint_t hint
      , xi_datastream_t* datastream )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( hint );
    XI_UNUSED( datastream );

    // parse the timestamp
    {

    }


    // parse the value
    {

    }

    return LAYER_STATE_OK;
}


layer_state_t csv_layer_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( hint );

    // unpack the layer data
    csv_layer_data_t* csv_layer_data = ( csv_layer_data_t* ) context->self->user_data;

    // BEGIN_CORO( context->self->layer_states[ FUNCTION_ID_DATA_READY ] )

    //
    switch( csv_layer_data->http_layer_input->query_type )
    {
        case HTTP_LAYER_INPUT_DATASTREAM_GET:
            break;
        default:
            break;
    }

    return LAYER_STATE_OK;

    // END_CORO()

    return LAYER_STATE_OK;
}

layer_state_t csv_layer_on_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( hint );

    // unpack the data, changing the constiness to avoid copying cause
    // these layers shares the same data and the generator suppose to be the only
    // field that set is required
    http_layer_input_t* http_layer_input = ( http_layer_input_t* ) ( data );

    // store the layer input in custom data
    ( ( csv_layer_data_t* ) context->self->user_data )->http_layer_input = ( void* ) http_layer_input;

    switch( http_layer_input->query_type )
    {
        case HTTP_LAYER_INPUT_DATASTREAM_GET:
        {
            http_layer_input->payload_generator = 0;
            return CALL_ON_PREV_DATA_READY( context->self, ( void* ) http_layer_input, hint );
        }
        default:
            return LAYER_STATE_ERROR;
    };

    return LAYER_STATE_ERROR;
}

layer_state_t csv_layer_close(
    layer_connectivity_t* context )
{
    XI_UNUSED( context );

    return LAYER_STATE_OK;
}

layer_state_t csv_layer_on_close(
    layer_connectivity_t* context )
{
    XI_UNUSED( context );

    return LAYER_STATE_OK;
}
