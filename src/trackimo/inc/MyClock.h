#ifndef __MY_CLOCK_FHQ_20200204_H__
#define __MY_CLOCK_FHQ_20200204_H__

#if defined(__cplusplus)
extern "C" {     
#endif
#include "WatchDateTime.h"

void clock_init(void);
structTimeStamp getCurrentHardwareTime(void);
void setCurrentHardwareTime(structTimeStamp stamp);


#if defined(__cplusplus)
}
#endif 

#endif //__MY_CLOCK_FHQ_20200204_H__



