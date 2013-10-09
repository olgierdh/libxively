#ifndef __LAYER_H__
#define __LAYER_H__

/**
 * \file    layer.h
 * \author  Olgierd Humenczuk
 * \brief   the layer type definition used across the whole layer system
 */

// local
#include "layer_connectivity.h"
#include "layer_debug_info.h"
#include "layer_interface.h"
#include "layer_type.h"

/**
 * \brief The layer struct that makes the access to the generated types possible
 */
typedef struct layer
{
    const layer_interface_t*    layer_functions;
    layer_connectivity_t        layer_connection;
    layer_type_id_t             layer_type_id;
    void*                       user_data;
    short                       layer_states[ 4 ];
#ifdef __DEBUG
    layer_debug_info_t          debug_info;
#endif
} layer_t;


#endif // __LAYER_H__
