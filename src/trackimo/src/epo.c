#include <string.h>
#include <stdio.h>

#include "WatchDateTime.h"
#include "MyGPS.h"
#include "MyClock.h"
#include "epo.h"

#include "gnss_api.h"
#include "hal_flash.h"

#define MTKEPO_SV_NUMBER 32
#define MTKEPO_RECORD_SIZE 72
//三天数据
#define MTKEPO_SEGMENT_NUM (3 * 4)

#define EPO_START_ADDRESS 0x003F0000
#define EPO_END_ADDRESS 0x00400000
#define FLASH_BLOCK_SIZE 4096

static int static_iStartRecord = 0;
static int static_iEndRecord = 0;
static BOOL static_bClearEPO = FALSE;
static char static_epoBuffer[MTKEPO_SV_NUMBER * MTKEPO_RECORD_SIZE];

//每条数据管6个小时

static int utc_to_gnss_hour (structTimeStamp time)
{
	structTimeStamp timeBase = getTimeStampByPara(1980, 1, 6, 0, 0, 0, 0);
	structTimeStampDiff diff = getTimeDiff(time, timeBase);
	int days = getDayOfDiff(diff);
	int hour = getHourOfDiff(diff);
	return days * 24 + hour;
}
static int getEPOValidSegs(void){
	int iLen = 0;
	iLen = EPO_END_ADDRESS - EPO_START_ADDRESS;
	return iLen / (MTKEPO_RECORD_SIZE * MTKEPO_SV_NUMBER);
}
BOOL isNeedCommitEPOClear(void){
	return static_bClearEPO;
}
void resetNeedCommitEPOClearFlag(void){
	static_bClearEPO = FALSE;
}
BOOL epo_clear_data(void){
	int address = EPO_START_ADDRESS;
	while(address < EPO_END_ADDRESS){
	if(hal_flash_erase(address, HAL_FLASH_BLOCK_4K) != HAL_FLASH_STATUS_OK)return FALSE;
		address += FLASH_BLOCK_SIZE;
	}
	static_bClearEPO = TRUE;
	return TRUE;
	
}
BOOL epo_write_data(int iOffset, char * pBuf, int iSize){

	int address = EPO_START_ADDRESS + iOffset;
	
	if(!pBuf)return FALSE;
	if((iSize <= 0) || (iSize > FLASH_BLOCK_SIZE))return FALSE;
	if((address + iSize) > EPO_END_ADDRESS)return FALSE;
	
	if(hal_flash_erase(address, HAL_FLASH_BLOCK_4K) != HAL_FLASH_STATUS_OK)return FALSE;
	if(hal_flash_write(address, pBuf, iSize) != HAL_FLASH_STATUS_OK)return FALSE;
	printf("epo_write_data\r\n", iOffset, iSize);
	return TRUE;
}

static BOOL epo_read_data(char* pBuf, int iSize, int iStartRecord, int iRecordsRead){
	int offset = iStartRecord * MTKEPO_RECORD_SIZE;
	int iReadSize = iRecordsRead * MTKEPO_RECORD_SIZE;
	int address;
	
	if(!pBuf)return FALSE;
	if((iStartRecord < 0) || (iRecordsRead <= 0))return FALSE;
	if(iSize < iReadSize)return FALSE;

	address = EPO_START_ADDRESS + offset;
	if((address + iRecordsRead * MTKEPO_RECORD_SIZE) > EPO_END_ADDRESS)return FALSE;
	if(hal_flash_read(address, pBuf, iReadSize) != HAL_FLASH_STATUS_OK)return FALSE;
	return TRUE;
}

static int epo_get_segment(structTimeStamp time, char *pBuf, int iSize){
	int iSeg, iEPOGNSSHour, iCurrentGNSSHour;
	int iMaxSegs = 0;


	if(!epo_read_data(pBuf, iSize, 0, 1))return -1;
	iEPOGNSSHour = *((int *)pBuf);
	iEPOGNSSHour &= 0x00FFFFFF;
	iCurrentGNSSHour = utc_to_gnss_hour(time);
	iSeg = (iCurrentGNSSHour - iEPOGNSSHour) / 6;
	//printf("aaaaaa:%d,%d,%d,%d,%d\r\n", getYearOfTime(time),getMonthOfTime(time),getDayOfTime(time),getHourOfTime(time),getMinuteOfTime(time));
	//printf("bbbbbb:%d,%d\r\n", iCurrentGNSSHour,iEPOGNSSHour);
	iMaxSegs = getEPOValidSegs();

	if(iMaxSegs > MTKEPO_SEGMENT_NUM)iMaxSegs = MTKEPO_SEGMENT_NUM;
	if ((iSeg < 0) || (iSeg > iMaxSegs))return -1;
	return iSeg;
}

BOOL epo_time_is_valid(structTimeStamp time)
{
    char epobuf[MTKEPO_RECORD_SIZE];
	//printf("systemtime+:%d\r\n", time);
    return epo_get_segment(time, epobuf, sizeof(epobuf)) >= 0;
}


void epo_send_utc_data(void)
{
	char epobuf[MTKEPO_RECORD_SIZE];
	structTimeStamp time = getCurrentHardwareTime();
	int iSeg = 0;

	iSeg = epo_get_segment(time, epobuf, sizeof(epobuf));
	printf("epo_send_utc_data:%d\r\n", iSeg);
	if(iSeg < 0)return;
	gps_set_dt_uts_by_stamp(time);
	static_iStartRecord = iSeg * (MTKEPO_SV_NUMBER);
	static_iEndRecord = static_iStartRecord + MTKEPO_SV_NUMBER;
	epo_read_data(static_epoBuffer, sizeof(static_epoBuffer), static_iStartRecord, MTKEPO_SV_NUMBER);
}

void epo_send_sat_data(void)
{
	int *epobuf;
	int iIndex = static_iStartRecord + MTKEPO_SV_NUMBER - static_iEndRecord + 1;

	printf("epo_send_sat_data:%d\r\n", static_iStartRecord);
	if (static_iStartRecord == static_iEndRecord){
		static_iStartRecord = 0;
		static_iEndRecord = 0;	
		return;
	}
	epobuf = (int *)static_epoBuffer;
	epobuf += iIndex * MTKEPO_RECORD_SIZE / sizeof(int);
	gps_set_sat_data(epobuf, iIndex);
	static_iStartRecord++;
}

