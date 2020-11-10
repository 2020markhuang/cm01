#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "MessageSystem.h"
#include "MyCommon.h"
#include "MyLed.h"
#include "MyBuzzer.h"
#include "MyViburator.h"
#include "MyBLE.h"
#include "MyKey.h"
#include "MygSensor.h"
#include "MyGPS.h"
#include "WatchTask.h"

#define WATCH_TASK_EVENT_QUEUE_LENGTH 10

static const int task = TASK_WATCH;
void watch_task_post_message(int msg, U32 wPara, int lpPara){
	MyPostMessage(task, task, msg, wPara, lpPara);
}

static void onIOProcess(int wPara, int lpPara){
	switch(wPara){
	case IO_STEP_KEY:	
		break;
	case IO_STEP_LED:	
		#ifdef SUPPORT_MY_LED
		on_led_mode_control();
		#endif
		break;	
	case IO_STEP_BUZZER:	
		#ifdef SUPPORT_MY_BUZZER
		on_buzzer_mode_control();
		#endif
		break;	
	}
}

#ifdef SUPPORT_MY_GPS
static void onGPSProcessData(int wPara, int lpPara){
	char buffer[256];
	//watchtask_printf("onGPSProcessData\r\n");
	if(gps_read_data(buffer, sizeof(buffer)) > 0) watch_task_post_message(WATCH_MSG_GPS_INT, 0, 0);

}
#endif
static void watch_task_process(MYMSG * pMsg){
	int iTime = 0;
	if(!pMsg)return;
	//watchtask_printf("watch_task_process:%d,%d,%d\r\n", (int)pMsg->id, (int)pMsg->wParam, pMsg->lpParam);
	switch(pMsg->id){
		case WATCH_MSG_IO_PROCESS:
			onIOProcess(pMsg->wParam, pMsg->lpParam);
			break;
		case WATCH_MSG_EXIT_SLEEP:
			watch_task_post_message(WATCH_MSG_IO_PROCESS, 0, 0);
			break;
		#ifdef SUPPORT_MY_GSENSOR
		case WATCH_MSG_GSENSOR_INT:
			ongSensorInt(pMsg->wParam, pMsg->lpParam);
			break;
		#endif		
		#ifdef SUPPORT_MY_GPS
		case WATCH_MSG_GPS_INT:
			onGPSProcessData(pMsg->wParam, pMsg->lpParam);
			break;
		#endif			
	}
}

void watch_task(void *pvParameters) {
	watchtask_printf("watch_task\r\n");
 	MyMessageStandTask(task, WATCH_TASK_EVENT_QUEUE_LENGTH, watch_task_process);

}
