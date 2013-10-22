#include "layer_api.h"
#include "http_layer.h"
#include "http_layer_constants.h"
#include "http_layer_input.h"
#include "http_layer_data.h"
#include "xi_macros.h"
#include "xi_debug.h"
#include "common.h"
#include "xi_stated_sscanf.h"
#include "xi_coroutine.h"
#include "layer_helpers.h"


// static array of recognizable http headers
static const char* XI_HTTP_TOKEN_NAMES[ XI_HTTP_HEADERS_COUNT ] =
    {
          "date"            // XI_HTTP_HEADER_DATE
        , "content-type"    // XI_HTTP_HEADER_CONTENT_TYPE
        , "content-length"  // XI_HTTP_HEADER_CONTENT_LENGTH
        , "connection"      // XI_HTTP_HEADER_CONNECTION
        , "x-request-id"    // XI_HTTP_HEADER_X_REQUEST_ID
        , "cache-control"   // XI_HTTP_HEADER_CACHE_CONTROL
        , "age"             // XI_HTTP_HEADER_AGE
        , "vary"            // XI_HTTP_HEADER_VARY
        , "unknown"         // XI_HTTP_HEADER_UNKNOWN, //!< !!!! this must be always on the last position
    };

/**
 * \brief   classify_header function that finds a proper type for a given http header name
 * \param   header header name to be classified
 * \return  classified http header type
 */
static inline http_header_type_t classify_header( const char* header )
{
    for( unsigned short i = 0; i < XI_HTTP_HEADER_COUNT - 1; ++i )
    {
        if( strcasecmp( header, XI_HTTP_TOKEN_NAMES[ i ] ) == 0 )
            return ( http_header_type_t ) i;
    }

    return XI_HTTP_HEADER_UNKNOWN;
}

/**
 * \brief http_layer_data_generator_datastream_get
 * \param input
 * \param state
 * \return
 */
const void* http_layer_data_generator_datastream_get(
          const void* input
        , short* state )
{
    // unpack the data
    const http_layer_input_t* const http_layer_input = ( const http_layer_input_t* const ) input;

    ENABLE_GENERATOR();

    BEGIN_CORO( *state )

        gen_ptr_text( *state, XI_HTTP_GET );
        gen_ptr_text( *state, XI_HTTP_TEMPLATE_FEED );
        gen_static_text( *state, "/" );

        {
            static char buff[ 16 ];
            sprintf( buff, "%d", http_layer_input->xi_context->feed_id );

            gen_ptr_text( *state, buff ); // feed id
        }

        gen_static_text( *state, "/datastreams/" );
        gen_ptr_text( *state, http_layer_input->http_layer_data_u.xi_get_datastream.datastream ); // datastream id
        gen_static_text( *state, ".csv " );

        // SEND HTTP
        gen_ptr_text( *state, XI_HTTP_TEMPLATE_HTTP );
        gen_ptr_text( *state, XI_HTTP_CRLF );

        // SEND HOST
        gen_ptr_text( *state, XI_HTTP_TEMPLATE_HOST );
        gen_static_text( *state, "api.xively.com" );
        gen_ptr_text( *state, XI_HTTP_CRLF );

        // SEND USER AGENT
        gen_ptr_text( *state, XI_HTTP_TEMPLATE_USER_AGENT );
        gen_static_text( *state, "libxively-posix/experimental" );
        gen_ptr_text( *state, XI_HTTP_CRLF );

        // SEND ACCEPT
        gen_ptr_text( *state, XI_HTTP_TEMPLATE_ACCEPT );
        gen_ptr_text( *state, XI_HTTP_CRLF );

        // A API KEY
        gen_ptr_text( *state, XI_HTTP_TEMPLATE_X_API_KEY );
        gen_ptr_text( *state, http_layer_input->xi_context->api_key ); // api key
        gen_ptr_text( *state, XI_HTTP_CRLF );

        // the end, no more data double crlf
        gen_ptr_text_and_exit( *state, XI_HTTP_CRLF );

    END_CORO()
}

/**
 * \brief  see the layer_interface for details
 */
