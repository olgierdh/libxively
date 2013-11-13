// Copyright (c) 2003-2013, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

/**
 * \file     dummy_comm.h
 * \author     Olgierd Humenczuk
 * \brief   Implements DUMMY _communication layer_ functions [see comm_layer.h and dummy_comm.c]
 */

#ifndef __DUMMY_COMM_H__
#define __DUMMY_COMM_H__

#include "connection.h"

connection_t* dummy_open_connection( const char* address, int32_t port );

int dummy_send_data( connection_t* conn, const char* data, size_t size );

int dummy_read_data( connection_t* conn, char* buffer, size_t buffer_size );

void dummy_close_connection( connection_t* conn );

#endif // __DUMMY_COMM_H__
