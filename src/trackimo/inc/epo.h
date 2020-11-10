#ifndef __EPO_H_20200803__
#define __EPO_H_20200803__

#include "WatchDateTime.h"

#if defined(__cplusplus)
extern "C" {     
#endif


BOOL epo_clear_data(void);
BOOL isNeedCommitEPOClear(void);
void resetNeedCommitEPOClearFlag(void);
BOOL epo_time_is_valid(structTimeStamp time);
BOOL epo_write_data(int iOffset, char * pBuf, int iSize);
void epo_send_utc_data(void);
void epo_send_sat_data(void);

#if defined(__cplusplus)
}
#endif 


#endif	//__EPO_H_20200803__
