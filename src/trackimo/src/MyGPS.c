#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "MyCommon.h"
#include "WatchTask.h"
#include "MainTask.h"
#include "MyTimer.h"
#include "MyGPS.h"
#include "TrackerMain.h"
#include "GpsDecode.h"
#include "My4G.h"
#include "epo.h"
#include "WatchDateTime.h"
#include "MyClock.h"
BOOL gps_on[GPS_ON_FROM_MAX];
U8 sat_number_q[GPS_LAST_NUMBER_SEC];
U8 sat_max_snr_q[GPS_LAST_NUMBER_SEC];
#define GNSS_NEMA_SENTENCE_BUF_LEN 256
extern U16 GN_GPS_2D_fix;
extern U16 GN_GPS_3D_fix;
extern BOOL smart_gps;
extern BOOL tracking_high_speed_turnoff_gps;
typedef enum{
GPS_SENTENCE_TYPE_GPGGA, 
GPS_SENTENCE_TYPE_GPGSA,
GPS_SENTENCE_TYPE_GPRMC,
GPS_SENTENCE_TYPE_GPVTG,
GPS_SENTENCE_TYPE_GPGSV,
GPS_SENTENCE_TYPE_GLGSV,
GPS_SENTENCE_TYPE_GLGSA,
GPS_SENTENCE_TYPE_BDGSV,
GPS_SENTENCE_TYPE_BDGSA,
GPS_SENTENCE_TYPE_GPACCURACY,
GPS_SENTENCE_TYPE_PMTK,
GPS_SENTENCE_TYPE_MAX
}enumGPSSentenceType;


const static char * static_gpsSentenceType[]={
	"$GPGGA",
	"$GPGSA",
	"$GPRMC",
	"$GPVTG",
	"$GPGSV",
	"$GLGSV",
	"$GLGSA",
	"$BDGSV",
	"$BDGSA",
	"$GPACCURACY",
	"$PMTK",
};
static trk_comm_gps_repoter_info static_gpsData;
static U8 static_arraySatellites[MAX_GPS_SATELLITES];
uint8_t gps_sleep_handle = 0xFF;
static BOOL gps_write_cmd_to_uart(const char * pString){
	if(!pString)return FALSE;
	gps_printf("gps_write_cmd_to_uart:%s", pString);
	
	#ifndef WIN32	
	gnss_send_command((I8 *)pString, strlen(pString));
	#else
	if(static_funSendWin32)static_funSendWin32(pString, strlen(pString));
	#endif
	return TRUE;
}
static BOOL gps_write_pmtk_cmd(int cmd, char* strFormat, ...){
	char buffer[GNSS_NEMA_SENTENCE_BUF_LEN];
	char * p = buffer;
	char * p2 = buffer + 1;
	U8 cs = 0;
	va_list args;
	if(strFormat){
		sprintf(buffer, "$PMTK%03d,", cmd);
		p += strlen(p);

		va_start (args, strFormat);
		vsprintf (p, strFormat, args);
		va_end (args);
	}else{
		sprintf(buffer, "$PMTK%03d", cmd);
		p += strlen(p);
	}

	while(*p2)cs ^= *p2++;
	p += strlen(p);
	sprintf(p, "*%02X\r\n", cs);
	if(!gps_write_cmd_to_uart(buffer))return FALSE;
	return TRUE;
}
void gps_set_search_mode(BOOL bGPS, BOOL bGlonass, BOOL bCalileo, BOOL bCalileoFull, BOOL bBeido) 	
{
	gps_write_pmtk_cmd(PMTK_API_SET_GNSS_SEARCH_MODE, "%d,%d,%d,%d,%d", bGPS, bGlonass, bCalileo, bCalileoFull, bBeido);
}
void gps_set_report_condition(BOOL bEnable) 	
{
	gps_write_pmtk_cmd(PMTK_SET_NMEA_REPORT_CONDITION, "%d", bEnable);
}
void gps_set_report_interval(int interval, BOOL bPowerSaving) 	
{
	gps_write_pmtk_cmd(PMTK_SET_NMEA_REPORT_INTERVAL, "%d,%d", interval, bPowerSaving);
}
void gps_set_glp_mode(BOOL bEnable) 	
{
	gps_write_pmtk_cmd(PMTK_SET_GLP_MODE, "%d", bEnable ? 3 : 0);
}
void gps_set_periodic_mode(int type, int runtime, int sleeptime, int sec_runtime, int sec_sleeptime) 	
{
	gps_write_pmtk_cmd(PMTK_SET_PERIODIC_MODE, "%d,%d,%d,%d,%d", type, runtime, sleeptime, sec_runtime, sec_sleeptime);
}
void gps_set_dt_pos(int lon, int lat, int alt, int year, int month, int day, int hour, int minute, int second) 	
{
	gps_write_pmtk_cmd(PMTK_SET_NMEA_REPORT_CONDITION, "%d.%06d,%d.%06d,%d,%d,%d,%d,%d,%d,%d",lon / 1000000, lon % 1000000, lat / 1000000, lat % 1000000,alt, year, month, day, hour, minute, second);
}
void gps_set_dt_uts(int year, int month, int day, int hour, int minute, int second) 	
{
	gps_write_pmtk_cmd(PMTK_DT_UTC, "%d,%d,%d,%d,%d,%d", year, month, day, hour, minute, second);
}
void gps_set_static_nav_thd(double speed) 	
{
	if(speed < 0.1 || speed > 2.0)return;
	gps_write_pmtk_cmd(PMTK_API_SET_STATIC_NAV, "%.1f", speed);
}
void gps_host_start(void) 	
{
	gps_write_pmtk_cmd(PMTK_CMD_HOT_START, NULL);
}
void gps_cold_start(void) 	
{
	gps_write_pmtk_cmd(PMTK_CMD_COLD_START, NULL);
}
void gps_warm_start(void) 	
{
	gps_write_pmtk_cmd(PMTK_CMD_WARM_START, NULL);
}
void gps_entry_glonass_mode(void) 	
{
	gps_set_search_mode(0, 1, 0, 0, 0);
}
void gps_speed_lock(void) 	
{
	gps_set_static_nav_thd(1);
}
void gps_entry_periodic_mode(void) 	
{
	gps_set_periodic_mode(1, 5000,25000,15000,75000);
}
void gps_wake_up(void) 	
{
	gps_write_pmtk_cmd(PMTK_WAKE_UP, NULL);
}

