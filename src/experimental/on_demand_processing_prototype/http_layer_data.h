#ifndef __HTTP_LAYER_DATA_H__
#define __HTTP_LAYER_DATA_H__

/**
 *\file     http_layer_data.h
 *\author   Olgierd Humenczuk
 *\brief    containes the structure related to the internal layer data that shall be used over the lifecycle of each instance
 *          to process the queries and parse the responses.
 */

/**
 *\brief    shall contain the data needed via parsing and constructing queries
 */
typedef struct
{
    char parser_state;  //!< represents the state of the http layer parser parsing header, state, payload etc, etc.
    char buffer[ 32 ];  //!< this buffer size must be equal to the size of the biggest token that may be parsed will be used to store rest of the unparsed data to continue in next sweep
} http_layer_data_t;


#endif // __HTTP_LAYER_DATA_H__
