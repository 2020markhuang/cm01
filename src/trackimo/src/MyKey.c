#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "MainTask.h"
#include "WatchTask.h"
#include "MyKey.h"
#include "MyCommon.h"
#include "TrackimoFrame.h"
#include "MyBuzzer.h"
#include "MyTimer.h"
#ifdef SUPPORT_MY_KEY
static int static_press_power_time = 0;
static pfKeyCB static_arrayKeyEventsFunc[MYKEY_MAX][KEYEVENT_MAX];
void clearDoubleClick(void)
{
	static_press_power_time = 0;
}
BOOL setDoubleClick(void)
{
	static_press_power_time ++;
	stopMyTimer(TIMER_ID_KEY_REPEAT);
	startMyTimer(TIMER_ID_KEY_REPEAT, 1000, clearDoubleClick);
	printf("static_press_power_time=%d\r\n",static_press_power_time);
	if(static_press_power_time >= 3)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}
void tracker_sos_key_longpress(void)
{
	printf("............................tracker_sos_key_longpress................................\r\n");
	if(is4GNetOK)
	{	
	 tracker_sos_send();
	}
	else
	{
	printf("sos: not full service\r\n");
	}
}	

pfKeyCB getMyKeyEvent(int key, int event){
	if((key < 0) || (key >= MYKEY_MAX))return NULL;
	if((event < 0) || (event >= KEYEVENT_MAX))return NULL;
	return static_arrayKeyEventsFunc[key][event];
}

void clearMyKeyEvents(void){
	memset(static_arrayKeyEventsFunc, 0, sizeof(static_arrayKeyEventsFunc));
}

BOOL setMyKeyEvent(int key, int event, pfKeyCB func){
	if((key < 0) || (key >= MYKEY_MAX))return FALSE;
	if((event < 0) || (event >= KEYEVENT_MAX))return FALSE;
	static_arrayKeyEventsFunc[key][event] = func;
	return TRUE;
}
static int getMyIntKeyEvent(int state){
	int iEvent = -1;
	switch(state){
		case  0:  //RELEASE
			iEvent = KEYEVENT_RELEASE;
			break;
		case  1:  //PRESS
			iEvent = KEYEVENT_PRESS;
			break;
		case  2:  //LONG_PRESS
			iEvent = KEYEVENT_LONGPRESS;
			break;
		case  3:  //REPEAT
			iEvent = KEYEVENT_RELEASE;
			break;
	}
	return iEvent;
}

static int getMyKeyValue(int hk){
	int key = -1;

	switch(hk){
		case 0:
			break;
		case 254:
			key = MYKEY_ENTRY;
			break;
	}
	return key;
}
static void keypad_handler(void){
	printf("keypad_handler\r\n");
	hal_keypad_event_t          keypad_event;
	int    ret = 0;
	int iEvent = -1;
	int iKey = -1;
	int key_data;	
	ret = hal_keypad_get_key(&keypad_event);
	key_data   = keypad_custom_translate_keydata(keypad_event.key_data);
	printf("keypad_handler11111 = %d\r\n",key_data);
	iKey = getMyKeyValue(key_data);
	printf("keypad_handler22222 = %d\r\n",iKey);
	iEvent = getMyIntKeyEvent(keypad_event.state);
	printf("keypad_handler33333 = %d\r\n",iEvent);
	main_task_post_message(MAIN_MSG_KEY_EVENT, iKey, iEvent);
}

#ifdef HAL_KEYPAD_FEATURE_POWERKEY
static void powerkey_handler(void){
	printf("powerkey_handler\r\n");
	int iEvent = -1;
	hal_keypad_powerkey_event_t powekey_event;
	hal_keypad_powerkey_get_key(&powekey_event);
	iEvent = getMyIntKeyEvent(powekey_event.state);
	printf("powerkey_handler55555 = %d\r\n",iEvent);
	main_task_post_message(MAIN_MSG_KEY_EVENT, MYKEY_POWER, iEvent);
}
#endif
void my_key_driver_init(void){
	if (!keypad_custom_init())return;
	hal_keypad_enable();
	hal_keypad_register_callback((hal_powerkey_callback_t)keypad_handler, NULL);
	#ifdef HAL_KEYPAD_FEATURE_POWERKEY
	hal_keypad_powerkey_register_callback((hal_powerkey_callback_t)powerkey_handler, NULL);
	#endif
}
#endif
