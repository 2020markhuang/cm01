#include <stddef.h>
#include <string.h>
#include "MyCommon.h"
#include "MyLed.h"
#include "mt25x3_hdk_backlight.h"
#include "MyTimer.h"
#include "WatchTask.h"
#include "MainTask.h"
#ifdef SUPPORT_MY_LED
#define LED_R  HAL_GPIO_42
#define LED_G  HAL_GPIO_43
#define LED_B  HAL_GPIO_40
#define LED_R_MODE  HAL_GPIO_42_GPIO42
#define LED_G_MODE  HAL_GPIO_43_GPIO43
#define LED_B_MODE  HAL_GPIO_40_GPIO40
static unsigned int SPECIAL_FLASH_POWERDOWN_TIMES = 0;
void led_driver_init(void){
	hal_gpio_init(LED_R);
	hal_pinmux_set_function(LED_R, LED_R_MODE); 
	hal_gpio_set_direction(LED_R, HAL_GPIO_DIRECTION_OUTPUT);

	hal_gpio_init(LED_G);
	hal_pinmux_set_function(LED_G, LED_G_MODE); 
	hal_gpio_set_direction(LED_G, HAL_GPIO_DIRECTION_OUTPUT);

	hal_gpio_init(LED_B);
	hal_pinmux_set_function(LED_B, LED_B_MODE); 
	hal_gpio_set_direction(LED_B, HAL_GPIO_DIRECTION_OUTPUT);
}

static void led_switch_by_bit(int flag){
	int i;	
	led_driver_init();
	for(i = 0; i < LED_BIT_MAX; i++){
		switch(i){
			case LED_BIT_R:
				hal_gpio_set_output(LED_R, (flag & LED_FLAG_R) ? HAL_GPIO_DATA_HIGH : HAL_GPIO_DATA_LOW);	
				break;
			case LED_BIT_G:
				hal_gpio_set_output(LED_G, (flag & LED_FLAG_G) ? HAL_GPIO_DATA_HIGH : HAL_GPIO_DATA_LOW);	
				break;
			case LED_BIT_B:
				if(flag & LED_FLAG_B)
				{
				bsp_backlight_enable(1);
				}
				else
				{
				 bsp_backlight_enable(0);
				}
				break;
		}
	}
}

void led_all_on(void){
	led_switch_by_bit(LED_FLAG_R | LED_FLAG_G |LED_FLAG_B);
}

void led_all_off(void){
	stopMyTimer(TIMER_ID_LED);
	stopMyTimer(TIMER_ID_LED_ON);
	stopMyTimer(TIMER_ID_LED_OFF);
	led_switch_by_bit(0);
}

void led_power_on(void){
	printf("led_power_on\r\n");
	setLedWorkFlag(LED_FLAG_R | LED_FLAG_G | LED_FLAG_B);
	setLedWorktime(LED_FLASH_1S);
	setLedWorktimeRepent(LED_REPENT);
	watch_task_post_message(WATCH_MSG_IO_PROCESS, IO_STEP_LED, 0);
}

void led_power_off(void){
	printf("led_power_off\r\n");
	setLedWorkFlag(LED_FLAG_R);
	setLedWorktime(LED_FLASH_1S);
	setLedWorktimeRepent(LED_REPENT_THERE);
	watch_task_post_message(WATCH_MSG_IO_PROCESS, IO_STEP_LED, 0);
}

void red_led_on(void){
	printf("red_led_on\r\n");
	setLedWorkFlag(LED_FLAG_R);
	setLedWorktime(LED_FLASH_1S);
	setLedWorktimeRepent(LED_REPENT);
	watch_task_post_message(WATCH_MSG_IO_PROCESS, IO_STEP_LED, 0);
}

void green_led_on(void){
	printf("green_led_on\r\n");
	setLedWorkFlag(LED_FLAG_G);
	setLedWorktime(LED_FLASH_1S);
	setLedWorktimeRepent(LED_REPENT);
	watch_task_post_message(WATCH_MSG_IO_PROCESS, IO_STEP_LED, 0);
}

void network_led_status(void){
	printf("network_led_status\r\n");
	if(is4GNetOK())
	{
		green_led_on();
	}else{
		red_led_on();
	}
}

void led_poweroff_with_system(void){
	stopMyTimer(TIMER_ID_LED);
}