static inline layer_state_t http_layer_data_ready_datastream_get(
      layer_connectivity_t* context
    , const http_layer_input_t* input )
{
    // generator state
    short gstate = 0;

    // send the data through the next layer
    while( gstate != 1 )
    {
        const const_data_descriptor_t* ret
                = ( const const_data_descriptor_t* ) http_layer_data_generator_datastream_get( input, &gstate );

        layer_state_t state
                = CALL_ON_PREV_DATA_READY(
                      context->self
                    , ( const void* ) ret
                    , ( gstate != 1 ) ? LAYER_HINT_MORE_DATA : LAYER_HINT_NONE );

        if( state != LAYER_STATE_OK )
        {
            return state;
        }
    }

    return LAYER_STATE_OK;
}

/**
 * \brief   see the layer_interface for details
 */
layer_state_t http_layer_data_ready(
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
            return http_layer_data_ready_datastream_get(
                          context, http_layer_input );
        default:
            return LAYER_STATE_ERROR;
    };

    return LAYER_STATE_ERROR;
}



/**
 * \brief  see the layer_interface for details
 */
layer_state_t http_layer_on_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint )
{

    XI_UNUSED( hint );

    // unpack http_layer_data so unpack it
    http_layer_data_t* http_layer_data = ( http_layer_data_t* ) context->self->user_data;

    // some tmp variables
    short sscanf_state     = 0;

    xi_stated_sscanf_state_t* xi_stated_state = &http_layer_data->xi_stated_sscanf_state;

    BEGIN_CORO( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ] )

    memset( xi_stated_state, 0, sizeof( xi_stated_sscanf_state_t ) );

    // STAGE 01 find the http status
    {
        //
        {
            sscanf_state                          = 0;

            while( sscanf_state == 0 )
            {
                const char status_pattern[]       = "HTTP/1.1 %d %" XI_STR( XI_HTTP_STATUS_STRING_SIZE ) "s\r\n";
                const const_data_descriptor_t v   = { status_pattern, sizeof( status_pattern ), sizeof( status_pattern ) };
                void* pv[]                  = { ( void* ) &( http_layer_data->response->http.http_status ), ( void* ) http_layer_data->response->http.http_status_string };

                sscanf_state = xi_stated_sscanf(
                              xi_stated_state
                            , ( const_data_descriptor_t* ) &v
                            , ( const_data_descriptor_t* ) data
                            , pv );

                if( sscanf_state == 0 )
                {
                    YIELD( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ], LAYER_STATE_MORE_DATA )
                    continue;
                }
            }

            if( sscanf_state == -1 )
            {
                EXIT( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ], LAYER_STATE_ERROR )
            }
        }
    }    

    // STAGE 02 reading headers
    {
        do
        {
            sscanf_state = 0;

            while( sscanf_state == 0 )
            {
                const char status_pattern[]       = "%" XI_STR( XI_HTTP_HEADER_NAME_MAX_SIZE ) "s: %" XI_STR( XI_HTTP_HEADER_VALUE_MAX_SIZE ) ".\r\n";
                const const_data_descriptor_t v   = { status_pattern, sizeof( status_pattern ), sizeof( status_pattern ) };
                void*                  pv[]       =
                {
                      ( void* ) http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].name
                    , ( void* ) http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].value
                };

                sscanf_state = xi_stated_sscanf(
                              xi_stated_state
                            , ( const_data_descriptor_t* ) &v
                            , ( const_data_descriptor_t* ) data
                            , pv );

                if( sscanf_state == 0 )
                {
                    YIELD( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ], LAYER_STATE_MORE_DATA )
                    continue;
                }
            }

            if( sscanf_state == 1 )
            {
                xi_debug_printf( "%s: %s\n"
                                 , http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].name
                                 , http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].value );

                http_header_type_t header_type = classify_header( http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].name );

                if( header_type == XI_HTTP_HEADER_CONTENT_LENGTH )
                {
                    xi_stated_sscanf_state_t tmp_state;
                    memset( &tmp_state, 0, sizeof( xi_stated_sscanf_state_t ) );

                    const char tmp_status_pattern[]           = "%d";
                    const const_data_descriptor_t tmp_v       = { tmp_status_pattern, sizeof( tmp_status_pattern ), sizeof( tmp_status_pattern ) };
                    const const_data_descriptor_t tmp_data    =
                    {
                          http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].value
                        , XI_HTTP_HEADER_VALUE_MAX_SIZE
                        , XI_HTTP_HEADER_VALUE_MAX_SIZE
                    };

                    void*                   tmp_pv[]    = { ( void* ) &http_layer_data->content_length };

                    sscanf_state = xi_stated_sscanf( &tmp_state, &tmp_v, &tmp_data, tmp_pv );

                    if( sscanf_state == -1 )
                    {
                        EXIT( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ], LAYER_STATE_ERROR )
                    }
                }

                if( header_type != XI_HTTP_HEADER_UNKNOWN )
                {
                    memcpy( http_layer_data->response->http.http_headers[ header_type ].name
                          , http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].name
                          , XI_HTTP_HEADER_NAME_MAX_SIZE );

                    memcpy( http_layer_data->response->http.http_headers[ header_type ].value
                          , http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].value
                          , XI_HTTP_HEADER_VALUE_MAX_SIZE );
                }

                http_layer_data->response->http.http_headers_checklist[ header_type ]
                        = &http_layer_data->response->http.http_headers[ header_type ];
            }

        } while( sscanf_state == 1 );
    }

    if( sscanf_state == -1 )
    {
        EXIT( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ], LAYER_STATE_ERROR )
    }

    // STAGE 03 reading second \r\n that means that the payload should begin just right after
    {
        sscanf_state                = 0;

        while( sscanf_state == 0 )
        {
            const char status_pattern[]       = "\r\n";
            const const_data_descriptor_t v   = { status_pattern, sizeof( status_pattern ), sizeof( status_pattern ) };

            sscanf_state = xi_stated_sscanf(
                          xi_stated_state
                        , ( const_data_descriptor_t* ) &v
                        , ( const_data_descriptor_t* ) data
                        , 0 );

            if( sscanf_state == 0 )
            {
                YIELD( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ], LAYER_STATE_MORE_DATA )
                continue;
            }
        }
    }

    if( sscanf_state == -1 )
    {
        EXIT( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ], LAYER_STATE_ERROR )
    }

    // STAGE 04 reading payload
    {
        // clear the buffer
        memset( http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].value, 0, XI_HTTP_HEADER_VALUE_MAX_SIZE );
        xi_debug_printf( "\n" );

        http_layer_data->counter    = 0;
        sscanf_state                = 0;

        while( sscanf_state == 0 )
        {
            short before = xi_stated_state->i;

            const char status_pattern[]       = "%B";
            const const_data_descriptor_t v   = { status_pattern, sizeof( status_pattern ), sizeof( status_pattern ) };
            void*                        pv[] = { ( void* ) http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].value };

            sscanf_state = xi_stated_sscanf(
                          xi_stated_state
                        , ( const_data_descriptor_t* ) &v
                        , ( const_data_descriptor_t* ) data
                        , pv );

            short after = xi_stated_state->i;

            http_layer_data->counter += ( after - before );

            if( http_layer_data->content_length == http_layer_data->counter )
            {
                xi_debug_printf( "%s\n", http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].value );
                EXIT( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ], LAYER_STATE_OK )
            }

            if( sscanf_state == 0 )
            {
                YIELD( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ], LAYER_STATE_MORE_DATA )
                xi_stated_state->tmp_i  = 0; // reset the value pointer
                xi_stated_state->i      = 0; // reset the source pointer
                xi_debug_printf( "%s", http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].value );
                memset( http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].value, 0, XI_HTTP_HEADER_VALUE_MAX_SIZE );
            }
        }
    }

    if( sscanf_state == -1 )
    {
        EXIT( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ], LAYER_STATE_ERROR )
    }

    EXIT( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ], LAYER_STATE_OK )

    END_CORO()

    return LAYER_STATE_ERROR;
}


/**
 * \brief  see the layer_interface for details
 */
layer_state_t http_layer_close(
    layer_connectivity_t* context )
{
    XI_UNUSED( context );

    return LAYER_STATE_OK;
}

/**
 * \brief  see the layer_interface for details
 */
layer_state_t http_layer_on_close(
    layer_connectivity_t* context )
{
    XI_UNUSED( context );

    return LAYER_STATE_OK;
}


/**
 * \brief connect_to_endpoint
 * \param layer
 * \return
 */
layer_t* init_http_layer(
      layer_t* layer )
{
    XI_UNUSED( layer );

    return LAYER_STATE_OK;
}
