#ifndef MY_VIBURATOR_FHQ_20190722_H
#define MY_VIBURATOR_FHQ_20190722_H

#include "MyDefine.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif


typedef enum{
VIBURATOR_OFF,
VIBURATOR_ON,
VIBURATOR_SHORT,
VIBURATOR_LONG,
VIBURATOR_BREATH,
VIBURATOR_STATUS_MAX
}enumViburatorStatus;


void viburator_driver_init(void);
void viburator_on(void);
void viburator_off(void);
void on_viburator_mode_control(void);


#if defined(__cplusplus)
}
#endif 

#endif //MY_VIBURATOR_FHQ_20190722_H



