#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "MyDefine.h"
#include "AudioResource.h"
#include "Module4GTask.h"
#include "My4G.h"
#include "MyCommon.h"
static BOOL firstPowerOn= TRUE;
static int static_iLedWorktime = 0;
static int static_iLedWorkFlag =0;
static int static_iLedWorkRepent = 0;
static int static_iBuzzerWorktime= 0;
static int static_iBuzzerWorkRepent= 0;
static BOOL static_iViburatorWorkmode = FALSE;
static BOOL static_bSleepMode = FALSE;
static BOOL static_bWifiOn = FALSE;
static BOOL static_bLwipOn = FALSE;
static BOOL static_bScan = FALSE;
static char static_imei[16]={0};
static char static_imsi[20]={0};
static BOOL static_i4GNetStatus = FALSE;
static int static_isCharging=0;
static int static_iATResult = 0;
static int static_iGPSStatus = 0;
static int static_BatteryCapacity= 0;
static int static_Batterylevel= 0;
static BOOL module_sleep_enable_flag = FALSE;
int getBatteryCapacity( )
{
	return static_BatteryCapacity;
}
void setBatteryCapacity(int capacity)
{
	static_BatteryCapacity = capacity;
}

int is_charging(void)
{
	return static_isCharging;
}
void set_ischarging(int isCharging)
{
	static_isCharging =  isCharging;
}

int GetBatteryLevel()
{
	return static_Batterylevel;
}
void setBatteryLevel(int Batterylevel)
{
	static_Batterylevel = Batterylevel;
}

char * getDeviceImei(void){
	return static_imei;
}
char * getDeviceImsi(void){
	return static_imsi;
}

BOOL isSystemSleep(void){
	return static_bSleepMode;
}
void setSystemSleep(BOOL value){
	static_bSleepMode = value;
}

BOOL is4GSleepMode(void){
	return module_sleep_enable_flag;
}
void set4GSleepMode(BOOL value){
	module_sleep_enable_flag = value;
}

BOOL isWifiOn(void){
	return static_bWifiOn;
}
void setWifiOn(BOOL value){
	static_bWifiOn = value;
}

BOOL isLwipOn(void){
	return static_bLwipOn;
}
void setLwipOn(BOOL value){
	static_bLwipOn = value;
}

BOOL isScan(void){
	return static_bScan;
}
void setScan(BOOL value){
	static_bScan = value;
}

int getGPSStatus(void){
	return static_iGPSStatus;
}
void setGPSStatus(int value){
	static_iGPSStatus = value;
}

BOOL is4GNetOK(void){
	return static_i4GNetStatus;
}
void setIs4GNetOK(BOOL value)
{
	static_i4GNetStatus = value;
}

int getATResult(void){
	return static_iATResult;
}

void setATResult(int value){
	static_iATResult = value;
}

int getLedWorktime(void){
	return static_iLedWorktime;
}
void setLedWorktime(int value){
	static_iLedWorktime = value;
}

int getLedWorkreRepent(void){
	return static_iLedWorkRepent;
}
void setLedWorktimeRepent(int value){
	static_iLedWorkRepent = value;
}

int getLedWorkFlag(void){
	return static_iLedWorkFlag;
}
void setLedWorkFlag(int value){
	static_iLedWorkFlag = value;
}

BOOL isFirstPowerOn(void){
	return firstPowerOn;
}
void setFirstPowerOn(BOOL value){
	firstPowerOn = value;
}

int getBuzzerWorktime(void){
	return static_iBuzzerWorktime;
}
void setBuzzerWorktime(int value){
	static_iBuzzerWorktime = value;	
}

int getBuzzerWorkRepent(void){
	return static_iBuzzerWorkRepent;
}

void setBuzzerWorkRepent(int value){
	static_iBuzzerWorkRepent = value;	
}

BOOL getViburatorWorkmode(void){
	return static_iViburatorWorkmode;
}
void setViburatorWorkmode(BOOL value){
	static_iViburatorWorkmode = value;
}

