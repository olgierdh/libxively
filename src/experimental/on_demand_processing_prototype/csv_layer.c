#include "csv_layer.h"
#include "xi_macros.h"
#include "xi_debug.h"
#include "xively.h"
#include "xi_stated_csv_decode_value_state.h"
#include "xi_coroutine.h"
#include "xi_generator.h"
#include "layer_api.h"
#include "http_layer_input.h"
#include "layer_helpers.h"

typedef enum
{
    XI_STATE_INITIAL = 0,
    XI_STATE_MINUS,
    XI_STATE_NUMBER,
    XI_STATE_FLOAT,
    XI_STATE_DOT,
    XI_STATE_STRING
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
    XI_CHAR_MINUS
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
static const short const states[][6][2] =
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

xi_datapoint_t* xi_stated_csv_decode_value(
    xi_stated_csv_decode_value_state_t* st, const char* buffer, xi_datapoint_t* p )
{
    XI_UNUSED( st );

    // PRECONDITION
    assert( buffer != 0 );
    assert( p != 0 );

    // secure the output buffer
    XI_GUARD_EOS( p->value.str_value, XI_VALUE_STRING_MAX_SIZE );

    // clean the counter
    size_t  counter = 0;

    char    c = *buffer;
    short   s = XI_STATE_INITIAL;

    while( c != '\n' && c !='\0' && c!='\r' )
    {
        if( counter >= XI_VALUE_STRING_MAX_SIZE - 1 )
        {
            xi_set_err( XI_DATAPOINT_VALUE_BUFFER_OVERFLOW );
            return 0;
        }

        xi_char_type_t ct = csv_classify_char( c );
        s = states[ ct ][ s ][ 1 ];

        switch( s )
        {
            case XI_STATE_MINUS:
            case XI_STATE_NUMBER:
            case XI_STATE_FLOAT:
            case XI_STATE_DOT:
            case XI_STATE_STRING:
                p->value.str_value[ counter ] = c;
                break;
        }

        c = *( ++buffer );
        ++counter;
    }

    // set the guard
    p->value.str_value[ counter ] = '\0';

    // update of the state for loose states...
    switch( s )
    {
        case XI_STATE_MINUS:
        case XI_STATE_DOT:
        case XI_STATE_INITIAL:
            s = XI_STATE_STRING;
            break;
    }

    switch( s )
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

    return p;
}


layer_state_t csv_layer_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( hint );

    // later aligator

    return LAYER_STATE_OK;
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
        gen_static_text( *state, "test3" );
        gen_static_text( *state, "test4" );
        gen_static_text_and_exit( *state, "test5" );

    END_CORO()
}

layer_state_t csv_layer_on_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( hint );

    // unpack the data
    const http_layer_input_t* http_layer_input = ( const http_layer_input_t* ) data;

    switch( http_layer_input->query_type )
    {
        case HTTP_LAYER_INPUT_DATASTREAM_GET:
        {
            http_layer_input_t tmp;
            memcpy( &tmp, http_layer_input, sizeof( http_layer_input_t ) );
            tmp.payload_generator = 0;
            return CALL_ON_PREV_DATA_READY( context->self, ( void* ) &tmp, hint );
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
