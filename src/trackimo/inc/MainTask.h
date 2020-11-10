#ifndef MAIN_TASK_20190722_FHQ_H
#define MAIN_TASK_20190722_FHQ_H

#include "MyDefine.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

typedef enum{
	POWER_STEP_WELCOM,
	POWER_STEP_GPS_START,
	POWER_STEP_TRACKIMO_WORK,
	POWER_STEP_BOOT_FINISH,
	POWER_STEP_MAX
}enumPowerStep;


typedef enum {
	ERROR_EVENT_GPS_SIGNAL_LOST,
	ERROR_EVENT_MAX
} enumErrorEvent;



typedef enum {
	SYSTEM_EVENT_LOW_POWER,
	SYSTEM_EVENT_SHUTDOWN,
	SYSTEM_EVENT_CHARGE_CONNECTED,
	SYSTEM_EVENT_CHARGE_DISCONNECTED,
	SYSTEM_EVENT_ENTRY_SLEEP,
	SYSTEM_EVENT_EXIT_SLEEP,
	SYSTEM_EVENT_MAX,
} enumSystemEvent;



typedef enum {
	MAIN_MSG_KEY_EVENT,
	MAIN_MSG_GPS_EVENT,
	MAIN_MSG_BLE_EVENT,
	MAIN_MSG_GSENSOR_EVENT,
	MAIN_MSG_WIFI_EVENT,
	MAIN_MSG_TP_EVENT,		//5
	MAIN_MSG_4G_EVENT,
	MAIN_MSG_ERROR_EVENT,
	MAIN_MSG_SYSTEM_EVENT,
	MAIN_MSG_POWERSTEP_EVENT,
	MAIN_MSG_STATUS_EVENT,
	MSG_TYPE_BATTERY_EVENT,
	MAIN_MSG_MAX
 } enumMainMsg;


void main_task_post_message(int msg, U32 wPara, int lpPara);
void main_task( void *pvParameters );
void closeWifiScan(void);
void startWifiScan(void);
#if defined(__cplusplus)
}
#endif 

#endif //MAIN_TASK_20190722_FHQ_H



