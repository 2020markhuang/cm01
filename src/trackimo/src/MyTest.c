#include <string.h>
#include <stdio.h>

#include "MyTimer.h"
#include "Module4GTask.h"
#include "My4G.h"
#include "TrackerCfg.h"
#include "MyGPS.h"
#include "MyCommon.h"
#include "MyTest.h"

static void trackimoFrameSendTest(void){
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS, 0);
	tracker_protocol_send_deal(REPLY_HEADER_STATUS_REPORT_1, 0);
	tracker_protocol_send_deal(REPLY_HEADER_GET_GPS, 0);
	tracker_protocol_send_deal(REPLY_HEADER_BUTTON, 0);
	tracker_protocol_send_deal(REPLY_HEADER_POWERUP, 0);
	tracker_protocol_send_deal(REPLY_HEADER_REPORT_IMSI, 0);
	tracker_protocol_send_deal(REPLY_HEADER_SESSION_START, 0);
	tracker_protocol_send_deal(REPLY_GET_ACK, 0);
	tracker_protocol_send_deal(REPLY_HEADER_ERROR, 0);
}
static void gpsSendTest(void){
	gps_set_dt_pos(113123456, 22654321, 123, 2019,9,8,9,41,20);

	gps_host_start();
	gps_cold_start();
	gps_warm_start();
	gps_entry_glonass_mode();
	gps_speed_lock();
	gps_entry_periodic_mode();
	gps_entry_sleep();
	gps_exit_sleep();
	gps_set_static_nav_thd(1.5);
	gps_set_dt_uts(2019,9,8,9,41,20);
	gps_set_glp_mode(1);
	gps_set_report_interval(20, 1);
	gps_set_report_condition(1);

}
static void gpsReceiveTest(void){
	int i, iCount;
	
	const char * pReceives[]={
		"$GPRMC,080655.00,A,4546.40891,N,12639.65641,E,1.045,328.42,080912,,,A*60\r\n",
		//"$PMTK011,MTKGPS*08\r\n",
		//"$GPGSA,A,1,,,,,,,,,,,,,,,*1E\r\n",
		"$GPGSV,2,1,08,14,68,099,36,16,59,243,37,31,44,030,40,32,42,129,32*7F\r\n",
		//"$GPGSV,2,2,08,22,30,273,42,03,24,295,43,29,19,054,40,193,,,15*77\r\n",
		"$GPGSA,A,3,22,03,29,14,16,31,32,,,,,,1.50,1.19,0.91*0E\r\n",
		//"$PMTK010,0A,132705.000,2230.9251,N,11355.4824,E,1,7,1.19,79.5,M,-2.6,M,,*77\r\n",
		//"$PMTK264,1*2F\r\n",
		"$GPGGA,014434.70,3817.13334637,N,12139.72994196,E,1,07,1.5,6.571,M,8.942,M,0.7,0016*79\r\n",
	};
	

	char **pp = (char **)pReceives;
	iCount = sizeof(pReceives) / sizeof(pReceives[0]);
	for(i = 0; i < iCount; i++, pp++){
		gps_receive_data(*pp, strlen(*pp) + 1);
	}	
}

#if 0
static void module4GReceiveTest(void){
	int i, iCount;
	
	const char * pReceives[]={
		"28\r\n",
		"\r\n+SYSSTART\r\n",
		"\r\nSEQUANS Communications\r\nOK\r\n",
		"+WS46:31\r\n",
		">00D0A\r\n",
		"+CGSN:\"123455432101234\"\r\n",
		">010D0A\r\n",
		">1C0D0A\r\n",
		">130D0A\r\n",
		">510D0A\r\n",
		">250D0A\r\n",
		"+CEREG:5\r\n",
		"+SHUTDOWN\r\n",

	};
	

	char **pp = (char **)pReceives;
	iCount = sizeof(pReceives) / sizeof(pReceives[0]);
	iCount = 2;
	for(i = 0; i < iCount; i++, pp++){
		module_4g_receive_data(*pp, strlen(*pp) + 1);
	}
}
#endif

static void module4GSendTest(void){
	int i;
	/**
	for(i = 0; i <= AT_4G_CMD_SHUTDOWN; i++){
		module4g_task_send_at_cmd(i, NULL);
	}
	module4g_task_send_at_cmd(AT_4G_CMD_SHUTDOWN, NULL);**/
	//module4g_task_send_at_cmd(AT_4G_CMD_CGSN, "%s","?");
}

#if 0//ndef WIN32
static void myFatFSTest(void){
	int size = 0;
	FRESULT res;
	char buf[10];	
	FIL * hFile = getFatFSFileHandle();
	
	res = f_open(hFile, _T("0:/aa.txt"), FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	res = f_write(hFile, "Hello", 6, &size);
	res = f_close(hFile);
	
	res = f_open(hFile, _T("0:/aa.txt"), FA_OPEN_EXISTING | FA_READ);
	memset(buf, 0, sizeof(buf));
	res = f_read(hFile, buf, 6, &size);
	printf("%s\r\n", buf);
	res = f_close(hFile);
}
#endif
static void myLineTest(void){
	char temp[100];
	char * p = "\r\nSEQUANS Communications\r\nOK\r\n";

	do{
	p = myReadLine(temp, p);
	printf("111:%s", temp);
	printf("222:%s", p);
	}while(*p);
}
void myTest(void){

	//myLineTest();
	//myTimerTest();
	#ifndef WIN32
	//gpsReceiveTest();
	//myFatFSTest();
	#endif
}
