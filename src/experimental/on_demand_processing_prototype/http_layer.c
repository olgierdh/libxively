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
 * \brief layer_sender little helper to encapsulate
 *        error handling over sending data between layers
 */
#define layer_sender( context, data, hint ) \
{ \
    const const_data_descriptor_t tmp_data = { (data), strlen( (data) ), 0 }; \
    layer_state_t layer_state = CALL_ON_PREV_DATA_READY( context->self, ( const void* ) &tmp_data, hint ); \
    switch( layer_state ) \
    { \
        case LAYER_STATE_OK: \
            break; \
        default: \
            return layer_state; \
    } \
}

/**
 * \brief  see the layer_interface for details
 */
static inline layer_state_t http_layer_data_ready_datastream_get(
      layer_connectivity_t* context
    , const int feed_id
    , const char* datastream_id
    , const char* api_key )
{
    // buffer that is locally used to encode the data to be sent over the layers system
    char buff[ 16 ];

    // SEND TYPE
    layer_sender( context, XI_HTTP_GET, LAYER_HINT_MORE_DATA );

    // SEND ADDRESS
    layer_sender( context, XI_HTTP_TEMPLATE_FEED, LAYER_HINT_MORE_DATA );
    layer_sender( context, "/", LAYER_HINT_MORE_DATA );

    memset( buff, 0, sizeof( buff ) );
    sprintf( buff, "%d", feed_id );

    layer_sender( context, buff, LAYER_HINT_MORE_DATA ); // feed id
    layer_sender( context, "/datastreams/", LAYER_HINT_MORE_DATA );
    layer_sender( context, datastream_id, LAYER_HINT_MORE_DATA ); // datastream id
    layer_sender( context, ".csv ", LAYER_HINT_MORE_DATA );

    // SEND HTTP
    layer_sender( context, XI_HTTP_TEMPLATE_HTTP, LAYER_HINT_MORE_DATA );
    layer_sender( context, XI_HTTP_CRLF, LAYER_HINT_MORE_DATA );

    // SEND HOST
    layer_sender( context, XI_HTTP_TEMPLATE_HOST, LAYER_HINT_MORE_DATA );
    layer_sender( context, "api.xively.com", LAYER_HINT_MORE_DATA );
    layer_sender( context, XI_HTTP_CRLF, LAYER_HINT_MORE_DATA );

    // SEND USER AGENT
    layer_sender( context, XI_HTTP_TEMPLATE_USER_AGENT, LAYER_HINT_MORE_DATA );
    layer_sender( context, "libxively-posix/experimental", LAYER_HINT_MORE_DATA );
    layer_sender( context, XI_HTTP_CRLF, LAYER_HINT_MORE_DATA );

    // SEND ACCEPT
    layer_sender( context, XI_HTTP_TEMPLATE_ACCEPT, LAYER_HINT_MORE_DATA );
    layer_sender( context, XI_HTTP_CRLF, LAYER_HINT_MORE_DATA );

    // A API KEY
    layer_sender( context, XI_HTTP_TEMPLATE_X_API_KEY, LAYER_HINT_MORE_DATA );
    layer_sender( context, api_key, LAYER_HINT_MORE_DATA ); // api key
    layer_sender( context, XI_HTTP_CRLF, LAYER_HINT_MORE_DATA );

    // the end, no more data double crlf
    layer_sender( context, XI_HTTP_CRLF, LAYER_HINT_NONE );

    return LAYER_STATE_OK;
}

/**
 * \brief http_layer_on_data_ready_http_parse
 * \param context
 * \param data
 * \param hint
 * \return
 */
static inline layer_state_t http_layer_on_data_ready_http_parse(
        layer_connectivity_t* context
      , const void* data
      , const layer_hint_t hint )
{
    XI_UNUSED( hint );
    XI_UNUSED( context );

    // expecting data buffer so unpack it
    const const_data_descriptor_t* data_description = ( const const_data_descriptor_t* ) data;
    // http_layer_data_t* http_data                    = ( http_layer_data_t* ) context->self->user_data;

    for( int i = 0; i < data_description->hint_size; ++i )
    {
        putchar( data_description->data_ptr[ i ] );
        fflush( stdout );
    }

    return LAYER_STATE_MORE_DATA;
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
                          context, http_layer_input->xi_context->feed_id
                        , http_layer_input->http_layer_data_u.xi_get_datastream.datastream
                        , http_layer_input->xi_context->api_key );
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
            sscanf_state                = 0;

            while( sscanf_state == 0 )
            {
                char status_pattern[]       = "HTTP/1.1 %d %" XI_STR( XI_HTTP_STATUS_STRING_SIZE ) "s\r\n";
                const_data_descriptor_t v   = { status_pattern, sizeof( status_pattern ), sizeof( status_pattern ) };
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

                if( memcmp( http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].name, "Content-Length", sizeof( "Content-Length" ) ) == 0 )
                {
                    xi_stated_sscanf_state_t tmp_state;
                    memset( &tmp_state, 0, sizeof( xi_stated_sscanf_state_t ) );

                    char tmp_status_pattern[]           = "%d";
                    const_data_descriptor_t tmp_v       = { tmp_status_pattern, sizeof( tmp_status_pattern ), sizeof( tmp_status_pattern ) };
                    const_data_descriptor_t tmp_data    =
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
                else
                {
                    http_header_type_t header_type = classify_header( http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].name );

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
            }

        } while( sscanf_state == 1 );
    }

    // STAGE 03 reading second \r\n that means that the payload should begin just right after
    {
        sscanf_state                = 0;

        while( sscanf_state == 0 )
        {
            char status_pattern[]       = "\r\n";
            const_data_descriptor_t v   = { status_pattern, sizeof( status_pattern ), sizeof( status_pattern ) };

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

            char status_pattern[]       = "%B";
            const_data_descriptor_t v   = { status_pattern, sizeof( status_pattern ), sizeof( status_pattern ) };
            void*                  pv[] = { ( void* ) http_layer_data->response->http.http_headers[ XI_HTTP_HEADER_UNKNOWN ].value };

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
