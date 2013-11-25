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

#include "util/debug.h"

#ifdef __cplusplus
extern "C" {
#endif

layer_state_t wiznet_io_layer_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint )
{
    wiznet_data_t* wiznet_data              = ( wiznet_data_t* ) context->self->user_data;
    const const_data_descriptor_t* buffer   = ( const const_data_descriptor_t* ) data;

    //xi_debug_printf( "%s", buffer->data_ptr );

    XI_UNUSED( hint );
    dbgPrintf("(%s:%d) -> entered\r\n", __func__, __LINE__);

    if( buffer != 0 && buffer->data_size > 0 )
    {
        dbgPrintf("%s: abour to call TCP_Send\r\n", __func__);
        int len = TCP_Send( wiznet_data->socket_fd, ( uint8_t* ) buffer->data_ptr, buffer->data_size );
        dbgPrintf("%s: len=%d\r\n", __func__, len);

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

    if( len < 0 )
    {
        return LAYER_STATE_ERROR;
    }

    buffer->real_size = len;

    buffer->data_ptr[ buffer->real_size ] = '\0'; // put guard
    buffer->curr_pos = 0;
    state = CALL_ON_NEXT_ON_DATA_READY( context->self, ( void* ) buffer, LAYER_HINT_MORE_DATA );

    if( state == LAYER_STATE_MORE_DATA )
    {
        return LAYER_STATE_NOT_READY;
    }

    return LAYER_STATE_OK;
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

    if( layer == 0 ) dbgPrintf("It is zero already!\r\n");

    // just a static data for now
    static wiznet_data_t wiznet_data;
    memset( &wiznet_data, 0, sizeof( wiznet_data_t ) );

    layer->user_data                            = ( void* ) &wiznet_data;

    dbgPrintf( "Creating socket...\r\n" );
    wiznet_data.socket_fd                       = 5;
    dbgPrintf( "Socket creation [ok]\r\n" );

    uint8_t ip[ 4 ] = XI_IP;

    dbgPrintf( "Connecting to the endpoint...\r\n" );
    TCP_OpenClientSocket( wiznet_data.socket_fd, 0, ip, XI_PORT );
    dbgPrintf( "Connecting to the endpoint [ok]\r\n" );

    // POSTCONDITIONS
    //assert( layer != 0 );
    //assert( wiznet_data->socket_fd != -1 );

    dbgPrintf("fd=%d\r\n", ((wiznet_data_t *) layer->user_data)->socket_fd);

    if( layer == 0 ) dbgPrintf("It is zero.\r\n");
    return layer;
}

#ifdef __cplusplus
}
#endif
