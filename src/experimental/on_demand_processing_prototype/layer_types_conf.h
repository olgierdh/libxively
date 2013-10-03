#ifndef __LAYER_TYPES_CONF_H__
#define __LAYER_TYPES_CONF_H__

/**
 *\file     layer_types_conf.h
 *\author   Olgierd Humenczuk
 *\brief    containes set of macros required to create the layers types configuration
 */

#include "layer_type.h"

#define BEGIN_LAYER_TYPES_CONF() \
    const layer_type_t LAYER_TYPES[] = {

#define LAYER_TYPE( type_id, data_ready, on_data_ready, close, on_close ) \
    { type_id, { data_ready, on_data_ready, close, on_close } }

#define END_LAYER_TYPES_CONF() \
    };



#endif // __LAYER_TYPES_CONF_H__
