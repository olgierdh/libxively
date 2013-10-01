#include "layer_api.h"
#include "http_layer.h"
#include "xi_macros.h"
#include "common.h"

static const char XI_HTTP_POST[]                    = "POST";
static const char XI_HTTP_GET[]                     = "GET";
static const char XI_HTTP_CRLF[]                    = "\r\n";
static const char XI_HTTP_TEMPLATE_FEED[]           = "/v2/feeds";
static const char XI_HTTP_TEMPLATE_CSV[]            = ".csv";
static const char XI_HTTP_TEMPLATE_HTTP[]           = "HTTP/1.1";
static const char XI_HTTP_TEMPLATE_HOST[]           = "Host: ";
static const char XI_HTTP_TEMPLATE_USER_AGENT[]     = "User-Agent: ";
static const char XI_HTTP_TEMPLATE_ACCEPT[]         = "Accept: */*";
static const char XI_HTTP_TEMPLATE_X_API_KEY[]      = "X-ApiKey: ";

/**
 * \brief   see the layer_interface for details
 */
layer_state_t http_layer_on_demand(
      layer_connectivity_t* context
    , void* data
    , const char impulse )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( impulse );

    return LAYER_STATE_OK;
}

/**
 * \brief  see the layer_interface for details
 */
layer_state_t http_layer_on_data_ready_datastream_update(
      layer_connectivity_t* context
    , const void* data
    , const char impulse )
{
    XI_UNUSED( impulse );

    char buff[ 32 ];
    memset( buff, 0, sizeof( buff ) );


    // SEND TYPE
    {
        data_descriptor_t tmp_data = { XI_HTTP_GET, sizeof( XI_HTTP_GET ) };
        CALL_ON_PREV_ON_DATA_READY( context->self, ( void* ) &tmp_data, 0 );
    }
    // SEND ADDRESS
    {

    }
    // SEND HTTP
    {

    }
    // SEND HOST
    {

    }
    // SEND USER AGENT
    {

    }
    // SEND USER AGENT
    {

    }
    // A API KEY
    {

    }


    return LAYER_STATE_OK;
}

/**
 * \brief  see the layer_interface for details
 */
layer_state_t http_layer_on_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const char impulse )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( impulse );

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
