#include "layer_api.h"
#include "http_layer.h"
#include "http_layer_constants.h"
#include "http_layer_input.h"
#include "http_layer_data.h"
#include "xi_macros.h"
#include "xi_debug.h"
#include "common.h"
#include "xi_stated_sscanf_state.h"

/**
 * @brief xi_stated_sscanf
 * @param state
 * @param pattern
 * @return -1 if pattern wasn't match, 0 if more data needed, 1 if pattern has been matched and there is still data in the source
 */
short xi_stated_sscanf(
          xi_stated_sscanf_state_t* s
        , const const_data_descriptor_t* pattern
        , const const_data_descriptor_t* source
        , void** variables )
{
    BEGIN_CORO( s->state )

    s->vi = 0;
    s->p  = 0;

    for( ; s->p < pattern->hint_size - 1; )
    {
        if( pattern->data_ptr[ s->p ] != '%' ) // accept state
        {
            if( pattern->data_ptr[ s->p++ ] != source->data_ptr[ s->i++ ] )
            {
                return -1;
            }
            else
            {
                if( s->i == source->hint_size ) { goto xi_stated_sscanf_1; }

                continue;

xi_stated_sscanf_1:
                YIELD( s->state, 0 )
                s->i = 0;
                continue;
            }
        }
        else // parsing state
        {
            // simplified so don't expect to parse %%
            s->p++; // let's move the marker to the type

            switch( pattern->data_ptr[ s->p ] )
            {
                case 'd':
                    {
                        s->buff_len = 0;

                        goto xi_stated_sscanf_3;
xi_stated_sscanf_2:
                        YIELD( s->state, 0 )
                        s->i = 0;
xi_stated_sscanf_3:
                        while( source->data_ptr[ s->i ] >= 48 && source->data_ptr[ s->i ] <= 57 )
                        {
                            s->buffer[ s->buff_len++ ] = source->data_ptr[ s->i++ ];

                            if( s->i == source->hint_size )
                            {
                                goto xi_stated_sscanf_2;
                            }
                        }

                        short base      = 10;
                        int*  value     = ( int* ) variables[ s->vi ];
                        *value          = ( s->buffer[ s->buff_len - 1 ] - 48 );

                        for( unsigned char j = 1; j < s->buff_len; ++j, base *= 10 )
                        { *value += base * ( s->buffer[ s->buff_len - j - 1 ] - 48 ); }

                        s->p++;     // move on, finished with parsing
                        s->vi++;    // switch to the next variable
                    }
                break;
                case 's':
                    {
                        s->tmp_i = 0;
                        char* svalue = ( char* ) variables[ s->vi ];

                        goto xi_stated_sscanf_5;
xi_stated_sscanf_4:
                        YIELD( s->state, 0 )
                        s->i = 0;
xi_stated_sscanf_5:

                        while( source->data_ptr[ s->i ] >= 65 && source->data_ptr[ s->i ] <= 122 )
                        {
                            svalue[ s->tmp_i++ ] = source->data_ptr[ s->i++ ];

                            if( s->i == source->hint_size )
                            {
                                goto xi_stated_sscanf_4;
                            }
                        }

                        svalue[ s->tmp_i ] = '\0'; // put guard

                        s->p++;     // move on, finished with parsing
                        s->vi++;    // switch to the next variable
                    }
                break;
            case '.':
                {
                    s->tmp_i = 0;
                    char* svalue = ( char* ) variables[ s->vi ];

                    goto xi_stated_sscanf_7;
xi_stated_sscanf_6:
                    YIELD( s->state, 0 )
                    s->i = 0;
xi_stated_sscanf_7:

                    while( source->data_ptr[ s->i ] >= 32 && source->data_ptr[ s->i ] <= 122 )
                    {
                        svalue[ s->tmp_i++ ] = source->data_ptr[ s->i++ ];

                        if( s->i == source->hint_size )
                        {
                            goto xi_stated_sscanf_6;
                        }
                    }

                    svalue[ s->tmp_i ] = '\0'; // put guard

                    s->p++;     // move on, finished with parsing
                    s->vi++;    // switch to the next variable
                }
            break;
            }
        }
    }

    RESTART( s->state, 1 )

    END_CORO()

    return 1;
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
    char buff[ 32 ];

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
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( hint );

    // unpack http_layer_data so unpack it
    http_layer_data_t* http_layer_data = ( http_layer_data_t* ) context->self->user_data;

    // expecting data buffer
    const const_data_descriptor_t* data_description = ( const const_data_descriptor_t* ) data;
    int  value            = 0;
    short sscanf_state    = 0;
    char svalue[ 32 ];
    char header_name[ 16 ];

    xi_stated_sscanf_state_t* xi_stated_state = &http_layer_data->xi_stated_sscanf_state;

    BEGIN_CORO( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ] )

    memset( xi_stated_state, 0, sizeof( xi_stated_sscanf_state_t ) );

    // STAGE 01 find the http status
    {

        //
        {
            char status_pattern[]       = "HTTP/1.1 %d %s\r\n";
            const_data_descriptor_t v   = { status_pattern, sizeof( status_pattern ), sizeof( status_pattern ) };
            void*                  pv[] = { ( void* ) &value, ( void* ) &svalue };

            goto http_layer_on_data_ready_2;

http_layer_on_data_ready_1:
            YIELD( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ], LAYER_STATE_MORE_DATA )

http_layer_on_data_ready_2:

            sscanf_state = xi_stated_sscanf(
                          xi_stated_state
                        , &v
                        , ( const_data_descriptor_t* ) data
                        , pv );

            switch( sscanf_state )
            {
                case 0:
                    goto http_layer_on_data_ready_1;
                case -1:
                    goto http_layer_on_data_ready_3;
                case 1:
                    goto http_layer_on_data_ready_4;
            }

http_layer_on_data_ready_3:
            EXIT( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ], LAYER_STATE_ERROR )
        }
    }

http_layer_on_data_ready_4:
    // stage 2 reading headers
    {
        char status_pattern[]       = "%s: %.\r\n";
        const_data_descriptor_t v   = { status_pattern, sizeof( status_pattern ), sizeof( status_pattern ) };
        void*                  pv[] = { ( void* ) &header_name, ( void* ) &svalue };

        goto http_layer_on_data_ready_6;

http_layer_on_data_ready_5:
        YIELD( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ], LAYER_STATE_MORE_DATA )

http_layer_on_data_ready_6:

        sscanf_state = xi_stated_sscanf(
                      xi_stated_state
                    , &v
                    , ( const_data_descriptor_t* ) data
                    , pv );

        switch( sscanf_state )
        {
            case 0:
                goto http_layer_on_data_ready_5;
            case -1:
                goto http_layer_on_data_ready_7;
            case 1:
                goto http_layer_on_data_ready_8;
        }

http_layer_on_data_ready_7:
        EXIT( context->self->layer_states[ FUNCTION_ID_ON_DATA_READY ], LAYER_STATE_ERROR )
    }

http_layer_on_data_ready_8:
    return LAYER_STATE_OK;

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