#ifdef SUPPORT_MY_GPS
static void gnss_at_command_driver_callback_func(gnss_notification_type_t type, void *param)
{
	//	gps_printf("xxx:%d\r\n", type);
	switch (type) {
		case GNSS_NOTIFICATION_TYPE_READ:
		watch_task_post_message(WATCH_MSG_GPS_INT, 0, 0);
		break;
	case GNSS_NOTIFICATION_TYPE_HOST_WAKEUP:
		gps_wake_up();
		break;
	case GNSS_NOTIFICATION_TYPE_POWER_ON_CNF:
		setGPSStatus(GPS_STATUS_ON_SEARCHING);
		gps_write_pmtk_cmd(PMTK_SET_PERIODIC_MODE, "0,0,0");
		break;
	case GNSS_NOTIFICATION_TYPE_POWER_OFF_CNF:
		setGPSStatus(GPS_STATUS_OFF_NOTHING);
		break;
	case GNSS_NOTIFICATION_TYPE_WRITE:
		break;
		default:
		return;
	}
}
void gps_driver_init(void){
	gps_printf("gps_driver_init\r\n");
	gps_sleep_handle = hal_sleep_manager_set_sleep_handle("gpsTaskSleepLock");
	//hal_sleep_manager_lock_sleep(gps_sleep_handle);
        gnss_init(gnss_at_command_driver_callback_func);
}
void gps_entry_sleep(void){
	// hal_sleep_manager_unlock_sleep(gps_sleep_handle);
}
void gps_exit_sleep(void){
	//hal_sleep_manager_lock_sleep(gps_sleep_handle);
}
static void onGPSPMTK(char* pBuf, int iBufSize){
	int iCmd = 0;
	char temp[10];
	int iValue = 0;
	if ((!pBuf) || (iBufSize <= 0))return;
	
	memset(temp, 0, sizeof(temp));
	strncpy(temp, pBuf, 8);
	iCmd = atoi(temp+5);
	pBuf += 9;
	printf("onGPSPMTK:%d\r\n", iCmd);
	switch(iCmd){
		case PMTK_TXT_MSG:
			if(strncmp(pBuf, "MTKGPS", 6) == 0){
			}
			break;
		case PMTK_SYS_MSG:
			if(strncmp(pBuf, "002", 3) == 0){
			epo_send_utc_data();
			}			
			break;
		case PMTK_ACK:
			strncpy(temp, pBuf, 3);
			temp[3] = 0;
			iValue = atoi(temp);
			switch(iValue){
			case PMTK_DT_UTC:
			case PMTK_DT_POS:
			case PMTK_SAT_DATA:
			epo_send_sat_data();
			break;			
			}
			break;
	}
}
static int parseGPSTime(char* pTime) {
	char temp[10];
	int time = 0;

	if (!pTime)return 0;
	memset(temp, 0, sizeof(temp));
	strncpy(temp, pTime, 2);
	time = atoi(temp);
	gps_printf("parseGPSTime111:%d\r\n", time);
	time *= 60;
	strncpy(temp, pTime + 2, 2);
	time += atoi(temp);
	gps_printf("parseGPSTime222:%d\r\n", time);
	time *= 60;
	strncpy(temp, pTime + 4, 2);
	time += atoi(temp);
	gps_printf("parseGPSTime333:%d\r\n", time);
	time *= 1000;
	strcpy(temp, pTime + 7);
	time += atoi(temp);
	gps_printf("parseGPSTime444:%d\r\n", time);
	return time;
}

