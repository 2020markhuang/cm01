#ifndef MY_TIMER_H_20190903
#define MY_TIMER_H_20190903

#include "MyDefine.h"
#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

typedef enum{
TIMER_ID_STARTUP,
TIMER_ID_SHUTDOWN,
TIMER_ID_KEY_REPEAT,
TRACKER_WORK_STATUS_TIMER,
TRACKER_NET_WORK_TIMER,
TRACKER_WORK_MIN_TIMER,
TRACKER_WIFI_TIMER_1,
TRACKER_WIFI_TIMER_2,
TRACKER_WIFI_TIMER_3,
TRACKER_GPS_START_WITH_MEASUREMENT,
TRACKER_GPS_FIX_WITH_MEASUREMENT_WAIT,
TRACKER_GPS_START_COMMAND,
TRACKER_GPS_FIX_COMMAND_WAIT,
TIMER_ID_LED,
TIMER_ID_LED_ON,
TIMER_ID_LED_OFF,
TIMER_ID_BUZZER,
TIMER_ID_READ_SMS,
MY_TIMER_31,
MY_TIMER_34,
MY_TIMER_33,
MY_TIMER_51,
MY_TIMER_54,
MY_TIMER_55,
MY_TIMER_56,
MY_TIMER_57,
MY_TIMER_58,
MY_TIMER_59,
MY_TIMER_60,
MY_TIMER_61,
MY_TIMER_62,
MY_TIMER_63,
MY_TIMER_64,
MY_TIMER_65,
MY_TIMER_66,
MY_TIMER_67,
MY_TIMER_84,
MY_TIMER_88,
MINI_MY_TIMER_9,
MINI_MY_TIMER_10,
MINI_MY_TIMER_11,
MINI_MY_TIMER_14,
MINI_MY_TIMER_15,
TIMER_MAX
}enumMyTimer;


/*
	功能作用:初始化定时器
	参数说明:无
	返 回 值:无
	开发人员:付华强
*/
void initMyTimer(void);
void deinitMyTimer(void);
/*
	功能作用:开启定时器
	参数说明:id		定时器id
			 time	需要等待的触发时间(毫秒)
			 cb		回调函数
			 user_data	回调时的用户数据
	返 回 值:无
	开发人员:付华强
*/
void startMyTimer(int id, int time, pfDefaultFunction cb);
/*
	功能作用:关闭定时器
	参数说明:id		定时器id
	返 回 值:无
	开发人员:付华强
*/
void stopMyTimer(int id);



#if defined(__cplusplus)
}
#endif 

#endif //MY_TIMER_H_20190903



