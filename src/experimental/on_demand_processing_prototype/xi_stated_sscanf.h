#ifndef __XI_STATED_SSCANF_H__
#define __XI_STATED_SSCANF_H__

#include "xi_stated_sscanf_state.h"
#include "common.h"

/**
 * @brief xi_stated_sscanf
 * @param s
 * @param pattern
 * @param source
 * @param variables
 * @return
 */
short xi_stated_sscanf(
          xi_stated_sscanf_state_t* s
        , const const_data_descriptor_t* pattern
        , const const_data_descriptor_t* source
        , void** variables );

#endif // __XI_STATED_SSCANF_H__