void nmea_parse_time(char* pTime, structUTC * pUTC) {
	char temp[10];

	if ((!pTime) || (!*pTime) || (!pUTC))return;

	memset(temp, 0, sizeof(temp));

	strncpy(temp, pTime, 2);
	pUTC->hour = atoi(temp);

	strncpy(temp, pTime + 2, 2);
	pUTC->minute = atoi(temp);

	strncpy(temp, pTime + 4, 2);
	pUTC->second = atoi(temp);
}

void nmea_parse_date(char* pDate, structUTC * pUTC) {
	char temp[10];
	
	if ((!pDate) || (!*pDate) || (!pUTC))return;
	
	memset(temp, 0, sizeof(temp));
	
	strncpy(temp, pDate, 2);
	pUTC->day = atoi(temp);

	strncpy(temp, pDate + 2, 2);
	pUTC->month = atoi(temp);
	
	strncpy(temp, pDate + 4, 2);
	pUTC->year = atoi(temp);
}

static int parseGPSLongtitude(char* pValue) {
	char temp[10];
	double value = 0;
//	char *p = NULL;
	
	if (!pValue)return 0;
	memset(temp, 0, sizeof(temp));
	strncpy(temp, pValue, 3);
	temp[3] = 0;
	value = atoi(temp);
/*	
	strncpy(temp, pValue + 2, 2);
	temp[2] = 0;
	value += atoi(temp) / 60.0;
	
	strcpy(temp, pValue + 6);

	value += atoi(temp) / 60.0;
*/	
	value += atof(pValue + 3) / 60.0;
	value *= 1000000;
	return (int)value;
}

static int parseGPSLatitude(char* pValue) {
	char temp[10];
	double value = 0;
//	char *p = NULL;
	
	if (!pValue)return 0;
	memset(temp, 0, sizeof(temp));
	strncpy(temp, pValue, 2);
	temp[2] = 0;
	value = atoi(temp);
	gps_printf("parseGPSLatitude111:%d\r\n", value);
/*	
	strncpy(temp, pValue + 2, 2);
	temp[2] = 0;
	value += atoi(temp) / 60.0;
	
	strcpy(temp, pValue + 5);

	value += atoi(temp) / 3600.0;
*/	
	value += atof(pValue + 2) / 60.0;
	gps_printf("parseGPSLatitude222:%d\r\n", value);
	value *= 1000000;
	return (int)value;
	gps_printf("parseGPSLatitude333:%d\r\n", value);
}



