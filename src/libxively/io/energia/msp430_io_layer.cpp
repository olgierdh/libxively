// c
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "msp430_io_layer.h"
#include "msp430_data.h"

// local
#include "common.h"
#include "layer_api.h"
#include "xi_helpers.h"
#include "xi_allocator.h"
#include "xi_debug.h"
#include "xi_err.h"
#include "xi_macros.h"
#include "xi_globals.h"

#include "SimplelinkWifi/wifi.h" 
#include "SimplelinkWifi/TCPclient.h"
#include "SimplelinkWifi/SimplelinkWifi.h" 

//#ifdef __cplusplus
//extern "C" {
//#endif


layer_state_t msp430_io_layer_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint )
{
    msp430_data_t* msp430_data                = ( msp430_data_t* ) context->self->user_data;
    const const_data_descriptor_t* buffer   = ( const const_data_descriptor_t* ) data;


    if( buffer != 0 && buffer->data_size > 0 )
    {
        unsigned int len = msp430_data->socket_ptr->write((uint8_t*) buffer->data_ptr, buffer->data_size);

        if( len < buffer->data_size )
        {
            return LAYER_STATE_ERROR;
        }
    }

    return LAYER_STATE_OK;
}

layer_state_t msp430_io_layer_on_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint )
{
    msp430_data_t* msp430_data = ( msp430_data_t* ) context->self->user_data;

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

    do
    {
        memset( buffer->data_ptr, 0, buffer->data_size );
        buffer->real_size = msp430_data->socket_ptr->read((uint8_t*) buffer->data_ptr, buffer->data_size - 1 );

        xi_debug_format( "received: %d", buffer->real_size );

        buffer->data_ptr[ buffer->real_size ] = '\0'; // put guard
        buffer->curr_pos = 0;
        state = CALL_ON_NEXT_ON_DATA_READY( context->self, ( void* ) buffer, LAYER_HINT_MORE_DATA );
    } while( state == LAYER_STATE_MORE_DATA );

    return LAYER_STATE_OK;
}

layer_state_t msp430_io_layer_close(
    layer_connectivity_t* context )
{
    // extract the layer specific data
    msp430_data_t* msp430_data
        = ( msp430_data_t* ) context->self->user_data;

    xi_debug_logger( "closing socket..." );

    // close the connection & the socket
    msp430_data->socket_ptr->stop();

/*    if( msp430_data->socket_ptr->stop() == -1 )
    {
        xi_debug_logger( "error closing socket..." );
        xi_set_err( XI_SOCKET_CLOSE_ERROR );
        goto err_handling;
    }

    // safely destroy the object
    if ( msp430_data && msp430_data->socket_ptr )
    {
        xi_debug_logger( "deleting socket..." );

        delete msp430_data->socket_ptr;
        msp430_data->socket_ptr = 0;
    }

    if( msp430_data ) { XI_SAFE_FREE( msp430_data ); }
*/
    return LAYER_STATE_OK;

/*    // cleanup the memory
err_handling:
    // safely destroy the object
    if ( msp430_data && msp430_data->socket_ptr )
    {
        delete msp430_data->socket_ptr;
        msp430_data->socket_ptr = 0;
    }

    if( msp430_data ) { XI_SAFE_FREE( msp430_data ); }

    return LAYER_STATE_ERROR;*/
}


layer_state_t msp430_io_layer_on_close( layer_connectivity_t* context )
{
    return CALL_ON_NEXT_CLOSE( context->self );
}

layer_t* connect_to_endpoint(
      layer_t* layer
    , const char* address
    , const int port )
{
    // variables
    msp430_data_t* msp430_data = 0;

    // allocation of the socket connector
    WiFiClient* socket_ptr = new WiFiClient();
    XI_CHECK_MEMORY( socket_ptr );

    // set the timeout for blocking operations
    //socket_ptr->set_blocking( false, xi_globals.network_timeout );

    // allocate memory for the mbed data specific structure
    msp430_data = ( msp430_data_t* )
            xi_alloc( sizeof( msp430_data_t ) );
    XI_CHECK_MEMORY( msp430_data );

    { // to prevent the skip initializtion warning
        msp430_data->socket_ptr = socket_ptr;

        // assign the layer specific data
        layer->user_data = ( void* ) msp430_data;

        // try to connect
        int s = msp430_data->socket_ptr->connect( address, port );

        // check if not failed
        if( s == -1 )
        {
            xi_set_err( XI_SOCKET_CONNECTION_ERROR );
            //goto err_handling;
        }
    }

    // POSTCONDITIONS
    assert( mbed_data->socket_ptr != 0 );

    return layer;

/*    // cleanup the memory
err_handling:
    // safely destroy the object
    if ( msp430_data && msp430_data->socket_ptr )
    {
        delete msp430_data->socket_ptr;
        msp430_data->socket_ptr = 0;
    }
    if( msp430_data ) { XI_SAFE_FREE( msp430_data ); }

    return 0;*/
}

//#ifdef __cplusplus
//}
//#endif
