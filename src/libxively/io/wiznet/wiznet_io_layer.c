// c
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// local
#include "wiznet_io_layer.h"
#include "wiznet_data.h"
#include "xi_macros.h"
#include "layer_api.h"
#include "common.h"
#include "xi_user_config.h"

#ifdef __cplusplus
extern "C" {
#endif

extern wiznet_data_t XI_IO_WIZNET_DATA;

layer_state_t wiznet_io_layer_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint )
{
    wiznet_data_t* wiznet_data              = ( wiznet_data_t* ) context->self->user_data;
    const const_data_descriptor_t* buffer   = ( const const_data_descriptor_t* ) data;

    //xi_debug_printf( "%s", buffer->data_ptr );

    XI_UNUSED( hint );
    //xi_debug_function_entered();

    if( buffer != 0 && buffer->data_size > 0 )
    {
        //xi_debug_logger("about to call TCP_Send");
        int len = TCP_Send( wiznet_data->socket_fd, ( uint8_t* ) buffer->data_ptr, buffer->data_size );
        xi_debug_format("sent len=%d", len);

        if( len < 0 )
        {
            return LAYER_STATE_ERROR;
        }

        if( len < ( int ) buffer->data_size )
        {
            return LAYER_STATE_ERROR;
        }
    }

    return LAYER_STATE_NOT_READY;
}

layer_state_t wiznet_io_layer_on_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint )
{
    //xi_debug_logger( "[wiznet_io_layer_on_data_ready]" );

    wiznet_data_t* wiznet_data = ( wiznet_data_t* ) context->self->user_data;

    XI_UNUSED( hint );

    data_descriptor_t* buffer = 0;

    if( data )
    {
        buffer = ( data_descriptor_t* ) data;
    }
    else
    {
        static char data_buffer[ 32 ];
        memset( data_buffer, 0, sizeof( data_buffer ) );
        static data_descriptor_t buffer_descriptor = { data_buffer, sizeof( data_buffer ), 0, 0 };
        buffer = &buffer_descriptor;
    }

    layer_state_t state = LAYER_STATE_OK;

    memset( buffer->data_ptr, 0, buffer->data_size );
    int len = TCP_Recv( wiznet_data->socket_fd, ( uint8_t* ) buffer->data_ptr, buffer->data_size - 1 );
    xi_debug_format("recv len=%d", len);


    if( len < 0 )
    {
        return LAYER_STATE_ERROR;
    }

    if( len == 0 )
    {
        return LAYER_STATE_NOT_READY;
    }

    buffer->real_size = len;

    buffer->data_ptr[ buffer->real_size ] = '\0'; // put guard
    buffer->curr_pos = 0;
    state = CALL_ON_NEXT_ON_DATA_READY( context->self, ( void* ) buffer, LAYER_HINT_MORE_DATA );

    if( state == LAYER_STATE_MORE_DATA )
    {
        return LAYER_STATE_NOT_READY;
    }

    return state;
}

layer_state_t wiznet_io_layer_close( layer_connectivity_t* context )
{
    XI_UNUSED( context );
    return LAYER_STATE_OK;
}

layer_state_t wiznet_io_layer_on_close( layer_connectivity_t* context )
{
    TCP_CloseSocket( ( ( wiznet_data_t* ) context->self->user_data )->socket_fd );
    return LAYER_STATE_OK;
}

layer_t* connect_to_endpoint(
      layer_t* layer
    , const char* address
    , const int port )
{
    XI_UNUSED( address );
    XI_UNUSED( port );

    //if( XI_IO_WIZNET_DATA )
    //{
    //    static wiznet_data_t wiznet_data = XI_IO_WIZNET_DATA;
    //}
    //else
    //{
        static wiznet_data_t wiznet_data = { XI_IO_WIZNET_SOCKET_FD, { 173, 203, 98, 29 }};
    //}

    layer->user_data                            = ( void* ) &wiznet_data;

    //xi_debug_logger( "Creating socket..." );

    //xi_debug_logger( "Socket creation [ok]" );

    //xi_debug_logger( "Connecting to the endpoint..." );
    TCP_OpenClientSocket( wiznet_data.socket_fd, 0, wiznet_data.ip_addr, XI_PORT );
    //xi_debug_logger( "Connecting to the endpoint [ok]" );

    // POSTCONDITIONS
    //assert( layer != 0 );
    //assert( wiznet_data->socket_fd != -1 );

    //i_debug_format("fd=%d", ((wiznet_data_t *) layer->user_data)->socket_fd);

    return layer;
}

#ifdef __cplusplus
}
#endif
