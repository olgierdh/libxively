// Copyright (c) 2003-2013, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

/**
 * \file    dummy_comm.c
 * \author  Olgierd Humenczuk
 * \brief   Implements DUMMY _communication layer_ abstraction interface [see comm_layer.h]
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "dummy_comm.h"
#include "comm_layer.h"
#include "xi_helpers.h"
#include "xi_allocator.h"
#include "dummy_comm_layer_data_specific.h"
#include "xi_err.h"
#include "xi_macros.h"
#include "xi_globals.h"


connection_t* dummy_open_connection( const char* address, int32_t port )
{
    // PRECONDITIONS
    assert( address != 0 );

    // variables
    dummy_comm_layer_data_specific_t* dummy_comm_data   = 0;
    connection_t* conn                                  = 0;

    // allocate memory for the dummy data specific structure
    dummy_comm_data
        = ( dummy_comm_layer_data_specific_t* ) xi_alloc(
                sizeof( dummy_comm_layer_data_specific_t ) );

    XI_CHECK_MEMORY( dummy_comm_data );

    // allocate memory for the connection layer
    conn
        = ( connection_t* ) xi_alloc(
                sizeof( connection_t ) );

    XI_CHECK_MEMORY( conn );

    // make copy of an address
    conn->address = xi_str_dup( address );
    conn->port = port;

    XI_CHECK_MEMORY( conn->address );

    // remember the layer specific part
    conn->layer_specific = ( void* ) dummy_comm_data;

    // POSTCONDITIONS
    assert( conn != 0 );
    assert( dummy_comm_data->socket_fd != -1 );

    return conn;

err_handling:
    // cleanup the memory
    if( dummy_comm_data ) { XI_SAFE_FREE( dummy_comm_data ); }
    if( conn ) { XI_SAFE_FREE( conn->address ); }
    XI_SAFE_FREE( conn );

    return 0;
}

int dummy_send_data( connection_t* conn, const char* data, size_t size )
{
    // PRECONDITIONS
    assert( conn != 0 );
    assert( conn->layer_specific != 0 );
    assert( data != 0 );
    assert( size != 0 );

    // extract the layer specific data
    /*dummy_comm_layer_data_specific_t* dummy_comm_data
        = ( dummy_comm_layer_data_specific_t* ) conn->layer_specific;*/

    // store the value
    conn->bytes_sent += size;

    return size;
}

int dummy_read_data( connection_t* conn, char* buffer, size_t buffer_size )
{
    // PRECONDITIONS
    assert( conn != 0 );
    assert( conn->layer_specific != 0 );
    assert( buffer != 0 );
    assert( buffer_size != 0 );

    // extract the layer specific data
    /*dummy_comm_layer_data_specific_t* dummy_comm_data
        = ( dummy_comm_layer_data_specific_t* ) conn->layer_specific;*/

    memset( buffer, 0, buffer_size );

    // store the value
    conn->bytes_received += 0;

    return 0;
}

void dummy_close_connection( connection_t* conn )
{
    // PRECONDITIONS
    assert( conn != 0 );

    // extract the layer specific data
    /*dummy_comm_layer_data_specific_t* dummy_comm_data
        = ( dummy_comm_layer_data_specific_t* ) conn->layer_specific;*/

//err_handling:
    // cleanup the memory
    if( conn ) { XI_SAFE_FREE( conn->layer_specific ); }
    if( conn ) { XI_SAFE_FREE( conn->address ); }
    XI_SAFE_FREE( conn );

    return;
}
