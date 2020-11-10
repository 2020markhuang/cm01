#ifndef MY_LED_FHQ_20190722_H
#define MY_LED_FHQ_20190722_H

#include "MyDefine.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
typedef enum{
LED_REPENT = 0,
LED_REPENT_ONCE,
LED_REPENT_TWO,
LED_REPENT_THERE,
LED_BATTEY_FULL,
LED_REPENT_INTURN,
LED_REPENT_MAX
}enumLEDRepent;

typedef enum{
LED_ALL_OFF,
LED_ALL_ON,
LED_FLASH_1S,
LED_FLASH_5S,
LED_FLASH_10S,
LED_FLASH_IN_TURN,
LED_FLASH_BREATH,
LED_STATUS_MAX
}enumLEDStatus;

typedef enum{
LED_BIT_R,
LED_BIT_G,
LED_BIT_B,
LED_BIT_MAX	
}enumLEDBit;

typedef enum  {
    LED_FLAG_R = 0x01,
    LED_FLAG_G = 0x02,
    LED_FLAG_B =  0x04,
}enumLEDFlag;

void led_driver_init(void);
void led_all_on(void);
void led_all_off(void);
void on_led_mode_control(void);
void led_power_on(void);
void led_power_off(void);
void led_poweroff_repent(void);
void led_battery_full(void);
void led_battery_full_repent(void);
void led_battery_ischarging(void);
void led_battery_charging_repent(void);
void red_led_on(void);
void green_led_on(void);
void network_led_status(void);
#if defined(__cplusplus)
}
#endif 

#endif //MY_LED_FHQ_20190722_H



