#include "layer_api.h"
#include "http_layer.h"
#include "xi_macros.h"
#include "common.h"


// set of constants
static const char XI_HTTP_POST[]                    = "POST ";
static const char XI_HTTP_GET[]                     = "GET ";
static const char XI_HTTP_CRLF[]                    = "\r\n";
static const char XI_HTTP_TEMPLATE_FEED[]           = "/v2/feeds";
static const char XI_HTTP_TEMPLATE_CSV[]            = ".csv";
static const char XI_HTTP_TEMPLATE_HTTP[]           = "HTTP/1.1";
static const char XI_HTTP_TEMPLATE_HOST[]           = "Host: ";
static const char XI_HTTP_TEMPLATE_USER_AGENT[]     = "User-Agent: ";
static const char XI_HTTP_TEMPLATE_ACCEPT[]         = "Accept: */*";
static const char XI_HTTP_TEMPLATE_X_API_KEY[]      = "X-ApiKey: ";

/**
 * \brief layer_sender little helper to encapsulate
 *        error handling over sending data between layers
 */
#define layer_sender( context, data, hint ) \
{ \
    const const_data_descriptor_t tmp_data = { (data), strlen( (data) ) }; \
    layer_state_t layer_state = CALL_ON_PREV_ON_DATA_READY( context->self, ( const void* ) &tmp_data, hint ); \
    switch( layer_state ) \
    { \
        case LAYER_STATE_OK: \
            break; \
        default: \
            return layer_state; \
    } \
}

/**
 * \brief   see the layer_interface for details
 */
layer_state_t http_layer_data_ready(
      layer_connectivity_t* context
    , void* data
    , const layer_hint_t hint )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( hint );

    return LAYER_STATE_OK;
}

/**
 * \brief  see the layer_interface for details
 */
layer_state_t http_layer_data_ready_datastream_get(
      layer_connectivity_t* context
    , const int feed_id
    , const char* datastream_id
    , const char* api_key )
{
    // buffer that is locally used to encode the data to be sent over the layers system
    char buff[ XI_HTTP_SEND_BUFFER_SIZE ];

    // SEND TYPE
    layer_sender( context, XI_HTTP_GET, LAYER_HINT_MORE_DATA );

    // SEND ADDRESS
    layer_sender( context, XI_HTTP_TEMPLATE_FEED, LAYER_HINT_MORE_DATA );
    layer_sender( context, "/", LAYER_HINT_MORE_DATA );

    memset( buff, 0, sizeof( buff ) );
    sprintf( buff, "%d", feed_id );

    layer_sender( context, buff, LAYER_HINT_MORE_DATA ); // feed id
    layer_sender( context, "datastreams", LAYER_HINT_MORE_DATA );
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
    layer_sender( context, "*/*", LAYER_HINT_MORE_DATA );
    layer_sender( context, XI_HTTP_CRLF, LAYER_HINT_MORE_DATA );

    // A API KEY
    layer_sender( context, XI_HTTP_TEMPLATE_X_API_KEY, LAYER_HINT_MORE_DATA );
    layer_sender( context, api_key, LAYER_HINT_MORE_DATA ); // api key
    layer_sender( context, XI_HTTP_CRLF, LAYER_HINT_MORE_DATA );

    // the end, no more data double crlf
    layer_sender( context, XI_HTTP_CRLF, LAYER_HINT_MORE_DATA );
    layer_sender( context, XI_HTTP_CRLF, LAYER_HINT_NONE );

    return LAYER_STATE_OK;
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

    return LAYER_STATE_OK;
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