static void onGPSGGA(char * pBuf, int iBufSize){
	char * p1 = pBuf + 1;
	char * p2;
	int iIndex = 0;
	int iLen = 0;
	char temp[64];

	trk_comm_gps_repoter_info *pGPS = &static_gpsData;
	if ((!pBuf) || (iBufSize <= 0))return;
	memset(pGPS, 0, sizeof(trk_comm_gps_repoter_info));
	do{
		p2 = strchr(p1, ',');
		if (!p2) {
			p2 = strchr(p1, '*');
			if(!p2)break;
		}
		iLen = p2 - p1;
		strncpy(temp, p1, iLen);
		temp[iLen] = 0;
		p1 = p2 + 1;
		switch(iIndex){
			case 0:
				//pGPS->time = parseGPSTime(temp);
				break;
			case 1:
				pGPS->latitude = parseGPSLatitude(temp);
				break;
			case 2:
				if(temp[0] == 'S')pGPS->latitude = -pGPS->latitude;
				break;
			case 3:
				pGPS->longitude= parseGPSLongtitude(temp);
				break;
			case 4:
				if(temp[0] == 'W')pGPS->longitude = -pGPS->longitude;
				break;
			case 5:
				break;
			case 6:
				break;
			case 7:
				break;
			case 8:
				pGPS->altitude = (int)(atof(temp) * 10);
				break;
			case 9:
				break;
			case 10:
				break;
			case 11:
				break;
		}
		iIndex++;
	}while(1);
	gps_printf("onGPSGGA:%d,%d,%d\r\n", pGPS->longitude,pGPS->latitude,pGPS->altitude);

	main_task_post_message(MAIN_MSG_GPS_EVENT, GPS_EVENT_NEW_DATA, (int)pGPS);
}
static void onGPSGSA(char * pBuf, int iBufSize){
	char * p1 = pBuf + 1;
	char * p2;
	int iIndex = 0;
	int iLen = 0;
	char temp[64];
	
	trk_comm_gps_repoter_info *pGPS = &static_gpsData;
	if ((!pBuf) || (iBufSize <= 0))return;
	memset(static_arraySatellites, 0, sizeof(static_arraySatellites));
	do{
		p2 = strchr(p1, ',');
		if (!p2) {
			p2 = strchr(p1, '*');
			if(!p2)break;
		}
		iLen = p2 - p1;
		strncpy(temp, p1, iLen);
		temp[iLen] = 0;
		p1 = p2 + 1;
		switch(iIndex){
			case 0:
				break;
			case 1:
				{
					int value = atoi(temp);
					switch(value){
						case GPS_LOCATED_2D:
							setGPSStatus(GPS_STATUS_ON_2D_FIX);
							main_task_post_message(MAIN_MSG_GPS_EVENT, GPS_EVENT_LOCATED, 0);
							break;
						case GPS_LOCATED_3D:
							setGPSStatus(GPS_STATUS_ON_3D_FIX);
							main_task_post_message(MAIN_MSG_GPS_EVENT, GPS_EVENT_LOCATED, 0);
							break;
					}
				}
				break;
			case 14:
			case 15:
			case 16:
				break;
			default:
				static_arraySatellites[iIndex - 2] = atoi(temp);
				break;
		}
		iIndex++;
	}while(1);
}
static void onGPSRMC(char * pBuf, int iBufSize){
	char * p1 = pBuf + 1;
	char * p2;
	int iIndex = 0;
	int iLen = 0;
	char temp[64];

	trk_comm_gps_repoter_info *pGPS = &static_gpsData;
	if ((!pBuf) || (iBufSize <= 0))return;
	memset(pGPS, 0, sizeof(trk_comm_gps_repoter_info));
	do{
		p2 = strchr(p1, ',');
		if (!p2) {
			p2 = strchr(p1, '*');
			if(!p2)break;
		}
		iLen = p2 - p1;
		strncpy(temp, p1, iLen);
		temp[iLen] = 0;
		p1 = p2 + 1;
		switch(iIndex){
			case 0:
				nmea_parse_time(temp, (structUTC *)&pGPS->time);
				//pGPS->time = parseGPSTime(temp);
				break;
			case 1:
				if(temp[0] == 'V')return;
				break;
			case 2:
				pGPS->latitude = parseGPSLatitude(temp);
				break;
			case 3:
				if(temp[0] == 'S')pGPS->latitude = -pGPS->latitude;
				break;
			case 4:
				pGPS->longitude= parseGPSLongtitude(temp);
				break;
			case 5:
				if(temp[0] == 'W')pGPS->longitude = -pGPS->longitude;
				break;
			case 6:
				break;
			case 7:
				break;
			case 8:
				break;
			case 9:
				break;
			case 10:
				break;
			case 11:
				break;
		}
		iIndex++;
	}while(1);
	gps_printf("onGPSRMC:%d,%d,%d\r\n", pGPS->longitude,pGPS->latitude,pGPS->altitude);
	main_task_post_message(MAIN_MSG_GPS_EVENT, GPS_EVENT_NEW_DATA, (int)pGPS);
}
static void onGPSVTG(char * pBuf, int iBufSize){
	char * p1 = pBuf + 1;
	char * p2;
	int iIndex = 0;
	int iLen = 0;
	char temp[64];
	
	trk_comm_gps_repoter_info *pGPS = &static_gpsData;
	if ((!pBuf) || (iBufSize <= 0))return;
	do{
		p2 = strchr(p1, ',');
		if (!p2) {
			p2 = strchr(p1, '*');
			if(!p2)break;
		}
		iLen = p2 - p1;
		strncpy(temp, p1, iLen);
		temp[iLen] = 0;
		p1 = p2 + 1;
		switch(iIndex){
			case 0:
				break;
			case 6:
				//pGPS->speed= atof(temp)*10;
				break;
		}
		iIndex++;
	}while(1);
}
static void onGPSGLGSV(char * pBuf, int iBufSize){
	char * p1 = pBuf + 1;
	char * p2;
	int iIndex = 0;
	int iLen = 0;
	char temp[64];
	
	trk_comm_gps_repoter_info *pGPS = &static_gpsData;
	if ((!pBuf) || (iBufSize <= 0))return;
	do{
		p2 = strchr(p1, ',');
		if (!p2) {
			p2 = strchr(p1, '*');
			if(!p2)break;
		}
		iLen = p2 - p1;
		strncpy(temp, p1, iLen);
		temp[iLen] = 0;
		p1 = p2 + 1;
		switch(iIndex){
			case 2:
				//pGPS->satellites = atoi(temp);
				break;
			case 6:
				//pGPS->bestSNR = atoi(temp);
				break;
		}
		iIndex++;
	}while(1);
}
static BOOL checkGPSSentenceValid(char * pBuf, int iBufSize){
	char * p = pBuf;
	U8 cs = 0;
	char temp[10];
	
	if((!pBuf) || (iBufSize <= 0))return FALSE;
	if(*p != '$')return FALSE;	
	p = pBuf + 1;
	while(((*p) != '*') && (*p))cs ^= *p++;
	
	sprintf(temp, "*%02X\r\n", cs);
	if(strcmp(temp, p))return FALSE;
	
	return TRUE;
}
static void gps_sentence_parse(char * pBuf, int iBufSize){
	int i;
	int iType = -1;
	char **pp = (char **)static_gpsSentenceType;
	char* pData = NULL;
	int iDataLen = 0;

	if((!pBuf) || (iBufSize <= 0))return;
	for(i = 0; i < GPS_SENTENCE_TYPE_MAX; i++, pp++){

		if(strncmp(pBuf, *pp, strlen(*pp)) == 0){
			iType = i;
			pData = pBuf;// + strlen(*pp);
			iDataLen = strlen(pData);
			break;			
		}
	}
	//gps_printf("gps_sentence_parsedata:%s\r\n",pData);
	//gps_printf("gps_sentence_parseType:%d\r\n", iType);
	switch(iType){
		case GPS_SENTENCE_TYPE_GPGGA:
			//onGPSGGA(pData, iDataLen);
			NMEA_GPGGA_Analysis(pData,iDataLen);
			break;
		case GPS_SENTENCE_TYPE_GPGSA:	// gps satellites used
			NMEA_GPGSA_Analysis(pData,iDataLen);
			//onGPSGSA(pData, iDataLen);
			break;
		case GPS_SENTENCE_TYPE_GPRMC:
			//onGPSRMC(pData, iDataLen);
			NMEA_GPRMC_Analysis(pData,iDataLen);
			break;
		case GPS_SENTENCE_TYPE_GPVTG:	//speed
			//onGPSVTG(pData, iDataLen);
			NMEA_GPVTG_Analysis(pData,iDataLen);
			break;
		case GPS_SENTENCE_TYPE_GPGSV:	// gps satellites can see
			NMEA_GPGSV_Analysis(pData, iDataLen);
			break;
		case GPS_SENTENCE_TYPE_GLGSV:	// glonass satellites can see
			break;
		case GPS_SENTENCE_TYPE_GLGSA:	// glonass satellites used
			break;
		case GPS_SENTENCE_TYPE_BDGSV:	// beidou satellites can see
			break;
		case GPS_SENTENCE_TYPE_BDGSA:	// beidou satellites used
			break;
		case GPS_SENTENCE_TYPE_GPACCURACY:
			break;
		case GPS_SENTENCE_TYPE_PMTK:
			onGPSPMTK(pData, iDataLen);
			break;
	}
}
int gps_read_data(char * pBuf, int iBufSize){
	int iLen = 0;

	if((!pBuf) || (iBufSize <= 0))return 0;
	memset(pBuf, 0, iBufSize);
	#ifndef WIN32	
	iLen = gnss_read_sentence((I8 *)pBuf, iBufSize);
	#endif
	//gps_printf("gps_read_data:%d,%s\r\n", iLen, pBuf);
	if(iLen <= 0)return 0;
	if(!checkGPSSentenceValid(pBuf, iBufSize))return 0;

	gps_sentence_parse(pBuf, iBufSize);
	return iLen;
}
BOOL gps_receive_data(char * pData, int iDataLen){
	if ((!pData) || (iDataLen <= 0))return 0;
	gps_printf("gps_receive_data:%s\r\n", pData);
	//if (!checkGPSSentenceValid(pData, iDataLen))return FALSE;
	gps_sentence_parse(pData, iDataLen);
	return TRUE;
}

