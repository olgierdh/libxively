// Copyright (c) 2003-2013, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

/**
 * \file    cc3000_comm.c
 * \author  Olgierd Humenczuk
 * \brief   Implements CC3000 _communication layer_ abstraction interface [see comm_layer.h]
 */

#include <stdio.h>
#include "cc3000/cc3000_common.h"
#include "cc3000/socket.h"

#include <string.h>
#include <stdint.h>

#include "cc3000_comm.h"
#include "comm_layer.h"
#include "xi_helpers.h"
#include "xi_allocator.h"
#include "cc3000_comm_layer_data_specific.h"
#include "xi_debug.h"
#include "xi_err.h"
#include "xi_macros.h"
#include "xi_globals.h"


connection_t* cc3000_open_connection( const char* address, int32_t port )
{
    // PRECONDITIONS
    assert( address != 0 );

    // variables
    cc3000_comm_layer_data_specific_t* comm_data = 0;
    connection_t* conn                           = 0;

    // allocate memory for the cc3000 data specific structure
    comm_data
        = ( cc3000_comm_layer_data_specific_t* ) xi_alloc(
                sizeof( cc3000_comm_layer_data_specific_t ) );

    XI_CHECK_MEMORY( comm_data );

    // allocate memory for the connection layer
    conn
        = ( connection_t* ) xi_alloc(
                sizeof( connection_t ) );

    XI_CHECK_MEMORY( conn );

    // make copy of an address
    conn->address = xi_str_dup( address );
    conn->port = port;

    XI_CHECK_MEMORY( conn->address );

    // initialze the fd for the TCP/IP socket
    comm_data->socket_fd = socket( AF_INET, SOCK_STREAM, 0 );
    if( comm_data->socket_fd == -1 )
    {
        xi_set_err( XI_SOCKET_INITIALIZATION_ERROR );
        return 0;
    }

    // remember the layer specific part
    conn->layer_specific = ( void* ) comm_data;

    // socket specific data
    sockaddr_in name;

    memset( &name, 0, sizeof( sockaddr_in ) );

    name.sin_family = AF_INET;
    name.sin_port = htons( port );

    //unsigned long saddr * = name.sin_addr;

    if( gethostbyname( conn->address, strlen( conn->address ), &(name.sin_addr.s_addr) ) == EFAIL )
    {
        xi_set_err( XI_SOCKET_GETHOSTBYNAME_ERROR );
        goto err_handling;
    }


    if( connect( comm_data->socket_fd, ( sockaddr* ) &name, sizeof( sockaddr ) ) == -1 )
    {
        xi_set_err( XI_SOCKET_CONNECTION_ERROR );
        goto err_handling;
    }

    // POSTCONDITIONS
    assert( conn != 0 );
    assert( comm_data->socket_fd != -1 );

    return conn;

err_handling:
    // cleanup the memory
    if( comm_data ) { XI_SAFE_FREE( comm_data ); }
    if( conn ) { XI_SAFE_FREE( conn->address ); }
    XI_SAFE_FREE( conn );

    return 0;
}

int cc3000_send_data( connection_t* conn, const char* data, size_t size )
{
    // PRECONDITIONS
    assert( conn != 0 );
    assert( conn->layer_specific != 0 );
    assert( data != 0 );
    assert( size != 0 );

    // extract the layer specific data
    cc3000_comm_layer_data_specific_t* comm_data
        = ( cc3000_comm_layer_data_specific_t* ) conn->layer_specific;

    int bytes_written = send( comm_data->socket_fd, data, size, 0 );

    if( bytes_written == - 1 )
    {
        xi_set_err( XI_SOCKET_WRITE_ERROR );
    }

    // store the value
    conn->bytes_sent += bytes_written;

    return bytes_written;
}

int cc3000_read_data( connection_t* conn, char* buffer, size_t buffer_size )
{
    // PRECONDITIONS
    assert( conn != 0 );
    assert( conn->layer_specific != 0 );
    assert( buffer != 0 );
    assert( buffer_size != 0 );

    // extract the layer specific data
    cc3000_comm_layer_data_specific_t* comm_data
        = ( cc3000_comm_layer_data_specific_t* ) conn->layer_specific;

    memset( buffer, 0, buffer_size );
    int bytes_read = recv( comm_data->socket_fd, buffer, buffer_size, 0 );

    if( bytes_read == -1 )
    {
        xi_set_err( XI_SOCKET_READ_ERROR );
    }

    // store the value
    conn->bytes_received += bytes_read;

    return bytes_read;
}

void cc3000_close_connection( connection_t* conn )
{
    // PRECONDITIONS
    assert( conn != 0 );

    // extract the layer specific data
    cc3000_comm_layer_data_specific_t* comm_data
        = ( cc3000_comm_layer_data_specific_t* ) conn->layer_specific;

    // close the connection & the socket
    if( closesocket( comm_data->socket_fd ) == -1 )
    {
        xi_set_err( XI_SOCKET_CLOSE_ERROR );
        goto err_handling;
    }

err_handling:
    // cleanup the memory
    if( conn ) { XI_SAFE_FREE( conn->layer_specific ); }
    if( conn ) { XI_SAFE_FREE( conn->address ); }
    XI_SAFE_FREE( conn );

    return;
}
