#ifndef MY_TP_FHQ_20190722_H
#define MY_TP_FHQ_20190722_H

#include "MyDefine.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

typedef enum {
	TP_EVENT_POWER_ON,
	TP_EVENT_POWER_OFF,
	TP_EVENT_MAX
} enumTPEvent;

void tp_driver_init(void);
void tp_on(void);
void tp_off(void);

#if defined(__cplusplus)
}
#endif 

#endif //MY_TP_FHQ_20190722_H



