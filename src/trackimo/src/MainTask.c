#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "timers.h"
#include "MyDefine.h"
#include "MessageSystem.h"
#include "MyKey.h"
#include "DriverInterface.h"
#include "MyCommon.h"
#include "MyWifi.h"
#include "MyGPS.h"
#include "MyLed.h"
#include "My4G.h"
#include "TrackerQueue.h"
#include "Module4GTask.h"
#include "MyBuzzer.h"
#include "MygSensor.h"
#include "MyBLE.h"
#include "WatchTask.h"
#include "MyTimer.h"
#include "MyTest.h"
#include "TrackerCfg.h"
#include "TrackimoFrame.h"
#include "wifi_host_api.h"
#include "wifi_host_private_api.h"
#include "MainTask.h"
#include "TrackerMain.h"
#include "MyFile.h"
#include "main.h"
#include "hal_sleep_manager.h"
static int MOVING_ALARM_COUNT = 0;//²»ÄÜÌ«ÁéÃô
#define MAIN_TASK_EVENT_QUEUE_LENGTH 10
static const int task = TASK_MAIN;

void main_task_post_message(int msg, U32 wPara, int lpPara){
	MyPostMessage(task, task, msg, wPara, lpPara);
}

static void onErrorEvent(int event, int para){
	switch(event){
		case ERROR_EVENT_GPS_SIGNAL_LOST:
			break;
	}	
}

static void ongSensorEvent(int event, int para){
	switch(event){
		case GSENSOR_EVENT_FALL:
			maintask_printf("gsensor fall\r\n");
			MOVING_ALARM_COUNT ++;
			if(MOVING_ALARM_COUNT > 10)
			{
				MOVING_ALARM_COUNT = 0;
				setViburatorWorkmode(TRUE);
			}
			else{
			}
			break;
	}
}

static void onBLEEvent(int event, int para){
	switch(event){
		case BLE_EVENT_CONNECTED:
			break;
		case BLE_EVENT_DISCONNECTED:
			break;
		case BLE_EVENT_SEND_OVER:
			break;
		case BLE_EVENT_NEW_DATA_INCOMING:
			break;
	}	
}

static void onExitSystemSleep(void){
	system_exit_sleep();
}

static void onEntrySystemSleep(void){
	system_entry_sleep();
}

static void onSystemEvent(int event, int para){
	switch(event){
		case SYSTEM_EVENT_LOW_POWER:
			break;
		case SYSTEM_EVENT_SHUTDOWN:
			 hal_sleep_manager_enter_power_off_mode();		
			break;
		case SYSTEM_EVENT_ENTRY_SLEEP:
			onEntrySystemSleep();
			break;
		case SYSTEM_EVENT_EXIT_SLEEP:
			onExitSystemSleep();
			break;
	}
}

static void onKeyEvent(int key, int event){
	  if((event == KEYEVENT_RELEASE)&&(key == MYKEY_POWER))
       	  {
               if(setDoubleClick())
               	{
               	event = KEYEVENT_DOUBLECLICK;
               	}
       	  }
	pfKeyCB fun = getMyKeyEvent(key, event);
	if(fun)fun();
}

void startWifiScan(void){
	maintask_printf("startWifi\r\n");
	#ifdef SUPPORT_MY_WIFI
	if(!isLwipOn()){
		lwip_on();
	}
	wifi_start_scan();
	#endif			
}

void closeWifiScan(void){
	maintask_printf("closeWifiScan\r\n");
	#ifdef SUPPORT_MY_WIFI
	wifi_stop_scan();
	if(isLwipOn()){
		//lwip_off();
	}
	if(isWifiOn()){
		wifi_off();
	}
	#endif			
}
 
static void onWifiEvent(int event, int para){
	switch(event){
		case WIFI_EVENT_INIT_DONE:
			startWifiScan();		
			break;
		case WIFI_EVENT_CONNECTED:
			break;
		case WIFI_EVENT_DISCONNECTED:
			break;
		case WIFI_EVENT_SCAN_COMPLETE:
			#ifndef MTK_WIFI_AT_COMMAND_ENABLE
			closeWifiScan();
			#endif
			break;
	}
}

static void on4GEvent(int msg, int iPara){
	switch(msg){
		case MODULE_4G_EVENT_POWER_ON:
			break;
		case MODULE_4G_EVENT_RESTART_OK:
			break;
		case MODULE_4G_EVENT_SEND_GPS_DATA:
			main_task_post_message(MAIN_MSG_SYSTEM_EVENT, SYSTEM_EVENT_EXIT_SLEEP, 0);
			break;
		case MODULE_4G_EVENT_SEND_OVER:
			main_task_post_message(MAIN_MSG_SYSTEM_EVENT, SYSTEM_EVENT_ENTRY_SLEEP, 0);
			break;
		case MODULE_4G_EVENT_SEND_FAILURE:
			break;
		case MODULE_4G_EVENT_NEW_DATA_INCOMING:
			#ifdef SUPPORT_MY_4G
			module_check_enable_to_send();
			#endif
			break;
		case MODULE_4G_EVENT_SMS_PARSE:
			Tracker_SMS_parse(iPara);
			break;		
	}
}

