#ifndef __MSP430_DATA_H__
#define __MSP430_DATA_H__

#include "SimplelinkWifi/TCPclient.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	WiFiClient* socket_ptr;
} msp430_data_t;

#ifdef __cplusplus
}
#endif

#endif
