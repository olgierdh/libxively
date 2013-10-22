#ifndef __HTTP_LAYER_INPUT_H__
#define __HTTP_LAYER_INPUT_H__

/**
 *\file     http_layer_input.h
 *\author   Olgierd Humenczuk
 *\brief    File contains declaration of the http layer data structure which is used
 *          within the between layer communication process.
 */


#include "xively.h"
#include "layer.h"
#include "xi_generator.h"

/**
 * \enum describes the xi http layer api input function types
 */
typedef enum
{
      HTTP_LAYER_INPUT_DATASTREAM_GET = 0
    , HTTP_LAYER_INPUT_DATASTREAM_DELETE
    , HTTP_LAYER_INPUT_DATASTREAM_UPDATE
} xi_query_type_t;


/**
 *\brief    Contains the data related to creation the transport for any payload
 *          that we shall send over network.
 *
 *\note     Version of that structure is specialized for communication with RESTful xively API
 *          so it is not fully functional generic implementation of the HTTP protocol capabilities.
 */
typedef struct
{
    xi_query_type_t     query_type;
    xi_context_t*       xi_context;
    xi_generator_t*     payload_generator;

    union
    {
        struct
        {
            const char* datastream;
        } xi_get_datastream;
    } http_layer_data_u;

} http_layer_input_t;


#endif // __HTTP_LAYER_INPUT_H__
