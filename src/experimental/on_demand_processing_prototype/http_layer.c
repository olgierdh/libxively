#include "http_layer.h"
#include "xi_macros.h"

/**
 * \brief   see the layer_interface for details
 */
layer_state_t http_layer_on_demand(
      layer_connectivity_t* context
    , char* buffer
    , size_t size
    , const char impulse )
{
    XI_UNUSED( context );
    XI_UNUSED( buffer );
    XI_UNUSED( size );
    XI_UNUSED( impulse );

    return LAYER_STATE_OK;
}

/**
 * \brief  see the layer_interface for details
 */
layer_state_t http_layer_on_data_ready(
      layer_connectivity_t* context
    , const char* buffer
    , size_t size
    , const char impulse )
{
    XI_UNUSED( context );
    XI_UNUSED( buffer );
    XI_UNUSED( size );
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