void gps_set_sat_data(int * pData, int iIndex) 	
{
	if(!pData)return;
	gps_write_pmtk_cmd(PMTK_SAT_DATA, "%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X", \
		iIndex, pData[0], pData[1], pData[2],  pData[3],  pData[4],  pData[5],  pData[6],  pData[7],  pData[8], \
		pData[9],  pData[10],  pData[11],  pData[12],  pData[13],  pData[14], pData[15], pData[16], pData[17]);
}

void gps_set_dt_uts_by_stamp(structTimeStamp time) 	
{
	int year, month, day;
	int hour,minute,second;
	
	getDateOfYear(time, &year, &month, &day);
	hour = getHourOfTime(time);
	minute = getMinuteOfTime(time);
	second = getSecondOfTime(time);
	gps_set_dt_uts(year, month, day, hour, minute, second);
}

void TESTMODE_COMMAND_TURN_OFF_GPS(void)
{
	gps_printf("TESTMODE_COMMAND_TURN_OFF_GPS");
	gps_on[GPS_ON_FROM_COMMAND_TESTMODE] = FALSE;
	CheckIfTurnOffGPS();
}	

void TESTMODE_COMMAND_TURN_ON_GPS(void)
{
	 gps_printf("TESTMODE_COMMAND_TURN_ON_GPS");
	 gps_on[GPS_ON_FROM_COMMAND_TESTMODE] = TRUE;
	 GPS_POWERON();
}
#endif
void GPS_POWERON(void){
	gps_printf("gps_onLOCKED=%d,%d\r\n",hal_sleep_manager_is_sleep_locked(),hal_sleep_manager_get_lock_status() & (1 << gps_sleep_handle));
	//gps_sleep_handle = hal_sleep_manager_set_sleep_handle("gpsTaskSleepLock");
	hal_sleep_manager_lock_sleep(gps_sleep_handle);
	gnss_power_on();
}
void GPS_POWEROFF(void){
	gps_printf("gps_offLOCKED=%d,%d\r\n",hal_sleep_manager_is_sleep_locked(),hal_sleep_manager_get_lock_status() & (1 << gps_sleep_handle));
	if(hal_sleep_manager_get_lock_status() & (1 << gps_sleep_handle))	
	{
		hal_sleep_manager_unlock_sleep(gps_sleep_handle);
		gps_printf("gps_offLOCKED=%d,%d\r\n",hal_sleep_manager_is_sleep_locked(),hal_sleep_manager_get_lock_status() & (1 << gps_sleep_handle));
	}
	GN_GPS_2D_fix = 0;
	GN_GPS_3D_fix = 0; 
	gnss_power_off();	
}

