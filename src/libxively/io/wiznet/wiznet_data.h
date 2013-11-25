#ifndef __POSIX_DATA_H__
#define __POSIX_DATA_H__

#include "tcpip/tcp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t socket_fd;
} wiznet_data_t;

#ifdef __cplusplus
}
#endif

#endif
