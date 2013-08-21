// Copyright (c) 2003-2013, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.
#include "comm_layer.h"
#include "dummy_comm.h"

/**
 * \file 	dummy_comm_layer.c
 * \author 	Olgierd Humenczuk
 * \brief   Implements DUMMY _communication layer_ functions [see comm_layer.h]
 */

 /**
  * \brief   Initialise mbed implementation of the _communication layer_
  */
const comm_layer_t* get_comm_layer()
{
    static comm_layer_t __dummy_comm_layer =
    {
          &dummy_open_connection
        , &dummy_send_data
        , &dummy_read_data
        , &dummy_close_connection
    };

    return &__dummy_comm_layer;
}
