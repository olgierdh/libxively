#ifndef __MSP430_DATA_H__
#define __MSP430_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "SimplelinkWifi/wifi.h" 
//#include "SimplelinkWifi/TCPclient.h"

typedef struct
{
	WiFiClient* socket_ptr;
} msp430_data_t;

#ifdef __cplusplus
}
#endif

#endif