void led_poweroff_led_off(){
	led_switch_by_bit(0);
	stopMyTimer(TIMER_ID_LED_ON);
	startMyTimer(TIMER_ID_LED_ON, 500, led_poweroff_repent);
}

void led_poweroff_repent(void){
	if(SPECIAL_FLASH_POWERDOWN_TIMES == 0)
	{
		led_all_off();
		main_task_post_message(MAIN_MSG_SYSTEM_EVENT, SYSTEM_EVENT_SHUTDOWN, 0);
		return;
	}
	led_switch_by_bit(getLedWorkFlag());
	stopMyTimer(TIMER_ID_LED_OFF);
	startMyTimer(TIMER_ID_LED_OFF, 500, led_poweroff_led_off);
	if(SPECIAL_FLASH_POWERDOWN_TIMES > 0)
	{
		SPECIAL_FLASH_POWERDOWN_TIMES--;
	}
}

void led_charging_inturn_led_off(void){
	led_switch_by_bit(0);
	stopMyTimer(TIMER_ID_LED_ON);
	startMyTimer(TIMER_ID_LED_ON, 500, led_battery_charging_repent);
}

void led_battery_charging_repent(void){
	if(is_charging() != 1)
	{
		led_all_off();
		return;
	}
	led_switch_by_bit(getLedWorkFlag());
	stopMyTimer(TIMER_ID_LED_OFF);
	startMyTimer(TIMER_ID_LED_OFF, 500, led_charging_inturn_led_off);
}

void led_battery_full_repent(void){
	if(is_charging() != 1)
	{
		led_all_off();
		return;
	}
	led_switch_by_bit(getLedWorkFlag());
	stopMyTimer(TIMER_ID_LED_OFF);
	startMyTimer(TIMER_ID_LED_OFF, 500, led_battery_full_repent);
}
void led_battery_full(void){
	printf("led_battery_full\r\n");
	setLedWorkFlag(LED_FLAG_G);
	setLedWorktime(LED_FLASH_1S);
	setLedWorktimeRepent(LED_BATTEY_FULL);
	watch_task_post_message(WATCH_MSG_IO_PROCESS, IO_STEP_LED, 0);
}

void led_battery_ischarging(void){	
	setLedWorkFlag(LED_FLAG_R);
	setLedWorktime(LED_FLASH_IN_TURN);
	setLedWorktimeRepent(LED_REPENT_INTURN);
	watch_task_post_message(WATCH_MSG_IO_PROCESS, IO_STEP_LED, 0);
}

static void onLedRepeatByTime(int iTime,int repeat){
	switch(repeat){
		case LED_REPENT:
			led_switch_by_bit(getLedWorkFlag());
			startMyTimer(TIMER_ID_LED, iTime, led_all_off);
			break;
		case LED_REPENT_ONCE:
			led_switch_by_bit(getLedWorkFlag());
			startMyTimer(TIMER_ID_LED, iTime, led_all_off);
			break;
		case LED_REPENT_TWO:
			SPECIAL_FLASH_POWERDOWN_TIMES = LED_REPENT_TWO;
			led_poweroff_repent();
			break;
		case LED_REPENT_THERE:
			SPECIAL_FLASH_POWERDOWN_TIMES = LED_REPENT_THERE;
			led_poweroff_repent();
			break;
		case LED_BATTEY_FULL:
			led_battery_full_repent();
			break;
		case LED_REPENT_INTURN:
			 led_battery_charging_repent();
			break;
	}
}	

void on_led_mode_control(void){
	int mode = getLedWorktime();
	switch(mode){
		case LED_ALL_OFF:
			led_all_off();
			break;
		case LED_ALL_ON:
			led_all_on();
			break;
		case LED_FLASH_1S:
			onLedRepeatByTime(1000,getLedWorkreRepent());
			break;
		case LED_FLASH_5S:
			onLedRepeatByTime(5000,getLedWorkreRepent());
			break;
		case LED_FLASH_10S:
			onLedRepeatByTime(10000,getLedWorkreRepent());
			break;
		case LED_FLASH_BREATH:
			break;
		case LED_FLASH_IN_TURN:
			onLedRepeatByTime(1000,getLedWorkreRepent());
			break;
	
	}
}
#endif