static void onMainTaskGPSTimeCalibration(int utm){
	structUTC * pUTM = (structUTC * )(&utm);
	structTimeStamp stamp;

	maintask_printf("onMainTaskGPSTimeCalibration:%d,%d,%d,%d,%d,%d\r\n", pUTM->year, pUTM->month, pUTM->day, pUTM->hour, pUTM->minute, pUTM->second);
	stamp = getTimeStampByPara(pUTM->year + 2000, pUTM->month, pUTM->day, pUTM->hour, pUTM->minute, pUTM->second, 0);
	setCurrentHardwareTime(stamp);
}
static void onGPSEvent(int msg, int iPara){
	switch(msg){
		case GPS_EVENT_LOCATED:
			#ifdef SUPPORT_MY_GPS
			GPS_POWEROFF();
			#endif	
			break;
		case GPS_EVENT_NEW_DATA:
			//memcpy(getGPSInfo(), pGPSData, sizeof(trk_comm_gps_repoter_info));
			//tracking_gps_send_device(pGPSData);
			break;
		case GPS_EVENT_TIME_CALIBRATE:
			onMainTaskGPSTimeCalibration(iPara);
		break;	
	}
}

static void onMyTimerWelcom(void){
	printf("onMyTimerWelcom\r\n");
	#ifdef SUPPORT_MY_4G
	module_4g_driver_init();
	#endif	
	#ifdef MTK_WIFI_AT_COMMAND_ENABLE
	wifi_on();
	wifi_atci_example_init();
	#endif
	#ifdef SUPPORT_MY_LED
	led_power_on();
	#endif
	#ifdef SUPPORT_MY_BUZZER
	buzzer_power_on();
	#endif
	main_task_post_message(MAIN_MSG_POWERSTEP_EVENT, POWER_STEP_WELCOM, 0);
}

static void onPowerStep(int iStep, int iPara){
	switch(iStep){
		case POWER_STEP_WELCOM:
			if(isFirstPowerOn())
			{	
				setFirstPowerOn(FALSE);
				tracker_start_work();
			}
			break;
		case POWER_STEP_TRACKIMO_WORK:
			Tracker_network_start();
			break;	
	}
}

static void onBatteryEvent(int msg, int iPara)
{
	BatteryEventStruct * pBatteryData = (BatteryEventStruct *)iPara;
	setBatteryCapacity(pBatteryData->capacity);
	set_ischarging(pBatteryData->charger_status);
	setBatteryLevel(pBatteryData->capacity_level);
	maintask_printf("getBatteryCapacity=%d,is_charging=%d,GetBatteryLevel=%d\r\n",getBatteryCapacity(),is_charging(),GetBatteryLevel());
}

static void main_task_process(MYMSG * pMsg){
	if(!pMsg)return;
	//maintask_printf("main_task_process:%d,%d,%d\r\n", (int)pMsg->id, (int)pMsg->wParam, pMsg->lpParam);
	switch(pMsg->id){
		case MAIN_MSG_KEY_EVENT:
			onKeyEvent(pMsg->wParam, pMsg->lpParam);
			break;
		case MAIN_MSG_ERROR_EVENT:
			onErrorEvent(pMsg->wParam, pMsg->lpParam);
			break;
		case MAIN_MSG_SYSTEM_EVENT:
			onSystemEvent(pMsg->wParam, pMsg->lpParam);
			break;
		case MAIN_MSG_POWERSTEP_EVENT:
			onPowerStep(pMsg->wParam, pMsg->lpParam);
			break;			
		case MAIN_MSG_WIFI_EVENT:
			onWifiEvent(pMsg->wParam, pMsg->lpParam);
			break;
		case MAIN_MSG_GPS_EVENT:
			onGPSEvent(pMsg->wParam, pMsg->lpParam);
			break;
		case MAIN_MSG_BLE_EVENT:
			onBLEEvent(pMsg->wParam, pMsg->lpParam);
			break;
		case MAIN_MSG_GSENSOR_EVENT:
			ongSensorEvent(pMsg->wParam, pMsg->lpParam);
			break;
		case MAIN_MSG_TP_EVENT:
			break;
		case MAIN_MSG_4G_EVENT:
			on4GEvent(pMsg->wParam, pMsg->lpParam);
			break;
		case MSG_TYPE_BATTERY_EVENT:
			onBatteryEvent(pMsg->wParam, pMsg->lpParam);
			break;
	}
}

static void registeNormalKeyEvent(void){
	clearMyKeyEvents();
	setMyKeyEvent(MYKEY_POWER, KEYEVENT_DOUBLECLICK, tracker_sos_key_longpress);
	#ifdef SUPPORT_MY_LED
	setMyKeyEvent(MYKEY_POWER, KEYEVENT_LONGPRESS, led_power_off);
	setMyKeyEvent(MYKEY_POWER, KEYEVENT_PRESS, network_led_status);
	#endif 
	setMyKeyEvent(MYKEY_ENTRY, KEYEVENT_LONGPRESS, tracker_sos_key_longpress);
	#ifdef SUPPORT_MY_LED
	setMyKeyEvent(MYKEY_ENTRY, KEYEVENT_RELEASE, network_led_status);
	#endif
}

void main_task( void *pvParameters ){
	maintask_printf("main_task start\r\n");
	registeNormalKeyEvent();
	startMyTimer(TIMER_ID_STARTUP, 200, onMyTimerWelcom);
 	MyMessageStandTask(task, MAIN_TASK_EVENT_QUEUE_LENGTH, main_task_process);
}


