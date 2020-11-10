#include <stdio.h>
#include "MyCommon.h"
#include "My4G.h"
#include "MyBLE.h"
#include "MyBuzzer.h"
#include "MyGPS.h"
#include "MygSensor.h"
#include "MyTP.h"
#include "MyLed.h"
#include "MyKey.h"
#include "MyViburator.h"
#include "MyWifi.h"
#include "MyTimer.h"
#include "MyClock.h"
#include "battery_management.h"
#include "DriverInterface.h"
extern uint8_t main_sleep_handle; 
void main_entry_sleep(void){
	module4g_printf("main_entry_sleep=%d,%d\r\n",hal_sleep_manager_is_sleep_locked(),hal_sleep_manager_get_lock_status() & (1 << main_sleep_handle));
	if(hal_sleep_manager_get_lock_status() & (1 << main_sleep_handle)) 
	{
		hal_sleep_manager_unlock_sleep(main_sleep_handle);
		module4g_printf("main_entry_sleep=%d,%d\r\n",hal_sleep_manager_is_sleep_locked(),hal_sleep_manager_get_lock_status() & (1 << main_sleep_handle));
	}
}
void main_exit_sleep(void){
	module4g_printf("main_exit_sleep=%d,%d\r\n",hal_sleep_manager_is_sleep_locked(),hal_sleep_manager_get_lock_status() & (1 << main_sleep_handle));
	hal_sleep_manager_lock_sleep(main_sleep_handle);
}
void driver_init(void){
	clock_init();
	#ifdef SUPPORT_MY_TIMER
	initMyTimer();
	#endif
	#ifdef SUPPORT_MY_BLE
	ble_driver_init();
	#endif
	#ifdef SUPPORT_MY_GPS
	gps_driver_init();
	#endif
	#ifdef SUPPORT_MY_KEY
	my_key_driver_init();
	#endif
	#ifdef SUPPORT_MY_LED
	led_driver_init();
	#endif
	#ifdef SUPPORT_MY_TP
	tp_driver_init();
	#endif
	#ifdef SUPPORT_MY_VIBURATOR
	viburator_driver_init();
	#endif
	#ifdef SUPPORT_MY_WIFI
	wifi_driver_init();
	#endif
	#ifdef SUPPORT_MY_GSENSOR
	gSensor_driver_init();
	gSensor_on();
	#endif
	Tracker_init();
}

void system_entry_sleep(void){
	printf("system_entry_sleep\r\n");
	setSystemSleep(TRUE);
	main_entry_sleep();
	#ifdef SUPPORT_MY_4G
	module_4g_entry_sleep();
	#endif
	#ifdef SUPPORT_MY_BLE
	ble_off();
	#endif
	#ifdef SUPPORT_MY_BUZZER
	buzzer_off();
	#endif
	#ifdef SUPPORT_MY_GPS
	gps_entry_sleep();
	#endif
	#ifdef SUPPORT_MY_LED
	led_all_off();
	#endif
	#ifdef SUPPORT_MY_WIFI
	closeWifiScan();
	#endif
	#ifdef SUPPORT_WATCH_DOG			
	hal_wdt_disable(HAL_WDT_FEED_MAGIC);
	#endif
	//hal_sleep_manager_enter_sleep_mode(HAL_SLEEP_MODE_SLEEP);
}

void system_exit_sleep(void){
	printf("system_exit_sleep\r\n");
	setSystemSleep(FALSE);
	main_exit_sleep();
	#ifdef SUPPORT_MY_4G
	module_4g_exit_sleep();
	#endif
	#ifdef SUPPORT_MY_BLE
	#endif
	#ifdef SUPPORT_MY_BUZZER
	#endif
	#ifdef SUPPORT_MY_GPS
	gps_exit_sleep();
	#endif
	#ifdef SUPPORT_MY_GSENSOR
	#endif
	#ifdef SUPPORT_MY_LED
	#endif
	#ifdef SUPPORT_MY_WIFI
	#endif
	#ifdef SUPPORT_WATCH_DOG			
	hal_wdt_enable(HAL_WDT_FEED_MAGIC);
	hal_wdt_feed(HAL_WDT_FEED_MAGIC);
	#endif	
}