//如果还有任何一个要求开着，就不能关
void CheckIfTurnOffGPS(void)
{
	U8 i=0;
	BOOL turnoff = FALSE;
	for(i=0;i<GPS_ON_FROM_MAX;i++)
	{
		if(gps_on[i] == TRUE)
		{
			gps_printf( "CheckIfTurnOffGPS: True\r\n",i);	
		  	return;
		}  
	}
	gps_printf("CheckIfTurnOffGPS: off\r\n");	
	GPS_POWEROFF();
}

BOOL gps_2d_3d_fix_with_hdop_and_fix_sec(U16 fix_sec)
{
	gps_printf("gps_2d_3d_fix:2dfix=%d,3dfix=%d\r\n",GN_GPS_2D_fix,GN_GPS_3D_fix);
	if((g_trk_var.g_trk_content_info.gps_info.gps_state== GPS_STATUS_ON_2D_FIX)||(g_trk_var.g_trk_content_info.gps_info.gps_state== GPS_STATUS_ON_3D_FIX))
	{
		if((GN_GPS_2D_fix+GN_GPS_3D_fix)>fix_sec)//等30s，防止速度漂移，将来要删掉
		   return TRUE;
	}
	return FALSE;
}

void GET_GPS_COMMAND_TURN_ON_GPS(void)
{
	gps_printf("GET_GPS_COMMAND_TURN_ON_GPS\r\n");
	gps_on[GPS_ON_FROM_COMMAND_GET_GPS] = TRUE;
	GPS_POWERON();
}
void GET_GPS_COMMAND_TURN_OFF_GPS(void)
{
	gps_printf("GET_GPS_COMMAND_TURN_OFF_GPS\r\n");
	gps_on[GPS_ON_FROM_COMMAND_GET_GPS] = FALSE;
	CheckIfTurnOffGPS();
}
void XMODE_COMMAND_TURN_OFF_GPS(void)
{
	 gps_printf("XMODE_COMMAND_TURN_OFF_GPS\r\n");
	 gps_on[GPS_ON_FROM_COMMAND_XMODE] = FALSE;
	 CheckIfTurnOffGPS();
}	

