#ifndef MY_BUZZER_FHQ_20190722_H
#define MY_BUZZER_FHQ_20190722_H

#include "MyDefine.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
#define BUZZER_DEFAULT_FREQ 3000

typedef enum{
BUZZER_REPENT,
BUZZER_REPENT_ONE,
BUZZER_REPENT_TWO,
BUZZER_REPENT_THREE,
BUZZER_REPENT_MAX
}enumBuzzerRepent;

typedef enum{
BUZZER_OFF,
BUZZER_ON,
BUZZER_500MS,
BUZZER_1S,
BUZZER_IN_TURN,
BUZZER_STATUS_MAX
}enumBuzzerStatus;

void buzzer_on(void);
void buzzer_off(void);
void buzzer_power_on(void);
void on_buzzer_mode_control(void);
#if defined(__cplusplus)
}
#endif 

#endif //MY_BUZZER_FHQ_20190722_H



