// Copyright (c) 2003-2013, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

/**
 * \file    cc3000_comm_layer.c
 * \author  Olgierd Humenczuk
 * \brief   Implements CC3000 _communication layer_ functions [see comm_layer.h]
 */

#include "comm_layer.h"
#include "cc3000_comm.h"

 /**
  * \brief   Initialise mbed implementation of the _communication layer_
  */
const comm_layer_t* get_comm_layer()
{
    static comm_layer_t __cc3000_comm_layer =
    {
          &cc3000_open_connection
        , &cc3000_send_data
        , &cc3000_read_data
        , &cc3000_close_connection
    };

    return &__cc3000_comm_layer;
}
