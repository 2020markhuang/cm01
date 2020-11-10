#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "MyCommon.h"
#include "MyTimer.h"
#include "MyBuzzer.h"
#include "WatchTask.h"
#ifdef SUPPORT_MY_BUZZER
#define BEEPER_GPIO_PIN    HAL_GPIO_11
#define BEEPER_GPIO_MODE  HAL_GPIO_11_PWM0

void buzzer_on(void){
    	U32 total_count = 0;
    	U32 duty_cycle = 0;
	buzzer_printf("buzzer_on\r\n");
	hal_gpio_init(BEEPER_GPIO_PIN);
	hal_pinmux_set_function(BEEPER_GPIO_PIN, BEEPER_GPIO_MODE);
	hal_pwm_init(HAL_PWM_0, HAL_PWM_CLOCK_13MHZ);
	hal_pwm_set_frequency(HAL_PWM_0, BUZZER_DEFAULT_FREQ, &total_count);
	duty_cycle = (total_count * 50) / 100;
	hal_pwm_set_duty_cycle(HAL_PWM_0, duty_cycle);
	hal_pwm_start(HAL_PWM_0);
	hal_gpio_deinit(BEEPER_GPIO_PIN);
}

void buzzer_off(void){
	buzzer_printf("buzzer_off\r\n");
	hal_gpio_init(BEEPER_GPIO_PIN);
	hal_pinmux_set_function(BEEPER_GPIO_PIN, HAL_GPIO_11_GPIO11);
	hal_gpio_set_direction(BEEPER_GPIO_PIN, HAL_GPIO_DIRECTION_OUTPUT);
	hal_gpio_set_output(BEEPER_GPIO_PIN, HAL_GPIO_DATA_LOW);	
    hal_gpio_deinit(BEEPER_GPIO_PIN);
}

void buzzer_power_on(void){
	buzzer_printf("buzzer_power_on\r\n");
	setBuzzerWorktime(BUZZER_500MS);
	setBuzzerWorkRepent(BUZZER_REPENT);
	watch_task_post_message(WATCH_MSG_IO_PROCESS, IO_STEP_BUZZER, 0);
}

static void onBuzzerRepeatByTime(int iTime,int repent){ 
	switch(repent){
		case BUZZER_REPENT:
			buzzer_on();
			startMyTimer(TIMER_ID_BUZZER, iTime, buzzer_off);		
			break;
		case BUZZER_REPENT_ONE:	
			break;
		case BUZZER_REPENT_TWO:	
			break;	
	}
}

void on_buzzer_mode_control(void){
	//buzzer_printf("on_buzzer_mode_control\r\n");
	int mode = getBuzzerWorktime();
	switch(mode){
		case BUZZER_OFF:
			buzzer_off();
			break;
		case BUZZER_ON:
			buzzer_on();
			break;
		case BUZZER_500MS:
			onBuzzerRepeatByTime(500,getBuzzerWorkRepent());	
			break;
		case BUZZER_1S:
			onBuzzerRepeatByTime(1000,getBuzzerWorkRepent());	
			break;
		case BUZZER_IN_TURN:	
			break;
	}
}
#endif
