#include <stddef.h>
#include <string.h>
#include <stdio.h>


#include "WatchDateTime.h"
#include "MyClock.h"
#include "epo.h"
#include "MyGPSEpo.h"


BOOL gps_epo_is_need_update(void){
	BOOL bRet = FALSE;
	structTimeStamp time = getCurrentTime();
	//printf("zzzzz:%d,%d,%d,%d,%d\r\n", getYearOfTime(time),getMonthOfTime(time),getDayOfTime(time),getHourOfTime(time),getMinuteOfTime(time));
	time = addTimeStamp(time, 1000 * 3600 * 3);
	bRet = !epo_time_is_valid(time);
	printf("gps_epo_is_need_update:%d\r\n", bRet);
	return bRet;
}

void gps_epo_refresh(void){
	int i;
	int iLoop,iLen;
	char * pData = getDownloadFileBuffer();
	const int iBufSize = 4096;
	int iOffset = 0;

	iLen = getDownloadFileLength();
	if(iLen <= 0)return;
	iLoop = iLen / iBufSize;
	for(i = 0; i < iLoop; i++, iOffset += iBufSize, pData += iBufSize){
		epo_write_data(iOffset, (U8 *)pData, iBufSize);
	}
	iLen = iLen % iBufSize;
	if(iLen){
		epo_write_data(iOffset, (U8 *)pData, iLen);
	}	
}


