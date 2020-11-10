#ifndef MY_GSENSOR_FHQ_20190722_H
#define MY_GSENSOR_FHQ_20190722_H

#include "MyDefine.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

typedef enum {
	GSENSOR_EVENT_POWER_ON,
	GSENSOR_EVENT_POWER_OFF,
	GSENSOR_EVENT_FALL,
	GSENSOR_EVENT_MAX
} enumGSensorEvent;

void gSensor_driver_init(void);
void gSensor_on(void);
void gSensor_off(void);
void ongSensorInt(int type, int para);
void gSensor_get_data(I16 * pX, I16 * pY, I16 * pZ);
void CheckTurnOnOffGsensor(void);
void fence_speed_alarm_turnon_gsensor(void);	
void fence_speed_alarm_turnoff_gsensor(void);
void moving_alarm_turnon_gsensor(void);
void moving_alarm_turnoff_gsensor(void);
#if defined(__cplusplus)
}
#endif 

#endif //MY_GSENSOR_FHQ_20190722_H



