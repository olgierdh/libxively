#include "layer_api.h"
#include "http_layer.h"
#include "http_layer_constants.h"
#include "http_layer_input.h"
#include "http_layer_data.h"
#include "xi_macros.h"
#include "xi_debug.h"
#include "common.h"


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

    // the end, no more data doule crlf
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
    // expecting data buffer so unpack it
    const const_data_descriptor_t* data_description = ( const const_data_descriptor_t* ) data;

    for( int i = 0; i < data_description->hint_size; ++i )
    {
        putchar( data_description->data_ptr[ i ] );
        if( data_description->data_ptr[ i ] == '\r' )
        {
            int a = 0;
        }
        fflush( stdout );
    }

    return LAYER_STATE_MORE_DATA;
}


/**
 * \brief  see the layer_interface for details
 */
static inline layer_state_t http_line_layer_on_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( hint );

    // expecting data buffer so unpack it
    const const_data_descriptor_t* data_description = ( const const_data_descriptor_t* ) data;
    http_layer_data_t* http_data                    = ( http_layer_data_t* ) context->self->user_data;

    // temporary variables
    char current_offset = 0;

    // check if the tmp buffer contains any partial line
    if( http_data->last_char_marker )
    {
        // check if the last sign was \r if that is so we are expecting the \n to be the first in line
        // if not we assume that we are looking for the sequence normally
        if( http_data->line_buffer[ http_data->last_char_marker - 1 ] == '\r' )
        {
            if( data_description->data_ptr[ 0 ] == '\n' )
            {
                // move the current offset to the proper position
                current_offset = 1;

                // call the next layer on data ready with proper values
                const const_data_descriptor_t tmp_data_description = { http_data->line_buffer, http_data->last_char_marker - 1, http_data->last_char_marker - 1 };
                printf( "\n0\n");
                layer_state_t tmp_layer_state = http_layer_on_data_ready_http_parse( &context->self->layer_connection, ( void* ) &tmp_data_description, LAYER_HINT_NONE );

                http_data->last_char_marker = 0;

                if( tmp_layer_state == LAYER_STATE_OK ) // the end of parsing may have happend here
                {
                    return LAYER_STATE_OK;
                } // else let's continue
            }
        }
        else
        {
            // get the last eol
            char* eol = strstr( data_description->data_ptr, XI_HTTP_CRLF );

            if( eol )
            {
                memcpy( http_data->line_buffer + http_data->last_char_marker, data_description->data_ptr, eol - data_description->data_ptr );

                // printf( "\n[ [%s] ]\n", http_data->line_buffer + http_data->last_char_marker );

                http_data->last_char_marker += eol - data_description->data_ptr;
                http_data->line_buffer[ http_data->last_char_marker ] = '\0';

                const const_data_descriptor_t tmp_data_description = { http_data->line_buffer, http_data->last_char_marker, http_data->last_char_marker };
                printf( "\n1\n" );
                layer_state_t tmp_layer_state = http_layer_on_data_ready_http_parse( &context->self->layer_connection, ( void* ) &tmp_data_description, LAYER_HINT_NONE );

                if( tmp_layer_state == LAYER_STATE_OK ) // the end of parsing may have happend here
                {
                    return LAYER_STATE_OK;
                } // else let's continue

                eol += strlen( XI_HTTP_CRLF );
                current_offset = eol - data_description->data_ptr;
                http_data->last_char_marker = 0;
            }
            else
            {
                // most probably we are in trouble here
                printf( "\nProblem!!!!\n" );
                printf( "\n[%s] [size:%d]\n", data_description->data_ptr, data_description->hint_size );
                return LAYER_STATE_ERROR;
            }
        }
    }

    // endless loop for normal case that looks for the \r\n token
    for( ; ; )
    {
        // get the last eol
        char* eol = strstr( data_description->data_ptr + current_offset, XI_HTTP_CRLF );

        if( eol ) //
        {

            // call the next layer on data ready with proper values
            const const_data_descriptor_t tmp_data_description = { data_description->data_ptr + current_offset, data_description->data_size - current_offset, eol - ( data_description->data_ptr + current_offset ) };
            printf( "\n2\n" );
            layer_state_t tmp_layer_state = http_layer_on_data_ready_http_parse( &context->self->layer_connection, ( void* ) &tmp_data_description, LAYER_HINT_NONE );

            if( tmp_layer_state == LAYER_STATE_OK ) // the end of parsing may have happend here
            {
                return LAYER_STATE_OK;
            } // else let's continue

            // don't forget to update the offset
            eol += strlen( XI_HTTP_CRLF );
            current_offset = eol - data_description->data_ptr;

            if( current_offset == data_description->data_size )
            {
                // we are at the end of the size and the CLRF has been read
                return LAYER_STATE_MORE_DATA;
            }
        }
        else
        {
            // copy the unparsed part of the buffer
            memcpy( http_data->line_buffer, data_description->data_ptr + current_offset, data_description->hint_size - current_offset );

            // set up the proper marker position
            http_data->last_char_marker = data_description->hint_size - current_offset;


            if( current_offset > data_description->hint_size )
            {
                printf( "\noffset too big!\n" );
            }

            // set up the guard in order to prevent strstr from overflow
            http_data->line_buffer[ http_data->last_char_marker ] = '\0';

            break;
        }
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


    // expecting data buffer so unpack it
    http_layer_data_t* http_layer_data = ( http_layer_data_t* ) context->self->user_data;

    switch( http_layer_data->parser_state )
    {
        case 0:
            return http_line_layer_on_data_ready( &context->self->layer_connection, data, hint );
    }

    return LAYER_STATE_MORE_DATA;
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