void XMODE_COMMAND_TURN_ON_GPS(void)
{
	 gps_printf("XMODE_COMMAND_TURN_ON_GPS\r\n");
	 gps_on[GPS_ON_FROM_COMMAND_XMODE] = TRUE;
	 GPS_POWERON();
}

void FENCE_OR_SPEED_COMMAND_TURN_ON_GPS(void)
{
	 gps_printf("FENCE_OR_SPEED_COMMAND_TURN_ON_GPS");
	 gps_on[GPS_ON_FROM_COMMAND_FENCE_OR_SPEED] = TRUE;
	 GPS_POWERON();
}

void FENCE_OR_SPEED_COMMAND_TURN_OFF_GPS(void)
{
	 gps_printf("FENCE_OR_SPEED_COMMAND_TURN_OFF_GPS");
	 gps_on[GPS_ON_FROM_COMMAND_FENCE_OR_SPEED] = FALSE;
	 CheckIfTurnOffGPS();
}

BOOL Is_fence_speed_gps_on(void)
{
	if(gps_on[GPS_ON_FROM_COMMAND_FENCE_OR_SPEED] == TRUE)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

int get_gps_onoff_total_sec(void)
{
	 return 1;
}	 	

BOOL Is_gps_searching(void)
{
	if(g_trk_var.g_trk_content_info.gps_info.gps_state == GPS_STATUS_ON_SEARCHING)
	{
		return TRUE;
	}
	else
	{
		return FALSE;		
	}
}

BOOL Is_xmode_gps_on(void)
{
	if(gps_on[GPS_ON_FROM_COMMAND_XMODE] == TRUE)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL Is_get_gps_on(void)
{
	if(gps_on[GPS_ON_FROM_COMMAND_GET_GPS] == TRUE)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