U8 Hex2Int(U8 c){
	if(c >= '0' && c <= '9'){
		c -= '0';
	}else if(c >= 'a' && c <= 'f'){
		c -= 'a' - 10;
	}else if(c >= 'A' && c <= 'F'){
		c -= 'A' - 10;
	}else{
		c = 0;
	}
	return c;
}
U8 hexCharToChar(U8 c1, U8 c2){
	return (U8)(Hex2Int(c1) << 4) | Hex2Int(c2);
}

char * myReadLine(char * pOut, char * pIn){
	char * pDest = pOut;
	
	if(!pOut || !pIn)return NULL;
	while((*pIn) && (*pIn != '\n')){
		*pDest++ = *pIn++;
	}
	while(((pDest - 1) >= pOut) && (*(pDest - 1) == '\r'))pDest--;
	*pDest = 0;
	return pIn + 1;
}
char *rtrim(char *str){
	int len;
	char * p;
	
	if (str == NULL || *str == '\0'){
	return str;	
	} 
	len = strlen(str);
	p = str + len - 1;
	while ((p >= str)  && isspace((U8)(*p)))	{		
		*p = '\0';	
		--p;
	} 
	return str;
} 
char *ltrim(char *str){
	int len;
	char * p;
	
	if (str == NULL || *str == '\0')	{
		return str;
	} 
	len = 0;
	p = str;
	while ((*p) && isspace((U8)(*p))){	
		++p;	
		++len;
	} 
	memmove(str, p, strlen(str) - len + 1); 
	return str;
} 
char *trim(char *str){
	str = rtrim(str);
	str = ltrim(str);
	return str;
}
char * deleteQuotation(char * pData){
	int iCount = 0;
	char * pStart = NULL;
	char * pEnd = NULL;
	
	if(!pData)return NULL;
	pStart = pData;
	while(*pStart == '\"')pStart++;
	
	iCount = strlen(pStart);
	if(iCount <= 0)return pStart;
	pEnd = pStart + iCount - 1;
	while((pEnd >= pStart) && (*pEnd == '\"')){
		pEnd--;
	}
	*(pEnd + 1) = 0;
	return pStart;	
}
char * deleteTitle(char * pData, const char * pTitle){
	int iLen;
	
	if(!pData || !pTitle)return pData;
	iLen = strlen(pTitle);
	if(memcmp(pData, pTitle, iLen) != 0)iLen = 0;
	return pData + iLen;
}
char * getATValidContent(char * pData, const char * pTitle){
	pData = deleteTitle(pData, pTitle);
	if (*pData == ':')pData++;
	pData = trim(pData);
//	pData = deleteQuotation(pData);
	return pData;
}
int getATRetCode(char * pData){
	char * p;
	char temp[15];
	
	if(!pData)return -1;
	p = strchr(pData, ',');
	if(!p){
		return atoi(pData);
		}
	else{
		int iLen = p - pData;
		strncpy(temp, pData, iLen);
		temp[iLen] = 0;
		return atoi(temp);
	}
}

int geConnectState(char * pData){
	char * p;
	char temp[15];
	
	if(!pData)return -1;
	p = strchr(pData, ',');
	if(!p){
		return atoi(p+1);
		}
	else{
		return atoi(p+1);
	}
}

void myMemDump(const U8 * pBuffer, int iLen){
	char string[128];
	U8 * pSrc = (U8 *)pBuffer;
	U8 * pDest = (U8 *)string;
	int i;
	U8 c;

	if(!pBuffer)return;
	for(i = 0; i < iLen; i++, pSrc++){
		c = (*pSrc) >> 4;
		if(c <= 9){
			*pDest = '0' + c;
		}else{
			*pDest = 'A' + c - 10;
		}

		pDest++;
		c = (*pSrc) & 0x0F;
		if( c <= 9){
			*pDest = '0' + c;
		}else{
			*pDest = 'A' + c - 10;
		}
		pDest++;

		*pDest++ = ' ';
	}
	*pDest++ = '\r';
	*pDest++ = '\n';
	*pDest = 0;
	//printf("%s", (char *)string);
}

