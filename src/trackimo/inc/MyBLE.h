#ifndef MY_BLE_FHQ_20190722_H
#define MY_BLE_FHQ_20190722_H

#include "MyDefine.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
typedef enum {
	BLE_EVENT_POWER_ON,
	BLE_EVENT_POWER_OFF,
	BLE_EVENT_CONNECTED,
	BLE_EVENT_DISCONNECTED,
	BLE_EVENT_SEND_OVER,
	BLE_EVENT_NEW_DATA_INCOMING,
	BLE_EVENT_MAX
} enumBLEEvent;
void ble_driver_init(void);
void ble_on(void);
void ble_off(void);

#if defined(__cplusplus)
}
#endif 

#endif //MY_BLE_FHQ_20190722_H



