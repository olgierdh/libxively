#ifndef __CSV_LAYER_DATA_H__
#define __CSV_LAYER_DATA_H__

#include "http_layer_input.h"
#include "xi_stated_csv_decode_value_state.h"
#include "xi_stated_sscanf_state.h"

typedef struct
{
    http_layer_input_t*                 http_layer_input;
    unsigned short                      datapoint_decode_state;
    unsigned short                      feed_decode_state;
    xi_stated_csv_decode_value_state_t  csv_decode_value_state;
    xi_stated_sscanf_state_t            stated_sscanf_state;
} csv_layer_data_t;

#endif // __CSV_LAYER_DATA_H__
