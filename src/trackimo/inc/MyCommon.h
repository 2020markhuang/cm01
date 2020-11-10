#ifndef AI_COMMON_FHQ_20190722_H
#define AI_COMMON_FHQ_20190722_H
#include "MyDefine.h"
#include "MyGPS.h"
#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
#define FATFS_BLOCK_SIZE 4096

typedef enum {
	DEVICE_STATUS_INIT,
	DEVICE_STATUS_POWER_ON,
	DEVICE_STATUS_POWER_ON_FINISH,
	DEVICE_STATUS_GPS_WORK,
	DEVICE_STATUS_WIFI_WORK,
	DEVICE_STATUS_4G_WORK,
	DEVICE_STATUS_SLEEP,
	DEVICE_STATUS_MAX
} enumDeviceStatusEvent;

char * getDeviceImei(void);
char * getDeviceImsi(void);
int getBatteryCapacity( );
void setBatteryCapacity(int capacity);
BOOL is_charging(void);
void set_ischarging(BOOL isCharging);
int GetBatteryLevel();
void setBatteryLevel(int Batterylevel);
BOOL isSystemSleep(void);
void setSystemSleep(BOOL value);
void setIsSIMCardInsert(BOOL value);
int getATResult(void);
void setATResult(int value);
int getGPSStatus(void);
void setGPSStatus(int value);
BOOL isWifiOn(void);
void setWifiOn(BOOL value);
BOOL isLwipOn(void);
void setLwipOn(BOOL value);
BOOL isScan(void);
BOOL is4GNetOK(void);
void setIs4GNetOK(BOOL value);
int getLedWorktime(void);
void setLedWorktime(int value);
int getLedWorkFlag(void);
void setLedWorkFlag(int value);
int getLedWorkreRepent(void);
void setLedWorktimeRepent(int value);
int getBuzzerWorktime(void);
void setBuzzerWorktime(int value);
int getBuzzerWorkRepent(void);
void setBuzzerWorkRepent(int value);
BOOL getViburatorWorkmode(void);
void setViburatorWorkmode(int value);
U8 Hex2Int(U8 c);
U8 hexCharToChar(U8 c1, U8 c2);
char * myReadLine(char * pOut, char * pIn);
char *rtrim(char *str);
char *ltrim(char *str);
char *trim(char *str);
char * deleteQuotation(char * pData);
char * deleteTitle(char * pData, const char * pTitle);
char * getATValidContent(char * pData, const char * pTitle);
int getATRetCode(char * pData);
int geConnectState(char * pData);
void myMemDump(const U8 * pBuffer, int iLen);
#if defined(__cplusplus)
}
#endif 
#endif //AI_COMMON_FHQ_20190722_H

