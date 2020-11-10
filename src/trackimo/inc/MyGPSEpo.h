#ifndef __MY_GPSEPO_FHQ_20200204_H__
#define __MY_GPSEPO_FHQ_20200204_H__

#include "MyDefine.h"

#if defined(__cplusplus)
extern "C" {     
#endif
#ifdef SUPPORT_MY_GPSEPO
BOOL gps_epo_is_need_update(void);
void gps_epo_refresh(void);

#endif

#if defined(__cplusplus)
}
#endif 


#endif //__MY_GPSEPO_FHQ_20200204_H__



