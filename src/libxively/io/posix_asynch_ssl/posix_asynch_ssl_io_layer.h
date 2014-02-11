#ifndef __POSIX_ASYNCH_SSL_IO_LAYER_H__
#define __POSIX_ASYNCH_SSL_IO_LAYER_H__

// local
#include "xi_layer.h"

#ifdef __cplusplus
extern "C" {
#endif

layer_state_t posix_asynch_ssl_io_layer_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint );

layer_state_t posix_asynch_ssl_io_layer_on_data_ready(
      layer_connectivity_t* context
    , const void* data
    , const layer_hint_t hint );

layer_state_t posix_asynch_ssl_io_layer_close(
    layer_connectivity_t* context );

layer_state_t posix_asynch_ssl_io_layer_on_close(
    layer_connectivity_t* context );

layer_t* connect_to_endpoint(
      layer_t* layer
    , const char* address
    , const int port );

#ifdef __cplusplus
}
#endif

#endif // __POSIX_ASYNCH_SSL_IO_LAYER_H__
