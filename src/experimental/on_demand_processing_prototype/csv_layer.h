#ifndef __CSV_LAYER_H__
#define __CSV_LAYER_H__

// local
#include "layer.h"
#include "common.h"
#include "xively.h"
#include "csv_layer_data.h"

/**
 * \file   csv_layer.h
 * \author Olgierd Humenczuk
 * \brief  File that containes the declaration of the csv layer functions
 */

/**
 * \brief   see the layer_interface for details
 */
layer_state_t csv_layer_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint );

/**
 * \brief  see the layer_interface for details
 */
layer_state_t csv_layer_on_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint );

/**
 * \brief  see the layer_interface for details
 */
layer_state_t csv_layer_close(
    layer_connectivity_t* context );

/**
 * \brief  see the layer_interface for details
 */
layer_state_t csv_layer_on_close(
    layer_connectivity_t* context );

layer_state_t csv_layer_parse_datastream(
        csv_layer_data_t* csv_layer_data
      , const_data_descriptor_t* data
      , const layer_hint_t hint
      , xi_datapoint_t* dp );


const void* csv_layer_data_generator_datastream_get(
          const void* input
        , short* state );

const void* csv_layer_data_generator_datapoint(
          const void* input
        , short* state );


#endif // __CSV_LAYER_H__
