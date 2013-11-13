// Copyright (c) 2003-2013, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

/**
 * \file    mbed_comm.cpp
 * \author  Olgierd Humenczuk
 * \brief   Implements mbed _communication layer_ abstraction interface using [TCPSocketConnection](http://mbed.org/users/mbed_official/code/Socket/docs/tip/classTCPSocketConnection.html) [see comm_layer.h]
 */
 
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// mbed
#include "mbed_io_layer.h"
#include "mbed_data.h"

// local
#include "xi_helpers.h"
#include "xi_allocator.h"
#include "xi_debug.h"
#include "xi_err.h"
#include "xi_macros.h"
#include "xi_globals.h"
#include "layer_api.h"

extern "C" {

layer_state_t mbed_io_layer_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint )
{
    // PRECONDITIONS
    assert( conn != 0 );
    assert( conn->layer_specific != 0 );
    assert( data != 0 );
    assert( size != 0 );

    // extract the layer specific data
    mbed_data_t* pos_comm_data
        = ( mbed_data_t* ) context->self->user_data;

    const const_data_descriptor_t* buffer   = ( const const_data_descriptor_t* ) data;

    // Why not const char* ???
    int bytes_written = pos_comm_data->socket_ptr->send_all( ( char* ) data, size );

    if( buffer != 0 && buffer->data_size > 0 )
    {
        int len = pos_comm_data->socket_ptr->send_all( ( char* ) buffer->data_ptr, buffer->data_size );

        if( len < buffer->data_size )
        {
            xi_set_err( XI_SOCKET_WRITE_ERROR );
            return LAYER_STATE_ERROR;
        }
    }

    if( hint == LAYER_HINT_NONE )
    {
        CALL_ON_SELF_ON_DATA_READY( context->self, 0, LAYER_HINT_NONE );
    }

    return LAYER_STATE_OK;
}

layer_state_t mbed_io_layer_on_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint )
{
    // PRECONDITIONS
    assert( conn != 0 );
    assert( conn->layer_specific != 0 );
    assert( buffer != 0 );
    assert( buffer_size != 0 );

    // extract the layer specific data
    mbed_data_t* pos_comm_data
        = ( mbed_data_t* ) context->self->user_data;

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
        buffer->real_size = pos_comm_data->socket_ptr->receive( buffer->data_ptr, buffer->data_size - 1 );
        buffer->data_ptr[ buffer->real_size ] = '\0'; // put guard
        buffer->curr_pos = 0;
        state = CALL_ON_NEXT_ON_DATA_READY( context->self, ( void* ) buffer, LAYER_HINT_MORE_DATA );
    } while( state == LAYER_STATE_MORE_DATA );

    return LAYER_STATE_OK;
}

layer_state_t mbed_io_layer_close(
    layer_connectivity_t* context )
{
    return CALL_ON_NEXT_CLOSE( context->self->layer_connection );
}


layer_state_t posix_io_layer_on_close( layer_connectivity_t* context )
{
    // PRECONDITIONS
    assert( context != 0 );
    assert( context->self->user_data != 0 );

    // extract the layer specific data
    mbed_data_t* pos_comm_data
        = ( mbed_data_t* ) context->self->user_data;

    // close the connection & the socket
    if( pos_comm_data->socket_ptr->close() == -1 )
    {
        xi_set_err( XI_SOCKET_CLOSE_ERROR );
        goto err_handling;
    }

    // cleanup the memory
err_handling:
    // safely destroy the object
    if ( pos_comm_data && pos_comm_data->socket_ptr )
    {
        delete pos_comm_data->socket_ptr;
        pos_comm_data->socket_ptr = 0;
    }
    if( context ) { XI_SAFE_FREE( conn->layer_specific ); }
    return LAYER_STATE_ERROR;
}

layer_t* connect_to_endpoint(
      layer_t* layer
    , const char* address
    , const int port )
{
    // PRECONDITIONS
    assert( address != 0 );

    // variables
    mbed_data_t* pos_comm_data = 0;

    // allocation of the socket connector
    TCPSocketConnection* socket_ptr = new TCPSocketConnection();
    XI_CHECK_MEMORY( socket_ptr );

    // set the timeout for blocking operations
    socket_ptr->set_blocking( true, xi_globals.network_timeout );

    // allocate memory for the mbed data specific structure
    pos_comm_data = ( mbed_data_t* )
            xi_alloc( sizeof( mbed_data_t ) );
    XI_CHECK_MEMORY( pos_comm_data );

    { // to prevent the skip initializtion warning
        pos_comm_data->socket_ptr = socket_ptr;

        // assign the layer specific data
        layer->user_data = ( void* ) pos_comm_data;

        // try to connect
        int s = pos_comm_data->socket_ptr->connect( address, port );

        // check if not failed
        if( s == -1 )
        {
            xi_set_err( XI_SOCKET_CONNECTION_ERROR );
            goto err_handling;
        }
    }

    // POSTCONDITIONS
    assert( pos_comm_data->socket_ptr != 0 );

    return layer;

    // cleanup the memory
err_handling:
    // safely destroy the object
    if ( pos_comm_data && pos_comm_data->socket_ptr )
    {
        delete pos_comm_data->socket_ptr;
        pos_comm_data->socket_ptr = 0;
    }
    if( pos_comm_data ) { XI_SAFE_FREE( pos_comm_data ); }

    return 0;
}

}
