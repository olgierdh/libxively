// Copyright (c) 2003-2013, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

/**
 * \file    cc3000_comm.h
 * \author  Olgierd Humenczuk
 * \brief   Implements CC3000 _communication layer_ functions [see comm_layer.h and cc3000_comm.c]
 */

#ifndef __CC3000_COMM_H__
#define __CC3000_COMM_H__

#include "connection.h"

connection_t* cc3000_open_connection( const char* address, int32_t port );

int cc3000_send_data( connection_t* conn, const char* data, size_t size );

int cc3000_read_data( connection_t* conn, char* buffer, size_t buffer_size );

void cc3000_close_connection( connection_t* conn );

#endif // __CC3000_COMM_H__
