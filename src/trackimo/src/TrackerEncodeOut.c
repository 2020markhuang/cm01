#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "MyCommon.h"
#include "TrackerCfg.h"
#include "My4G.h"
#include "TrackimoFrame.h"
#include "TrackerQueue.h"
#include "MyMemory.h"
#include "TrackerMain.h"
#include "MygSensor.h"
#include "MyFile.h"
#include "MyGPS.h"
#include "MyTimer.h"
extern BOOL GET_HISTORY_PROCESSING;
extern trackergps_backup_data_struct gps_backup[GPS_BACKUP_MAX_NUMBER];//±£´æ500¸ö×ø±ê,¹²40*500 = 20000byte,20k byte
extern trackergps_backup_data_struct gps_backup_fixed_history_buffer[GPS_BACKUP_MAX_NUMBER];//¸ù¾ÝÊ±¼äµÄfix×ø±ê£¬×î¶àÒ»Ñù500¸ö£¬20k byte
extern BOOL wifi_read_from[];

U8 compression_buffer[100];
//ussd Ò»Ìõ×î¶à²»ÄÜ³¬¹ý183
U16 multi_history_total_records = 0;
U16 multi_history_current_spot = 0;
char multi_cell_info_1[200];//Ç°3¸ö
char multi_cell_info_2[200];//ºó3¸
#define RECORDS_HISTORY_PER_USSD_MESSAGE 6 //history ussd Ò»Ìõ¿ÉÒÔ·¢6¸ö,choice sim µÄussdÒ»´Î×î¶à140-150bytes
#define RECORDS_HISTORY_PER_DATA_MESSAGE 100 //history data Ò»Ìõ¿ÉÒÔ·¢100¸ö
void tracker_reply_server_msg_success(reply_channel data_channel) 
{
	printf("tracker_reply_server_msg_success\r\n");
	tracker_struct_msg *result;
	U8 reply[100]={0};
	int length = 0;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));

	//#ifdef __SIM_DEFAULT_CHANNEL_DATA__
	//   if(g_trk_var.g_user_settings.check_ack_timer_enable==KAL_TRUE)
	//	 	 return;
	//#endif
	//   if(data_channel == 0xff)
	//         data_channel = g_trk_var.g_user_settings.data_transfer_channel;

	reply[length]  = REPLY_HEADER_SUCCESS;length++;
	//dataÒª¼Ódevice id
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	reply[length] = g_trk_var.g_trk_content_info.command;length++;
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);
}

void tracker_send_gps_info_to_server(int data_channel)
{
	char reply[100]={0};
	U8 length=0;
	//trk_comm_gps_repoter_info * pGPSInfo = getGPSInfo();
	memset(reply, 0, sizeof(reply));
	printf("tracker_send_gps_info_to_server12121212121,%d,%d,%d\r\n",g_trk_var.g_trk_content_info.gps_info.latitude,g_trk_var.g_trk_content_info.gps_info.longitude,g_trk_var.g_trk_content_info.gps_info.speed_km);
	do_cordinates_compression((double)g_trk_var.g_trk_content_info.gps_info.latitude / 1000000,(double)g_trk_var.g_trk_content_info.gps_info.longitude / 1000000,(double)g_trk_var.g_trk_content_info.gps_info.speed_km/1000,COMMAND_GET_GPS_COMPRESSION_TYPE);
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));
	reply[length] = REPLY_HEADER_GET_GPS;length++;
	//ÏÖÔÚcompression_bufferÖÐÓÐ11¸ö×Ö½Ú£¬5¸ölat£¬5¸ölong£¬1¸öspeed£¬bcdÑ¹Ëõ
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0]; length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1]; length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2]; length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3]; length++;
	//lat
	reply[length] = compression_buffer[0];length++;
	reply[length] = compression_buffer[1];length++;
	reply[length] = compression_buffer[2];length++;
	reply[length] = compression_buffer[3];length++;
	reply[length] = compression_buffer[4];length++;

	//long
	reply[length] = compression_buffer[5];length++;  
	reply[length] = compression_buffer[6];length++;
	reply[length] = compression_buffer[7];length++;
	reply[length] = compression_buffer[8];length++;
	reply[length] = compression_buffer[9];length++;

	//speed
	reply[length] = compression_buffer[10];length++;

	//¿´µ½¶àÉÙ¿ÅÎÀÐÇ      
	reply[length] = g_trk_var.g_trk_content_info.gps_info.satellite_using_num;length++;

	//×îÇ¿ÎÀÐÇsnr
	reply[length] = 50;length++;

	//Ê²Ã´fix
	if(getGPSStatus()==GPS_STATUS_ON_3D_FIX)
	{  
	reply[length] = 3;
	length++;
	}      
	else
	{  
	reply[length] = 2;
	length++; 
	}      

	//bat
	reply[length] = getBatteryCapacity();length++;
	reply[length] = 0;length++;
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);	
}

/*********************************************************************************
0 - USSD/ 1 - SMS/ 2 ¨C Data (1byte)
UDP ip (4 bytes)
UDP port (2 bytes)
APN (variable length string)
Device ID (4 bytes)
IMEI (16 digits - BCD)(8 bytes)
Tracking configuration info (see below)
--------------------------------------
-	Tracking Channel: 0 - USSD/1 - SMS/2 ¨C Data (1 byte)
-	Tracking UDP ip (4 bytes)
-	Tracking UDP port (2 bytes)
-	Sample Time ¨C the rate at which the GPS device samples location and speed (mins ¨C 1byte) (ex: every 1 min).
-	Report Time  ¨C the rate at which the GPS device reports location and speed to the server (mins ¨C 1byte)  (ex: every 10 min).
-	Location accuracy ¨C the accuracy of the reported location measurement (0 - Low/1 - High/2 - Debug)
-	Transmit Device ID (0 ¨C off/1 ¨C on) (1 byte)
-	Delta Compression (0 ¨C off/1 ¨C on) (1 byte)

--------------------------------------
TestMode off/on (1 byte)
Active alarm ids (concatenated strings)
FW Version

*********************************************************************************/
//choice ÐÂsim,ussd²»ÄÜ³¬¹ý140!
void tracker_server_msg_analyze_status_1(int channel) 
{
	printf("tracker_server_msg_analyze_status_1\r\n");
	U8 reply[100]={0};
	int length = 0;
	int i = 0;
	U16 number = 0;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));
	reply[length] = REPLY_HEADER_STATUS_REPORT_1;length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	reply[length] = g_trk_var.g_user_settings.data_transfer_channel;length++;
	//UDP ip (4 bytes)
	reply[length] = g_trk_var.g_user_settings.platform_url.host.ip[0];length++;
	reply[length] = g_trk_var.g_user_settings.platform_url.host.ip[1];length++;
	reply[length] = g_trk_var.g_user_settings.platform_url.host.ip[2];length++;
	reply[length] = g_trk_var.g_user_settings.platform_url.host.ip[3];length++;		
	//UDP port (2 bytes)
	reply[length] = g_trk_var.g_user_settings.platform_url.port>>8;length++;   //¸ß 8 Î»
	reply[length] = g_trk_var.g_user_settings.platform_url.port&0xFF;length++; //µÍ  8 Î»
	//APN (variable length string),¿É±ä×Ö·û´®³¤Î²²¿¼Ó0
	strcpy((char *)(reply+length), "trackimo.lpwa.kddi.com");length = length + strlen("trackimo.lpwa.kddi.com");
	strcpy((char*)(reply+length),g_trk_var.g_user_settings.platform_url.apn);length = length + strlen(g_trk_var.g_user_settings.platform_url.apn);
	reply[length] = 0;length++;
	//Device ID (4 bytes)
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	//IMEI (16 digits - BCD)(8 bytes)
	reply[length] = (g_trk_var.g_trk_device_status.imei_4G[0]-48)<<4 | (g_trk_var.g_trk_device_status.imei_4G[1]-48);length++;
	reply[length] = (g_trk_var.g_trk_device_status.imei_4G[2]-48)<<4 | (g_trk_var.g_trk_device_status.imei_4G[3]-48);length++;
	reply[length] = (g_trk_var.g_trk_device_status.imei_4G[4]-48)<<4 | (g_trk_var.g_trk_device_status.imei_4G[5]-48);length++;
	reply[length] = (g_trk_var.g_trk_device_status.imei_4G[6]-48)<<4 | (g_trk_var.g_trk_device_status.imei_4G[7]-48);length++;
	reply[length] = (g_trk_var.g_trk_device_status.imei_4G[8]-48)<<4 | (g_trk_var.g_trk_device_status.imei_4G[9]-48);length++;
	reply[length] = (g_trk_var.g_trk_device_status.imei_4G[10]-48)<<4 | (g_trk_var.g_trk_device_status.imei_4G[11]-48);length++;
	reply[length] = (g_trk_var.g_trk_device_status.imei_4G[12]-48)<<4 | (g_trk_var.g_trk_device_status.imei_4G[13]-48);length++;
	reply[length] = (((g_trk_var.g_trk_device_status.imei_4G[14]-48)<<4) & 0xf0);length++;
	if(g_trk_var.g_trk_settings.sample_time> 0)//ÐÂtracking
	{
	reply[length] = g_trk_var.g_trk_settings.track_channel;length++;
	}
	else
	{			
	reply[length] =  g_trk_var.g_trk_settings.Xmode_channel;length++;
	} 
	//UDP ip (4 bytes)
	reply[length] = g_trk_var.g_user_settings.platform_url.host.ip[0];length++;
	reply[length] = g_trk_var.g_user_settings.platform_url.host.ip[1];length++;
	reply[length] = g_trk_var.g_user_settings.platform_url.host.ip[2];length++;
	reply[length] = g_trk_var.g_user_settings.platform_url.host.ip[3];length++;
	//UDP port (2 bytes)
	//reply[length] = g_trk_var.g_user_settings.platform_url.port>>8;length++;   //¸ß 8 Î»
	//reply[length] = g_trk_var.g_user_settings.platform_url.port&0xFF;length++; //µÍ  8 Î»
	if(g_trk_var.g_trk_settings.sample_time> 0)//ÐÂtracking
	{
	//-   Sample Time ¨C the rate at which the GPS device samples location and speed (mins ¨C 1byte) (ex: every 1 min).
	reply[length] = g_trk_var.g_trk_settings.sample_time;  length++;
	//- Report Time  ¨C the rate at which the GPS device reports location and speed to the server (mins ¨C 1byte)  (ex: every 10 min).
	reply[length] = g_trk_var.g_trk_settings.report_time;  length++;   
	//- Location accuracy ¨C the accuracy of the reported location measurement (0 - Low/1 - High/2 - Debug)
	reply[length] = g_trk_var.g_trk_settings.location_accuracy;  length++;   
	//- Transmit Device ID (0 ¨C off/1 ¨C on) (1 byte)
	reply[length] = bit_of_byte(g_trk_var.g_trk_settings.track_setting,7);;  length++;                      
	//- Delta Compression (0 ¨C off/1 ¨C on) (1 byte)
	reply[length] = bit_of_byte(g_trk_var.g_trk_settings.track_setting,6);   length++;   
	}
	else
	{   
	//- Sample Time ¨C the rate at which the GPS device samples location and speed (mins ¨C 1byte) (ex: every 1 min).
	reply[length] =  g_trk_var.g_trk_settings.Xmode_smaple_time;   length++;
	//- Report Time  ¨C the rate at which the GPS device reports location and speed to the server (mins ¨C 1byte)  (ex: every 10 min).
	reply[length] = g_trk_var.g_trk_settings.Xmode_report_time;   length++;   
	//- Location accuracy ¨C the accuracy of the reported location measurement (0 - Low/1 - High/2 - Debug)
	reply[length] = g_trk_var.g_trk_settings.Xmode_location_accuracy;length++;   
	//- Transmit Device ID (0 ¨C off/1 ¨C on) (1 byte)
	reply[length] =  g_trk_var.g_trk_settings.Xmode_transmit_deviceid;length++;   
	//- Delta Compression (0 ¨C off/1 ¨C on) (1 byte)
	reply[length] =  g_trk_var.g_trk_settings.Xmode_delta_compression; length++;   
	}    
	//TestMode off/on (1 byte)
	reply[length] = 0;length++;
	//Number of active alarms (1 byte) 
	//Active alarm ids (concatenated strings)   
	number = 0;
	//tracker_prompt_trace("total alarm id number:%d",number);
	//°ÑNumber of active alarms (1 byte) Ð´½øÈ¥ 
	reply[length] = number;
	length = length+number+1;//×¢ÒâÒª¼ÓÕâ¸ö1
	printf("server_msg_analyze_status_1 final length=%d\n",length);
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	//tracker_protocol_send(result,data_channel);
	module_message_buffer_push(result);
	myFree(result);
	//module_4g_send_trackimo_data(reply, length);
}  

void tracker_alm_msg_send(reply_channel data_channel)
{
	printf("tracker_alm_msg_send\r\n");
	U8 reply[100]={0};
	int length=0;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));
	char mcc[10];
	char mnc[10];
	char lac[10];
	char cellid[10];
	memset(mcc,0,10);
	memset(mnc,0,10);
	memset(lac,0,10);
	memset(cellid,0,10);
	sprintf(mcc,"%d",400);
	sprintf(mnc,"%d", 6);
	sprintf(lac,"%d",7);
	sprintf(cellid,"%d",9);
	reply[length] = REPLY_HEADER_ALARM_TRIGGERED_WITH_GSM;length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	reply[length] = g_trk_var.g_trk_status.alarm_type;length++;
	reply[length] = 0;length++;//speed  

	reply[length] = 0;length++;//duration

	strcpy((reply+length),cellid);length = length + strlen(cellid);
	reply[length] = 0;length++;

	strcpy((reply+length),lac);length = length + strlen(lac);
	reply[length] = 0;length++;

	strcpy((reply+length),mcc);length = length + strlen(mcc);
	reply[length] = 0;length++;

	strcpy((reply+length),mnc);length = length + strlen(mnc);
	reply[length] = 0;length++;
	//bat
	reply[length] = getBatteryCapacity();length++;
	//para
	reply[length] = 0;length++;
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);
}

/**************************************
Number of imsis (1 byte)
Imsis (concatenated strings)
Index of active Imsi  (1 byte)
***************************************/
void tracker_server_msg_analyze_get_imsi(reply_channel data_channel)
{
 	printf("tracker_server_msg_analyze_get_imsi\r\n");
	U8 reply[100]={0};
	int length=0;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));
     // if(data_channel == 0xff)
     //       data_channel = g_trk_var.g_user_settings.data_transfer_channel;
	reply[length] = REPLY_HEADER_REPORT_IMSI;length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	//Number of imsis (1 byte)
	reply[length] = 1;length++;
	//Imsis (concatenated strings)
	strcpy((reply+length),g_trk_var.g_trk_device_status.imsi_4G);length = length + strlen(g_trk_var.g_trk_device_status.imsi_4G);
	reply[length] = 0;length++;
	//Index of active Imsi  (1 byte)
	reply[length] = 1;length++;
	result->msgLen =length;
    	memcpy(result->body_msg,reply,length);
    	module_message_buffer_push(result);
    	myFree(result);
   	 if(check_ack_setting_bit(ACK_BIT_Get_IMSI) == TRUE)
      		{
			g_trk_var.g_trk_content_info.command=COMMAND_GET_IMSI;
			tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,data_channel);
      		}
}

carrier_name_struct tracker_carrie;
//Èç¹û¶ªÁËgsmÔÙ×¢²áÉÏ£¬¸æÖªserver
void tracker_sent_carrier(reply_channel data_channel)
{
    	char temp[50]={0};
       U8 reply[100]={0};
	int length = 0;
       int i;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));
    //  if(data_channel == 0xff)
    //        data_channel = g_trk_var.g_user_settings.data_transfer_channel;
	memset(&tracker_carrie,0,sizeof(carrier_name_struct));
	strcpy(temp,"not available");
	strcpy(&(tracker_carrie.carrier_name_buffer[0]),(char*)temp);
	reply[length] = REPLY_HEADER_REPORT_CARRIERS;length++;
	reply[length] =  g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] =  g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] =  g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] =  g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	for(i=0;i<tracker_carrie.len;i++)
	 {
	     reply[length]=tracker_carrie.carrier_name_buffer[i];length++;
	 }
	result->msgLen =length;
    	memcpy(result->body_msg,reply,length);
    	module_message_buffer_push(result);
    	myFree(result);
}

void tracker_sent_carrier_error(reply_channel data_channel)
{
       U8 reply[100]={0};
	int length = 0;
       char temp[10]={0};
       int i;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));
     // if(data_channel == 0xff)
    //        data_channel = g_trk_var.g_user_settings.data_transfer_channel;
	reply[length] = REPLY_HEADER_REPORT_CARRIERS;length++;
	reply[length] =  g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] =  g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] =  g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] =  g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	reply[length] = 1;length++;
	strcpy(temp,"error-");
	for(i=0;i<strlen(temp);i++)
	{
	  reply[length]=(U8)temp[i];length++;
	}

	for(i=0;i<tracker_carrie.len;i++)
	{
	  reply[length]=tracker_carrie.carrier_name_buffer[i];length++;
	}
	result->msgLen =length;
    	memcpy(result->body_msg,reply,length);
    	module_message_buffer_push(result);
    	myFree(result);
}

void tracker_send_first_tower_info(reply_channel data_channel)
{
	char reply[100]={0};
	U8 length = 0;
	char mcc[10];
	char mnc[10];
	char lac[10];
	char cellid[10];
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));
	memset(mcc,0,10);
	memset(mnc,0,10);
	memset(lac,0,10);
	memset(cellid,0,10);

   // if(data_channel == 0xff)
   // data_channel = g_trk_var.g_user_settings.data_transfer_channel;
         reply[length] = REPLY_GSM_LOCATION_REPORT;length++;
         reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
         reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
         reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
         reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;

	  sprintf(mcc,"%d",400);
	  sprintf(mnc,"%d", 6);
	  sprintf(lac,"%d",7);
	  sprintf(cellid,"%d",9);
        //sprintf(mcc,"%d",g_trk_var.g_trk_content_info.cell_info[0].mcc);
        //sprintf(mnc,"%d", g_trk_var.g_trk_content_info.cell_info[0].mnc);
        //sprintf(lac,"%d",g_trk_var.g_trk_content_info.cell_info[0].lac);
        //sprintf(cellid,"%d", g_trk_var.g_trk_content_info.cell_info[0].cellid);
        
        strcpy((reply+length),cellid);length = length + strlen(cellid);
        reply[length] = 0;length++;
        
        strcpy((reply+length),lac);length = length + strlen(lac);
        reply[length] = 0;length++;
        
        strcpy((reply+length),mcc);length = length + strlen(mcc);
        reply[length] = 0;length++;
        
        strcpy((reply+length),mnc);length = length + strlen(mnc);
        reply[length] = 0;length++;
        
        //bat
        reply[length] = getBatteryCapacity();length++;
	 result->msgLen =length;
    	 memcpy(result->body_msg,reply,length);
    	 module_message_buffer_push(result);
    	 myFree(result);
}

/*********************************************
Byte Count (3 bytes)
History Size (3 bytes)
Battery Percent Status (1 byte)
CellID (variable length string)
LAC (variable length string)
MCC (variable length string)
MNC (variable length string)
Number Of Satelites (1 byte)
Signal Strength (1 byte)
Is Fix (1 byte)
Minutes since last fix (3 bytes) 
Response Delay Time (secs) (1byte)
***********************************************/
//Ö±½Ó»Ø¸´notify
void tracker_server_msg_analyze_status_2(reply_channel data_channel)
{
	printf("tracker_server_msg_analyze_status_2\r\n");
	char reply[100]={0};
	int length=0;
	int number = 0;
	int i ;
	char mcc[10];
	char mnc[10];
	char lac[10];
	char cellid[10];
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));
  	memset(mcc,0,10);
 	memset(mnc,0,10);
 	memset(lac,0,10);
 	memset(cellid,0,10);
        if(data_channel == 0xff)
        data_channel = g_trk_var.g_user_settings.data_transfer_channel;
	reply[length] = REPLY_HEADER_STATUS_REPORT_2;length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	//dataÐè¼Ódevice id

	//Byte Count (3 bytes)  byte count is how many bytes you used in the current session.ÔÝ²»ÊµÏÖ
	reply[length] = 0;length++; 
	reply[length] = 0;length++; 
	reply[length] = 0;length++; 

	//History Size (3 bytes)  
	number = 400;//get_number_of_records_in_backup_database();

	//×î¶à500£¬µÚÒ»¸ö¿Ï¶¨ÊÇ0
	reply[length] = 0;length++; 
	reply[length] = number/256;length++; 
	reply[length] = number- (number/256)*256;length++; 

	//Battery Percent Status (1 byte)
	reply[length] = getBatteryCapacity();length++; 

	//CellID (variable length string)
	//LAC (variable length string)
	//MCC (variable length string)
	//MNC (variable length string)
  	sprintf(mcc,"%d",400);
	sprintf(mnc,"%d", 6);
	sprintf(lac,"%d",7);
	sprintf(cellid,"%d",9);
	//sprintf(mcc,"%d",g_trk_var.g_trk_content_info.cell_info[0].mcc);
	//sprintf(mnc,"%d", g_trk_var.g_trk_content_info.cell_info[0].mnc);
	//sprintf(lac,"%d",g_trk_var.g_trk_content_info.cell_info[0].lac);
	//sprintf(cellid,"%d", g_trk_var.g_trk_content_info.cell_info[0].cellid);
	strcpy((reply+length),cellid);length = length + strlen(cellid);
	reply[length] = 0;length++;

	strcpy((reply+length),lac);length = length + strlen(lac);
	reply[length] = 0;length++;

	strcpy((reply+length),mcc);length = length + strlen(mcc);
	reply[length] = 0;length++;

	strcpy((reply+length),mnc);length = length + strlen(mnc);
	reply[length] = 0;length++;

	//Number Of Satelites (1 byte)
	reply[length] = 1;length++;//g_trk_var.g_trk_content_info.gps_info.satellite_visable_num;length++;

	//Signal Strength (1 byte)
	//reply[length] = srv_nw_info_get_signal_strength_in_percentage(MMI_SIM1);length++;
	reply[length] = 1;length++; //g_trk_var.g_trk_content_info.gps_info.max_snr;length++;//ÊÇgpsÐÅºÅ²»ÊÇgsmÐÅºÅ

	//Is Fix (1 byte)
	if(g_trk_var.g_trk_content_info.gps_info.gps_state==GPS_STATUS_ON_3D_FIX)
	{
	reply[length] = 3;length++;
	}
	else if(g_trk_var.g_trk_content_info.gps_info.gps_state==GPS_STATUS_ON_2D_FIX)
	{
	reply[length] = 2;length++;
	}
	else
	{
	reply[length] = 0;length++; 
	}
	//Minutes since last fix (3 bytes),ÔÝ²»ÊµÏÖ
	reply[length] = 0;length++; 
	reply[length] = 0;length++;
	reply[length] = 0;length++;

	//wait_dur
	reply[length] =g_trk_var.g_user_settings.ussd_lock_timer;length++; 

	//GPS on When Charging (1 byte)
	reply[length] =g_trk_var.g_user_settings.charge_powon_gps;length++;

	//Wake Up When Off (1 byte),Î´ÊµÏÖ
	reply[length] = 0;length++;

	//gsm keep alive, in hour
	reply[length] = g_trk_var.g_user_settings.ussd_heart_beat_in_hour;length++;

	//data keep alive, in min
	reply[length] = g_trk_var.g_user_settings.data_heart_beat_in_min;length++;

	//beep enable
	reply[length] =g_trk_var.g_user_settings.beeper_enable;length++;

	// tracker_prompt_trace("beep enable =%d",g_trk_var.g_user_settings.beeper_enable);

	//send session start
	reply[length] = g_trk_var.g_user_settings.session_enable;length++;

	//session max kbytes
	reply[length] =  g_trk_var.g_user_settings.current_session_max_bytes;length++;

	//reboot timer active,È¥×ö¶¨Ê±ÖØÆôÁË
	reply[length] =  g_trk_var.g_user_settings.reboot_timer;length++;

	//reboot time(hours)£¬Ã»ÊµÏÖ
	reply[length] = 0;length++;

	//reboot time off time£¬Ã»ÊµÏÖ
	reply[length] = 0;length++;


	//10sÄÚ¶àÉÙ¸öÖÐ¶ÏÈÏÎªÊÇÕð¶¯£¬¿ÉÒÔÖØÐÂ¿ªÊ¼tracking, alarm...
	reply[length] = g_trk_var.g_user_settings.tracking_alarm_in_10s;length++;

	//heart pulse, ¶à³¤Ê±¼ä·¢Ò»´ÎÐÄÌø°ü£¬µ¥Î»Ð¡Ê±
	reply[length] = g_trk_var.g_user_settings.tracking_heart_pulse;length++;

	//Èç¹ûÐÄÌø°üÃ»»Ø¸´£¬¹Ø»ú¼¸·ÖÖÓ¿ª»ú£¬µ¥Î»·ÖÖÓ
	reply[length] = g_trk_var.g_user_settings.no_heart_pulse_reboot_in_min;length++;

	//Èç¹ûÁ¬Ðøn¸öussd send fail£¬reboot
	reply[length] =g_trk_var.g_user_settings.send_ussd_fail_num;length++;

	//ussd q timer
	reply[length] = get_ussd_q_timer();length++;

	//moving alarm count
	reply[length] = get_motion_para1();length++;  

	//moving alarm filter       
	reply[length] = get_motion_para2();length++;    

	//moving alarm filter 2 16Î»µÄ¸ß8Î»     
	reply[length] = get_motion_para3();length++;
	result->msgLen =length;
    	memcpy(result->body_msg,reply,length);
    	module_message_buffer_push(result);
    	myFree(result);
}

void tracker_protocol_encode_powerup(int channel) 
{
	printf("tracker_protocol_encode_powerup\r\n");
	U8 reply[100]={0};
	int length = 0;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));
	reply[length] = REPLY_HEADER_POWERUP;length++;
	//ÏÖÔÚcompression_bufferÖÐÓÐ11¸ö×Ö½Ú£¬5¸ölat£¬5¸ölong£¬1¸öspeed£¬bcdÑ¹Ëõ
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0]; length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1]; length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2]; length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3]; length++;
	reply[length] = g_trk_var.g_trk_device_status.power_on_status;length++;
	
	//carrier       
	strcpy((char*)(reply+length),TRACKER_COMM_NETWORK);
	length = length + strlen(TRACKER_COMM_NETWORK);
	reply[length] = 0;length++;

	//version number
	strcpy((char*)(reply+length),TRACKER_COMM_VERSION);
	length = length + strlen(TRACKER_COMM_VERSION);

	strcpy((char*)(reply+length),TRACKER_COMM_BULDTIME);
	length = length + strlen(TRACKER_COMM_BULDTIME); 
	reply[length] = 0;length++;
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);
    //module_4g_send_trackimo_data(reply, length);
}

void multi_history_data_send(void)
{
	 char reply[100]={0};
	 int i;
	 U16 relative_min = 0;
	 U8 standard_time[6] = {0,0,0,0,0,0};
	 U8 current_time[6] = {0,0,0,0,0,0};
	 U16 length = 0;
	 U16 temp = 0;
	//Ç¡ÇÉÕûÊý·¢ÍêÁË£¬µ½´ËÎªÖ¹
	 if(multi_history_total_records <= multi_history_current_spot)
	 {
	 	 GET_HISTORY_PROCESSING = FALSE;	
	 	 return;	
	 }	
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));
	 if(multi_history_total_records > multi_history_current_spot+ RECORDS_HISTORY_PER_DATA_MESSAGE)
	 {
	     reply[length] = REPLY_HEADER_HISTORYREPORT;length++;
	    
	   	//dataÒª¼Ódevice id
            reply[length] = g_trk_var.g_user_settings.deviceid[0];length++;
            reply[length] = g_trk_var.g_user_settings.deviceid[1];length++;
            reply[length] = g_trk_var.g_user_settings.deviceid[2];length++;
            reply[length] = g_trk_var.g_user_settings.deviceid[3];length++;
	    
	    //number sent
	    reply[length] = RECORDS_HISTORY_PER_DATA_MESSAGE/256;length++;
	    reply[length] = RECORDS_HISTORY_PER_DATA_MESSAGE - reply[length-1]*256;length++;
	    
	    //ÓÐ¿ÉÄÜÒª2¸ö×Ö½Ú
	    temp = multi_history_total_records-1-multi_history_current_spot-RECORDS_HISTORY_PER_DATA_MESSAGE;
	    
	    //number left
	    reply[length] = temp/256;length++;
	    reply[length] = temp - reply[length-1]*256;length++;
	    
	    //»ù×¼Ê±¼ä£¬6bytes
	    reply[length] = gps_backup_fixed_history_buffer[multi_history_current_spot].year;length++;
	    reply[length] = gps_backup_fixed_history_buffer[multi_history_current_spot].month;length++;
	    reply[length] = gps_backup_fixed_history_buffer[multi_history_current_spot].day;length++;
	    reply[length] = gps_backup_fixed_history_buffer[multi_history_current_spot].hour;length++;
	    reply[length] = gps_backup_fixed_history_buffer[multi_history_current_spot].minute;length++;
	    reply[length] = gps_backup_fixed_history_buffer[multi_history_current_spot].second;length++;
	    
	    //standard time
	    standard_time[0] = gps_backup_fixed_history_buffer[multi_history_current_spot].year;
	    standard_time[1] = gps_backup_fixed_history_buffer[multi_history_current_spot].month;
	    standard_time[2] = gps_backup_fixed_history_buffer[multi_history_current_spot].day;
	    standard_time[3] = gps_backup_fixed_history_buffer[multi_history_current_spot].hour;
	    standard_time[4] = gps_backup_fixed_history_buffer[multi_history_current_spot].minute;
	    standard_time[5] = gps_backup_fixed_history_buffer[multi_history_current_spot].second;
	   
	     
	    for(i=multi_history_current_spot;i<(multi_history_current_spot+RECORDS_HISTORY_PER_DATA_MESSAGE);i++)
	    {
	    	//gps_backup_fixed_history_bufferÀïÃæ×îÀÏµÄÔÚ×îÇ°Ãæ
	      do_cordinates_compression(gps_backup_fixed_history_buffer[i].latitude,gps_backup_fixed_history_buffer[i].longitude,gps_backup_fixed_history_buffer[i].speed,COMPRESSION_2);//3bytesÑ¹Ëõ
	      
	      //lat
	      reply[length] = compression_buffer[0];length++;
	      reply[length] = compression_buffer[1];length++;
	      reply[length] = compression_buffer[2];length++;
	      
	      //long
	      reply[length] = compression_buffer[3];length++;
	      reply[length] = compression_buffer[4];length++;
	      reply[length] = compression_buffer[5];length++;
	      
	      //speed
	      reply[length] = compression_buffer[6];length++;
	      
	      //time relative in min
	      if(i==multi_history_current_spot)
	      {
	      	relative_min = 0;
	      }
	      else
	      {
   	        current_time[0] = gps_backup_fixed_history_buffer[i].year;
	        current_time[1] = gps_backup_fixed_history_buffer[i].month;
	        current_time[2] = gps_backup_fixed_history_buffer[i].day;
	        current_time[3] = gps_backup_fixed_history_buffer[i].hour;
	        current_time[4] = gps_backup_fixed_history_buffer[i].minute;
	        current_time[5] = gps_backup_fixed_history_buffer[i].second;
	        
	        relative_min = 10; //get_relative_min(current_time,standard_time);
	      }  
	        reply[length] = relative_min/256;length++;
	        reply[length] = relative_min - reply[length-1]*256;length++;
	    }
	    
	    //·¢
          result->msgLen =length;
          memcpy(result->body_msg,reply,length);
    	    module_message_buffer_push(result);
	  //Ìøcurrent
	  multi_history_current_spot = multi_history_current_spot+RECORDS_HISTORY_PER_DATA_MESSAGE;
	    
	    //15sºóÔÙ·¢£¬ÕâÑù²»»áÓ°ÏìQÄÇ±ß
	    //StopTimer(MY_TIMER_78);
           // StartTimer(MY_TIMER_78, 15*1000, multi_history_data_send);
	    
    }
	else //¾ÍÊ£×îºóÒ»µãÁË
	  {
	    reply[length] = REPLY_HEADER_HISTORYREPORT;length++;
	    
  	    //dataÒª¼Ódevice id
          reply[length] = g_trk_var.g_user_settings.deviceid[0];length++;
          reply[length] = g_trk_var.g_user_settings.deviceid[1];length++;
          reply[length] = g_trk_var.g_user_settings.deviceid[2];length++;
          reply[length] = g_trk_var.g_user_settings.deviceid[3];length++;
  	    
	    //number sent
	    reply[length] = 0;length++;
	    reply[length] = multi_history_total_records-multi_history_current_spot;length++;
	    
	    //number left
	    reply[length] = 0;length++;
	    reply[length] = 0;length++;
	    
	    //»ù×¼Ê±¼ä£¬6bytes
	    reply[length] = gps_backup_fixed_history_buffer[multi_history_current_spot].year;length++;
	    reply[length] = gps_backup_fixed_history_buffer[multi_history_current_spot].month;length++;
	    reply[length] = gps_backup_fixed_history_buffer[multi_history_current_spot].day;length++;
	    reply[length] = gps_backup_fixed_history_buffer[multi_history_current_spot].hour;length++;
	    reply[length] = gps_backup_fixed_history_buffer[multi_history_current_spot].minute;length++;
	    reply[length] = gps_backup_fixed_history_buffer[multi_history_current_spot].second;length++;
	    
	    //standard time
	    standard_time[0] = gps_backup_fixed_history_buffer[multi_history_current_spot].year;
	    standard_time[1] = gps_backup_fixed_history_buffer[multi_history_current_spot].month;
	    standard_time[2] = gps_backup_fixed_history_buffer[multi_history_current_spot].day;
	    standard_time[3] = gps_backup_fixed_history_buffer[multi_history_current_spot].hour;
	    standard_time[4] = gps_backup_fixed_history_buffer[multi_history_current_spot].minute;
	    standard_time[5] = gps_backup_fixed_history_buffer[multi_history_current_spot].second;
	   
	     
	    for(i=multi_history_current_spot;i<multi_history_total_records;i++)
	    {
	    	//gps_backup_fixed_history_bufferÀïÃæ×îÀÏµÄÔÚ×îÇ°Ãæ
	      do_cordinates_compression(gps_backup_fixed_history_buffer[i].latitude,gps_backup_fixed_history_buffer[i].longitude,gps_backup_fixed_history_buffer[i].speed,COMPRESSION_2);//3bytesÑ¹Ëõ
	      
	      //lat
	      reply[length] = compression_buffer[0];length++;
	      reply[length] = compression_buffer[1];length++;
	      reply[length] = compression_buffer[2];length++;
	      
	      //long
	      reply[length] = compression_buffer[3];length++;
	      reply[length] = compression_buffer[4];length++;
	      reply[length] = compression_buffer[5];length++;
	      
	      //speed
	      reply[length] = compression_buffer[6];length++;
	      
	      //time relative in min
	      if(i==multi_history_current_spot)
	      {
	      	relative_min = 0;
	      }
	      else
	      {
   	        current_time[0] = gps_backup_fixed_history_buffer[i].year;
	        current_time[1] = gps_backup_fixed_history_buffer[i].month;
	        current_time[2] = gps_backup_fixed_history_buffer[i].day;
	        current_time[3] = gps_backup_fixed_history_buffer[i].hour;
	        current_time[4] = gps_backup_fixed_history_buffer[i].minute;
	        current_time[5] = gps_backup_fixed_history_buffer[i].second;
	        relative_min = 10;//get_relative_min(current_time,standard_time);
	      }  
	      reply[length] = relative_min/256;length++;
	      reply[length] = relative_min - reply[length-1]*256;length++;
	    }
	     result->msgLen =length;
            memcpy(result->body_msg,reply,length);
    	     module_message_buffer_push(result);
	    //finish
	    GET_HISTORY_PROCESSING = FALSE;	
   }
 	myFree(result);
}	  		

void send_history_data_with_data(void)
{
	 char reply[100]={0};
	 U16 total_requested_number = 0;
	 int i;
	 U16 relative_min = 0;
	 U8 standard_time[6] = {0,0,0,0,0,0};
	 U8 current_time[6] = {0,0,0,0,0,0};
	 U16 length = 0;
	 tracker_struct_msg *result;
	 result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	 memset(result,0,sizeof(tracker_struct_msg));

	 total_requested_number = get_number_of_records_in_backup_requested_history_buffer();
	 if(total_requested_number == 0)
	 {
	     reply[length] = REPLY_HEADER_HISTORYREPORT;length++;
	 	 	
	 	 	//dataÒª¼Ódevice id
            reply[length] = g_trk_var.g_user_settings.deviceid[0];length++;
            reply[length] = g_trk_var.g_user_settings.deviceid[1];length++;
            reply[length] = g_trk_var.g_user_settings.deviceid[2];length++;
            reply[length] = g_trk_var.g_user_settings.deviceid[3];length++;
	    
	    //number sent
	    reply[length] = 0;length++;
	    reply[length] = 0;length++;
	    
	    //number left
	    reply[length] = 0;length++;
	    reply[length] = 0;length++;

	    result->msgLen =length;
    	    memcpy(result->body_msg,reply,length);
    	    module_message_buffer_push(result);
          GET_HISTORY_PROCESSING = FALSE;
	 }   
	 else if(total_requested_number <= RECORDS_HISTORY_PER_DATA_MESSAGE)//Ò»Ìõ×î¶à100¸öµã
	 {
	  	reply[length] = REPLY_HEADER_HISTORYREPORT;length++;
	    
		//dataÒª¼Ódevice id
		reply[length] = g_trk_var.g_user_settings.deviceid[0];length++;
		reply[length] = g_trk_var.g_user_settings.deviceid[1];length++;
		reply[length] = g_trk_var.g_user_settings.deviceid[2];length++;
		reply[length] = g_trk_var.g_user_settings.deviceid[3];length++;

		//number sent
		reply[length] = total_requested_number/256;length++;
		reply[length] = total_requested_number - reply[length-1]*256;length++;

		//number left
		reply[length] = 0;length++;
		reply[length] = 0;length++;

		//»ù×¼Ê±¼ä£¬6bytes
		reply[length] = gps_backup_fixed_history_buffer[0].year;length++;
		reply[length] = gps_backup_fixed_history_buffer[0].month;length++;
		reply[length] = gps_backup_fixed_history_buffer[0].day;length++;
		reply[length] = gps_backup_fixed_history_buffer[0].hour;length++;
		reply[length] = gps_backup_fixed_history_buffer[0].minute;length++;
		reply[length] = gps_backup_fixed_history_buffer[0].second;length++;

		//standard time
		standard_time[0] = gps_backup_fixed_history_buffer[0].year;
		standard_time[1] = gps_backup_fixed_history_buffer[0].month;
		standard_time[2] = gps_backup_fixed_history_buffer[0].day;
		standard_time[3] = gps_backup_fixed_history_buffer[0].hour;
		standard_time[4] = gps_backup_fixed_history_buffer[0].minute;
		standard_time[5] = gps_backup_fixed_history_buffer[0].second;
	  	
		for(i=0;i<total_requested_number;i++)
		{
			//gps_backup_fixed_history_bufferÀïÃæ×îÀÏµÄÔÚ×îÇ°Ãæ
		  do_cordinates_compression(gps_backup_fixed_history_buffer[i].latitude,gps_backup_fixed_history_buffer[i].longitude,gps_backup_fixed_history_buffer[i].speed,COMPRESSION_2);//3bytesÑ¹Ëõ
		  
		  //lat
		  reply[length] = compression_buffer[0];length++;
		  reply[length] = compression_buffer[1];length++;
		  reply[length] = compression_buffer[2];length++;
		  
		  //long
		  reply[length] = compression_buffer[3];length++;
		  reply[length] = compression_buffer[4];length++;
		  reply[length] = compression_buffer[5];length++;
		  
		  //speed
		  reply[length] = compression_buffer[6];length++;
		  
		  //time relative in min
		  if(i==0)
		  {
		  	relative_min = 0;
		  }
		  else
		  {
		    current_time[0] = gps_backup_fixed_history_buffer[i].year;
		    current_time[1] = gps_backup_fixed_history_buffer[i].month;
		    current_time[2] = gps_backup_fixed_history_buffer[i].day;
		    current_time[3] = gps_backup_fixed_history_buffer[i].hour;
		    current_time[4] = gps_backup_fixed_history_buffer[i].minute;
		    current_time[5] = gps_backup_fixed_history_buffer[i].second;
		    
		    relative_min = 0;//get_relative_min(current_time,standard_time);
		  }  

		//2×Ö½Ú£¬relative min
		  reply[length] = relative_min/256;length++;
		  reply[length] = relative_min - reply[length-1]*256;length++;
		}
             result->msgLen =length;
             memcpy(result->body_msg,reply,length);
    	      module_message_buffer_push(result);
            GET_HISTORY_PROCESSING = FALSE;
	  }
	  else //¶àµã£¬³¬¹ý1Ìõ·¢ËÍ
	  {
	  	  multi_history_current_spot = 0;
	  	  multi_history_total_records = total_requested_number;
	  	  multi_history_data_send();
	  }	
        myFree(result);
}	  


void tracker_send_history_data(reply_channel data_channel)
{

     //if(data_channel == 0xff)
     //      data_channel = g_trk_var.g_user_settings.data_transfer_channel;
     fixed_backup_history_buffer_init();
     fixed_backup_history_buffer_creat();
     send_history_data_with_data();	 	
}

void tracker_caller_id_notify(reply_channel data_channel)
{
	char reply[100]={0};
	int length = 0;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));
     //  if(data_channel == 0xff)
     //        data_channel = g_trk_var.g_user_settings.data_transfer_channel;
	 reply[length] = REPLY_HEADER_CALLERID;length++;
	 reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	 reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	 reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	 reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	 strcpy((reply+length),"1");   //caller_id
	 length = length + strlen("1");
	 result->msgLen =length;
        memcpy(result->body_msg,reply,length);
    	 module_message_buffer_push(result);
	 myFree(result);
}

void tracker_data_session_start_msg(reply_channel data_channel)
{
	char reply[100]={0};
	int length = 0;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));
	reply[length] = REPLY_HEADER_SESSION_START;length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	reply[length] = 0;length++;
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);
}

void tracker_data_session_stop_msg(reply_channel data_channel)
{
	char reply[100]={0};
	int length = 0;   
	 if(g_trk_var.g_user_settings.session_enable==0)
	    return;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));
	reply[length] = REPLY_HEADER_SESSION_STOP;length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	reply[length] = 0;length++;
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);
}

//·¢Ò»¸öack
void tracker_heart_pulse_send(reply_channel data_channel)
{
	char reply[100]={0};
	int length = 0;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));
	reply[length] = REPLY_HEADER_PING;length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);
} 

void trakcer_send_multi_tower_info(reply_channel data_channel)
{
	char reply[100]={0};
	int length = 0;
	int i;
	memset(multi_cell_info_1,0,200);
	memset(multi_cell_info_2,0,200);
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));
	if(data_channel == 0xff)
	data_channel = g_trk_var.g_user_settings.data_transfer_channel;
	reply[length] = REPLY_MULTI_GSM_LOCATION_REPORT_1;length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	for(i=0;i<strlen(multi_cell_info_1);i++)
	{
	if(multi_cell_info_1[i] == '-')
	{   
	reply[length] = 0;length++;
	}
	else
	{
	reply[length] = multi_cell_info_1[i];length++;
	}
	}  
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	if(multi_cell_info_2[0]!= 0)
	{
	length = 0;
	memset(reply,0,100);
	memset(result,0,sizeof(tracker_struct_msg));
	reply[length] = REPLY_MULTI_GSM_LOCATION_REPORT_2;length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	 for(i=0;i<strlen(multi_cell_info_2);i++)
	{
	   if(multi_cell_info_2[i] == '-')
	   {   
	     reply[length] = 0;length++;
	   }
	   else
	   {
	       reply[length] = multi_cell_info_2[i];length++;
	   }
	}  
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	}
	myFree(result);
}	   


/*************************************************************
8 bits - Identification.
32 bits (Optional, in case the message was transmitted over data channel) - Device Id
	8 bits - Expected wakeup hour diff - Optional
	8 bits - Expected wakeup min diff - Optional
	8 bits - Reason (User shutdown, as in scheduled sleep, or Battery Depleted).
	40 bits - Location Latitude
40 bits - Location Longitude
16 bits - Location Altitude
8 bits - Battery
16 bits - MCC1
16 bits - MNC1
16 bits - LAC1
32 bits - CellId1
16 bits - MCC2
16 bits - MNC2
16 bits - LAC2
32 bits - CellId2
16 bits - MCC3
16 bits - MNC3
16 bits - LAC3
32 bits - CellId3
16 bits - MCC4
16 bits - MNC4
16 bits - LAC4
32 bits - CellId4
16 bits - MCC5
16 bits - MNC5
16 bits - LAC5
32 bits - CellId5
16 bits - MCC6
16 bits - MNC6
16 bits - LAC6
32 bits - CellId6

IMPORTANT! The MCC/MNC/LAC/CellId should be sent as NUMBERS and not as Strings.
***********************************************************************/
void tracker_shutdown_message_send(reply_channel data_channel) //kal_uint8 reason,kal_uint16 wake_total_min)
{
	char reply[100]={0};
	int length = 0;  
	U8 wake_hours = 0;
	U8 wake_mins = 0;    
	U16 mcc;
	U16 mnc;
	U16 lac;
	int cellid;
	U8 mcc1;
	U8 mcc2;
	U8 mnc1;
	U8 mnc2;
	U8 lac1;
	U8 lac2;
	U8 cellid1;
	U8 cellid2;
	U8 cellid3;
	U8 cellid4;

	if(g_trk_var.g_user_settings.power_off_message_enable== 0)
	{
	   return;
	}      
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));           
	reply[length] = REPLY_HEADER_SHUTDOWN;length++; 
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	                
	wake_hours = g_trk_var.g_trk_status.wake_total_min>>8;
	wake_mins = g_trk_var.g_trk_status.wake_total_min&0xFF;

	reply[length] = wake_hours;length++;
	reply[length] = wake_mins;length++;

	reply[length] = g_trk_var.g_trk_status.shutdown_reason;length++;

	if(TRUE == gps_2d_3d_fix_with_hdop_and_fix_sec(10))
	{
	do_cordinates_compression((double)g_trk_var.g_trk_content_info.gps_info.latitude / 1000000,(double)g_trk_var.g_trk_content_info.gps_info.longitude / 1000000,(double)g_trk_var.g_trk_content_info.gps_info.speed_km / 1000,COMPRESSION_1);//bcd

	reply[length] = compression_buffer[0];length++;   
	reply[length] = compression_buffer[1];length++;   
	reply[length] = compression_buffer[2];length++;       
	reply[length] = compression_buffer[3];length++;   
	reply[length] = compression_buffer[4];length++;   

	reply[length] = compression_buffer[5];length++;   
	reply[length] = compression_buffer[6];length++;   
	reply[length] = compression_buffer[7];length++;   
	reply[length] = compression_buffer[8];length++;   
	reply[length] = compression_buffer[9];length++;    

	if(g_trk_var.g_trk_content_info.gps_info.gps_state==GPS_STATUS_ON_3D_FIX)
	{ 
	   U16 alt = g_trk_var.g_trk_content_info.gps_info.altitude;
	   U16 alt_high = alt>>8;
	   U16 alt_low = alt&0xFF;
	   
	   reply[length] = alt_high;length++;
	   reply[length] = alt_low;length++;
	}
	else
	{
	   reply[length] = 0;length++;
	   reply[length] = 0;length++;    
	}
	}
	else
	{
	reply[length] = 0;length++;   
	reply[length] = 0;length++;   
	reply[length] = 0;length++;       
	reply[length] = 0;length++;   
	reply[length] = 0;length++;   

	reply[length] = 0;length++;   
	reply[length] = 0;length++;   
	reply[length] = 0;length++;   
	reply[length] = 0;length++;   
	reply[length] = 0;length++;    

	reply[length] = 0;length++;
	reply[length] = 0;length++;   
	}

	//8 bits - Battery
	reply[length] = getBatteryCapacity();length++;

	//gsm info 1
	mcc = 40;//g_trk_var.g_trk_content_info.cell_info[0].mcc ;
	mnc = 6; //g_trk_var.g_trk_content_info.cell_info[0].mnc;
	lac = 10;//g_trk_var.g_trk_content_info.cell_info[0].lac;
	cellid =  8;//g_trk_var.g_trk_content_info.cell_info[0].cellid;

	mcc1 = mcc/256;
	mcc2 = mcc - mcc1*256;

	mnc1 = mnc/256;
	mnc2 = mnc - mnc1*256;

	lac1 = lac/256;
	lac2 = lac - lac1*256;

	cellid1 = cellid/16777216;//256^3
	cellid2 = (cellid - cellid1*16777216)/65536;
	cellid3 = (cellid - cellid1*16777216 - cellid2*65536)/256;
	cellid4 = cellid - cellid1*16777216 - cellid2*65536 - cellid3*256;

	reply[length] = mcc1;length++;
	reply[length] = mcc2;length++;
	reply[length] = mnc1;length++;
	reply[length] = mnc2;length++;
	reply[length] = lac1;length++;
	reply[length] = lac2;length++;
	reply[length] = cellid1;length++;
	reply[length] = cellid2;length++;
	reply[length] = cellid3;length++;
	reply[length] = cellid4;length++;

	//gsm info 2
	mcc = 40;//g_trk_var.g_trk_content_info.cell_info[1].mcc;
	mnc = 10;//g_trk_var.g_trk_content_info.cell_info[1].mnc;
	lac = 4;//g_trk_var.g_trk_content_info.cell_info[1].lac;
	cellid = 6; //g_trk_var.g_trk_content_info.cell_info[1].cellid;

	mcc1 = mcc/256;
	mcc2 = mcc - mcc1*256;

	mnc1 = mnc/256;
	mnc2 = mnc - mnc1*256;

	lac1 = lac/256;
	lac2 = lac - lac1*256;

	cellid1 = cellid/16777216;//256^3
	cellid2 = (cellid - cellid1*16777216)/65536;
	cellid3 = (cellid - cellid1*16777216 - cellid2*65536)/256;
	cellid4 = cellid - cellid1*16777216 - cellid2*65536 - cellid3*256;

	reply[length] = mcc1;length++;
	reply[length] = mcc2;length++;
	reply[length] = mnc1;length++;
	reply[length] = mnc2;length++;
	reply[length] = lac1;length++;
	reply[length] = lac2;length++;
	reply[length] = cellid1;length++;
	reply[length] = cellid2;length++;
	reply[length] = cellid3;length++;
	reply[length] = cellid4;length++;          
	//gsm info 3
	mcc = 40;//g_trk_var.g_trk_content_info.cell_info[2].mcc;
	mnc = 2;//g_trk_var.g_trk_content_info.cell_info[2].mnc;
	lac   = 10;// g_trk_var.g_trk_content_info.cell_info[2].lac;
	cellid = 9;//g_trk_var.g_trk_content_info.cell_info[2].cellid;

	mcc1 = mcc/256;
	mcc2 = mcc - mcc1*256;

	mnc1 = mnc/256;
	mnc2 = mnc - mnc1*256;

	lac1 = lac/256;
	lac2 = lac - lac1*256;

	cellid1 = cellid/16777216;//256^3
	cellid2 = (cellid - cellid1*16777216)/65536;
	cellid3 = (cellid - cellid1*16777216 - cellid2*65536)/256;
	cellid4 = cellid - cellid1*16777216 - cellid2*65536 - cellid3*256;
	reply[length] = mcc1;length++;
	reply[length] = mcc2;length++;
	reply[length] = mnc1;length++;
	reply[length] = mnc2;length++;
	reply[length] = lac1;length++;
	reply[length] = lac2;length++;
	reply[length] = cellid1;length++;
	reply[length] = cellid2;length++;
	reply[length] = cellid3;length++;
	reply[length] = cellid4;length++;
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);
}    

void tracker_server_msg_analyze_get_bt_mac(reply_channel data_channel)
{
	char reply[100]={0};
	U8 bt_mac_address[50]={0};
	int length = 0;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));    
	reply[length] = REPLY_BT_MAC_REPORT;length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	reply[length] = bt_mac_address[0];length++;
	reply[length] = bt_mac_address[1];length++;
	reply[length] = bt_mac_address[2];length++;
	reply[length] = bt_mac_address[3];length++;
	reply[length] = bt_mac_address[4];length++;
	reply[length] = bt_mac_address[5];length++;
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);     
	if(check_ack_setting_bit(ACK_BIT_Get_Bluetooth_MAC_Report) == TRUE)
	{             
	     g_trk_var.g_trk_content_info.command=COMMAND_STATUS;
	     tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
	}     
}


/*********************************************
11. A new ¡°Status Report 3¡± notification message will be sent from the device every time a ¡°Get Status Report¡± control message will be sent. The message body will look as such:

    8 bits - Identification.
32 bits (Optional, in case the message was transmitted over data channel) - Device Id
64 bits - IMEI (BCD) 
String - FW version
8 bits - BT on/off(0=off,1=on)
8 bits - Network preference (0=2G,1=3G,2=LTE).
8 bits - Smart power saving on/off(0=off,1=on)
string - Protocol Version
16 bits - Configuration Version 
string - USSD Prefix
GSM signal level (1 byte)
String - IMSI 
************************************************/
void tracker_server_msg_analyze_status_3(reply_channel data_channel)
{
	char reply[100]={0};
	int length=0;
	U16 number = 0;
	U8 i = 0;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));    
	reply[length] = REPLY_HEADER_STATUS_REPORT_3;length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;

	//64 bits - IMEI (BCD) 
	reply[length] = ( g_trk_var.g_trk_device_status.imei_4G[0]-48)<<4 | ( g_trk_var.g_trk_device_status.imei_4G[1]-48);length++;
	reply[length] = ( g_trk_var.g_trk_device_status.imei_4G[2]-48)<<4 | ( g_trk_var.g_trk_device_status.imei_4G[3]-48);length++;
	reply[length] = ( g_trk_var.g_trk_device_status.imei_4G[4]-48)<<4 | ( g_trk_var.g_trk_device_status.imei_4G[5]-48);length++;
	reply[length] = ( g_trk_var.g_trk_device_status.imei_4G[6]-48)<<4 | ( g_trk_var.g_trk_device_status.imei_4G[7]-48);length++;
	reply[length] = ( g_trk_var.g_trk_device_status.imei_4G[8]-48)<<4 | ( g_trk_var.g_trk_device_status.imei_4G[9]-48);length++;
	reply[length] = ( g_trk_var.g_trk_device_status.imei_4G[10]-48)<<4 | ( g_trk_var.g_trk_device_status.imei_4G[11]-48);length++;
	reply[length] = ( g_trk_var.g_trk_device_status.imei_4G[12]-48)<<4 | ( g_trk_var.g_trk_device_status.imei_4G[13]-48);length++;
	reply[length] = ((( g_trk_var.g_trk_device_status.imei_4G[14]-48)<<4) & 0xf0);length++;

	    //fw version
	strcpy(&(reply[length]),(const char*)TRACKER_COMM_VERSION);
	length = length + strlen((const char*)TRACKER_COMM_VERSION);

	strcpy((reply+length),TRACKER_COMM_BULDTIME);
	length = length + strlen(TRACKER_COMM_BULDTIME);

	reply[length] = 0;length++;

	//8 bits - BT on/off(0=off,1=on)
	reply[length] = g_trk_var.g_user_settings.bt_enable;length++;

	//8 bits - Network preference (0=2G,1=3G,2=LTE).
	reply[length] =  g_trk_var.g_user_settings.network_preference;length++;

	//8 bits - Smart power saving on/off(0=off,1=on)
	reply[length] = g_trk_var.g_user_settings.smart_save_power_enable;length++;

	//string - Protocol Version
	memcpy(&(reply[length]),(const char*)PROTOCOL_VERSION,strlen((const char*)PROTOCOL_VERSION));
	length = length + strlen((const char*)PROTOCOL_VERSION);
	reply[length] = 0;length++;

	//16 bits - Configuration Version 
	reply[length] = g_trk_var.g_user_settings.agps_setting[0];length++;
	reply[length] = g_trk_var.g_user_settings.agps_setting[1];length++;

	//string - USSD Prefix
	strcpy((reply+length),g_trk_var.g_trk_settings.gps_ussd_prefix_read);
	length = length + strlen(g_trk_var.g_trk_settings.gps_ussd_prefix_read);
	reply[length] = 0;length++;

	//GSM signal level (1 byte)
	{
	    U8 signal_strength = 0;
	    signal_strength = 11;//srv_nw_info_get_signal_strength_in_percentage(MMI_SIM1);
	    reply[length] = signal_strength;length++;
	}   
	    
	//Imsis (concatenated strings)
	strcpy((reply+length),g_trk_var.g_trk_device_status.imsi_4G);length = length + strlen(g_trk_var.g_trk_device_status.imsi_4G);
	reply[length] = 0;length++;
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);     
}

/**********************************************************
Button ID (1 byte)
Latitude (5 bytes)
Longitude (5 bytes)
Speed (1 byte)
Duration (1 byte)
************************************************************/
void tracker_send_button_without_wifi(reply_channel data_channel)
{
	char reply[100]={0};
	int length = 0;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));   
	if(TRUE == gps_2d_3d_fix_with_hdop_and_fix_sec(10))
	{   
		do_cordinates_compression((double)g_trk_var.g_trk_content_info.gps_info.latitude / 1000000,(double)g_trk_var.g_trk_content_info.gps_info.longitude / 1000000,(double)g_trk_var.g_trk_content_info.gps_info.speed_km / 1000,COMPRESSION_1);//bcd
		reply[length] = REPLY_BUTTON_WITH_IDENTIFIERS_GPS;length++;
		reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
		reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
		reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
		reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
		reply[length] = g_trk_var.g_trk_device_status.botton_id;length++;  

		reply[length] = compression_buffer[0];length++;    
		reply[length] = compression_buffer[1];length++;    
		reply[length] = compression_buffer[2];length++;        
		reply[length] = compression_buffer[3];length++;    
		reply[length] = compression_buffer[4];length++;    

		reply[length] = compression_buffer[5];length++;    
		reply[length] = compression_buffer[6];length++;    
		reply[length] = compression_buffer[7];length++;    
		reply[length] = compression_buffer[8];length++;    
		reply[length] = compression_buffer[9];length++;    

		//altitude
		if(g_trk_var.g_trk_content_info.gps_info.gps_state==GPS_STATUS_ON_3D_FIX)
		{  
		U16 alt = g_trk_var.g_trk_content_info.gps_info.altitude;
		U16 alt_high = alt/256;
		U16 alt_low = alt - alt_high*256;

		reply[length] = alt_high;length++;
		reply[length] = alt_low;length++;
		}
		else
		{
		reply[length] = 0;length++;
		reply[length] = 0;length++;    
		}
		//speed
		reply[length] = compression_buffer[10];length++;
		//battery
		reply[length] =  getBatteryCapacity();length++;//long press ÔÝ¶¨ÒåÎª1
	}
	else
	{
	    U16 mcc;
	    U16 mnc;
	    U16 lac;
	    int cellid;
	  
	    U8 mcc1;
	    U8 mcc2;
	    U8 mnc1;
	    U8 mnc2;
	    U8 lac1;
	    U8 lac2;
	    U8 cellid1;
	    U8 cellid2;
	    U8 cellid3;
	    U8 cellid4;
	 
	  reply[length] = REPLY_BUTTON_WITH_IDENTIFIERS_GSM;length++;                 
	  reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	  reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	  reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	  reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	  reply[length] = g_trk_var.g_trk_device_status.botton_id;length++; 
	  
	   //battery
	  reply[length] = getBatteryCapacity();length++;
	  
	  mcc = 40;//g_trk_var.g_trk_content_info.cell_info[0].mcc ;
	  mnc =  10;//g_trk_var.g_trk_content_info.cell_info[0].mnc;
	  lac = 6;//g_trk_var.g_trk_content_info.cell_info[0].lac;
	  cellid = 5; //g_trk_var.g_trk_content_info.cell_info[0].cellid;
	 
	  mcc1 = mcc/256;
	  mcc2 = mcc - mcc1*256;
	 
	  mnc1 = mnc/256;
	  mnc2 = mnc - mnc1*256;
	 
	  lac1 = lac/256;
	  lac2 = lac - lac1*256;
	 
	  cellid1 = cellid/16777216;//256^3
	  cellid2 = (cellid - cellid1*16777216)/65536;
	  cellid3 = (cellid - cellid1*16777216 - cellid2*65536)/256;
	  cellid4 = cellid - cellid1*16777216 - cellid2*65536 - cellid3*256; 
	  
	  reply[length] = mcc1;length++;
	  reply[length] = mcc2;length++;
	  reply[length] = mnc1;length++;
	  reply[length] = mnc2;length++;
	  reply[length] = lac1;length++;
	  reply[length] = lac2;length++;
	  reply[length] = cellid1;length++;
	  reply[length] = cellid2;length++;
	  reply[length] = cellid3;length++;
	  reply[length] = cellid4;length++;
	  	
	  mcc = 40;//g_trk_var.g_trk_content_info.cell_info[1].mcc ;
	  mnc =  10;//g_trk_var.g_trk_content_info.cell_info[1].mnc;
	  lac = 6;//g_trk_var.g_trk_content_info.cell_info[1].lac;
	  cellid = 5; //g_trk_var.g_trk_content_info.cell_info[1].cellid;
	 
	  mcc1 = mcc/256;
	  mcc2 = mcc - mcc1*256;
	 
	  mnc1 = mnc/256;
	  mnc2 = mnc - mnc1*256;
	 
	  lac1 = lac/256;
	  lac2 = lac - lac1*256;
	 
	  cellid1 = cellid/16777216;//256^3
	  cellid2 = (cellid - cellid1*16777216)/65536;
	  cellid3 = (cellid - cellid1*16777216 - cellid2*65536)/256;
	  cellid4 = cellid - cellid1*16777216 - cellid2*65536 - cellid3*256; 
	  
	  reply[length] = mcc1;length++;
	  reply[length] = mcc2;length++;
	  reply[length] = mnc1;length++;
	  reply[length] = mnc2;length++;
	  reply[length] = lac1;length++;
	  reply[length] = lac2;length++;
	  reply[length] = cellid1;length++;
	  reply[length] = cellid2;length++;
	  reply[length] = cellid3;length++;
	  reply[length] = cellid4;length++;

	  mcc = 40;//g_trk_var.g_trk_content_info.cell_info[2].mcc ;
	  mnc =  10;//g_trk_var.g_trk_content_info.cell_info[2].mnc;
	  lac = 6;//g_trk_var.g_trk_content_info.cell_info[2].lac;
	  cellid = 5; //g_trk_var.g_trk_content_info.cell_info[2].cellid;
	 
	  mcc1 = mcc/256;
	  mcc2 = mcc - mcc1*256;
	 
	  mnc1 = mnc/256;
	  mnc2 = mnc - mnc1*256;
	 
	  lac1 = lac/256;
	  lac2 = lac - lac1*256;
	 
	  cellid1 = cellid/16777216;//256^3
	  cellid2 = (cellid - cellid1*16777216)/65536;
	  cellid3 = (cellid - cellid1*16777216 - cellid2*65536)/256;
	  cellid4 = cellid - cellid1*16777216 - cellid2*65536 - cellid3*256; 
	  
	  reply[length] = mcc1;length++;
	  reply[length] = mcc2;length++;
	  reply[length] = mnc1;length++;
	  reply[length] = mnc2;length++;
	  reply[length] = lac1;length++;
	  reply[length] = lac2;length++;
	  reply[length] = cellid1;length++;
	  reply[length] = cellid2;length++;
	  reply[length] = cellid3;length++;
	  reply[length] = cellid4;length++;

	}

	//IMEI (16 digits - BCD)(8 bytes)
	reply[length] = ( g_trk_var.g_trk_device_status.imei_4G[0]-48)<<4 | ( g_trk_var.g_trk_device_status.imei_4G[1]-48);length++;
	reply[length] = ( g_trk_var.g_trk_device_status.imei_4G[2]-48)<<4 | ( g_trk_var.g_trk_device_status.imei_4G[3]-48);length++;
	reply[length] = ( g_trk_var.g_trk_device_status.imei_4G[4]-48)<<4 | ( g_trk_var.g_trk_device_status.imei_4G[5]-48);length++;
	reply[length] = ( g_trk_var.g_trk_device_status.imei_4G[6]-48)<<4 | ( g_trk_var.g_trk_device_status.imei_4G[7]-48);length++;
	reply[length] = ( g_trk_var.g_trk_device_status.imei_4G[8]-48)<<4 | ( g_trk_var.g_trk_device_status.imei_4G[9]-48);length++;
	reply[length] = ( g_trk_var.g_trk_device_status.imei_4G[10]-48)<<4 | ( g_trk_var.g_trk_device_status.imei_4G[11]-48);length++;
	reply[length] = ( g_trk_var.g_trk_device_status.imei_4G[12]-48)<<4 | ( g_trk_var.g_trk_device_status.imei_4G[13]-48);length++;
	reply[length] = ((( g_trk_var.g_trk_device_status.imei_4G[14]-48)<<4) & 0xf0);length++;

	//imsi
	strcpy((reply+length),g_trk_var.g_trk_device_status.imsi_4G);length = length + strlen(g_trk_var.g_trk_device_status.imsi_4G);
	reply[length] = 0;length++;
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);     
}   

/*********************************************
Message ID  - 1 byte (34 in dec)
GPS timer 1 - 1 bytes , default 0
GPS timer 2 - 1 bytes , default 0
Fence speed timer - 1 bytes , default 0
Gps on cpu sleep timer - 1 bytes , default 0
Gps on cpu wake up timer - 1 bytes , default 0
Indoor checking condition sat number - 1 bytes , default 0
Indoor checking condition sat max signal - 1 bytes , default 0
Bluetooth Watchdog on/off - 1 bytes , default 0
Scheduled sleep on/off - 1 bytes , default 0
Scheduled sleep time diff in minutes - 2 bytes , default 0
Scheduled sleep period in minutes - 2 bytes , default 0
Scheduled sleep awake period in  - 2 bytes , default 0
Scheduled sleep send GPS and sleep - 1 bytes , default 0
Scheduled sleep repeat - 1 bytes , default 0


////////////search mode///////////////////
Timer1 (1 byte),
Timer2 (1 byte),

Ç°2¸öbyteÊÇÒÔÇ°2gÓÃµÄ

fence_speed_timer (1 byte in sec)
gps_on_cpu_sleep_timer (1 byte in sec/10, 55 is 5.5sec)
gps_on_cpu_wake_timer (1 byte in sec/10, 15 is 1.5sec)
indoor_checking_condition_sat_number (1 byte)
indoor_checking_condition_sat_max_signal (1 byte)
//////////////////////////////////////////////////////////
************************************************/
void tracker_server_msg_analyze_status_4(reply_channel data_channel)
{
      char reply[100]={0};
      U16 length=0;
      U16 number = 0;
      int i;
      char* build_time = NULL;
      tracker_struct_msg *result;
      result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
      memset(result,0,sizeof(tracker_struct_msg));   
	reply[length] = REPLY_HEADER_STATUS_REPORT_4;length++;
	//dataÐè¼Ódevice id
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;

	//GPS timer 1 - 1 bytes , default 0
	reply[length] =  g_trk_var.g_trk_settings.gps_search_control[0] ;length++;

	//GPS timer 2 - 1 bytes , default 0
	reply[length] =  g_trk_var.g_trk_settings.gps_search_control[1];length++;

	//Fence speed timer - 1 bytes , default 0
	reply[length] =  g_trk_var.g_trk_settings.gps_search_control[2];length++;

	//Gps on cpu sleep timer - 1 bytes , default 0
	reply[length] =  g_trk_var.g_trk_settings.gps_search_control[3];length++;

	//Gps on cpu wake up timer - 1 bytes , default 0
	reply[length] =  g_trk_var.g_trk_settings.gps_search_control[4];length++;

	//Indoor checking condition sat number - 1 bytes , default 0
	reply[length] =  g_trk_var.g_trk_settings.gps_search_control[5];length++;

	//Indoor checking condition sat max signal - 1 bytes , default 0
	reply[length] =  g_trk_var.g_trk_settings.gps_search_control[6] ;length++;

	reply[length] =  g_trk_var.g_trk_settings.gps_search_control[7];length++;

	//Scheduled sleep on/off - 1 bytes , default 0
	reply[length] = g_trk_var.g_trk_settings.gps_schedule_sleep[0];length++;

	//Scheduled sleep time diff in minutes - 2 bytes , default 0
	reply[length] = g_trk_var.g_trk_settings.gps_schedule_sleep[1];length++;
	reply[length] = g_trk_var.g_trk_settings.gps_schedule_sleep[2];length++;

	//Scheduled sleep period in minutes - 2 bytes , default 0
	reply[length] = g_trk_var.g_trk_settings.gps_schedule_sleep[3];length++;
	reply[length] = g_trk_var.g_trk_settings.gps_schedule_sleep[4];length++;

	//Scheduled sleep awake period in  - 2 bytes , default 0
	reply[length] = g_trk_var.g_trk_settings.gps_schedule_sleep[5];length++;
	reply[length] = g_trk_var.g_trk_settings.gps_schedule_sleep[6];length++;

	//Scheduled sleep send GPS and sleep - 1 bytes , default 0
	reply[length] = g_trk_var.g_trk_settings.gps_schedule_sleep[7];length++;

	//Scheduled sleep repeat - 1 bytes , default 0
	reply[length] = g_trk_var.g_trk_settings.gps_schedule_sleep[8];length++;
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);     
}

void tracker_reply_server_sequence_success(reply_channel data_channel)
{
	char reply[100]={0};
	int length = 0;
	U8 sequence_1 = g_trk_var.g_trk_content_info.sequence_1;
	U8 sequence_2 = g_trk_var.g_trk_content_info.sequence_2;
	#ifdef __SIM_DEFAULT_CHANNEL_DATA__
	 if(g_trk_var.g_user_settings.check_ack_timer_enable==TRUE)
	     return;
	#endif
	tracker_struct_msg *result;
       result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
       memset(result,0,sizeof(tracker_struct_msg));   
	 reply[length] = REPLY_ACK_SEQUENTIAL;length++;
	 reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	 reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	 reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	 reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;            
	 reply[length] = g_trk_var.g_trk_content_info.command;length++;
	 reply[length] = sequence_1;length++;
	 reply[length] = sequence_2;length++;
	 result->msgLen =length;
	 memcpy(result->body_msg,reply,length);
	 module_message_buffer_push(result);
	 myFree(result);   
}

  
void tracker_server_msg_analyze_test_component(reply_channel data_channel)
{
	U8 test_component = 0;
	U8 test_component2 = 0;
	BOOL ret = FALSE;
	U8 ls1_0,ls1_1,ls2_0,ls2_1,ls1_id,ls2_id;
	int  g_x,g_y,g_z;
	U8 num_charge = 0;
	U8 cali_ret = 1;//track_calibration_status();
	U8 signal_strength = 13;//srv_nw_info_get_signal_strength_in_percentage(MMI_SIM1);
	U8 reply[100]={0};
	int length = 0;
	test_component = TEST_COMPO_ALL_INFO_1;
	tracker_struct_msg *result;
       result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
       memset(result,0,sizeof(tracker_struct_msg));   
	#ifdef   SUPPORT_MY_GSENSOR
	gSensor_get_data(&g_x,&g_y,&g_z);
	#endif
        if(is_charging()==TRUE)
        {
                num_charge = 1;
        }
        else
        {
                num_charge = 0;
        }           
	 reply[length] = REPLY_UNIVERSAL_TEST;length++;
	 reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	 reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	 reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	 reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	 reply[length] = 0;length++;
	 reply[length] = 0;length++;
	 reply[length] = 0;length++;
	 reply[length] = 0;length++;
	 reply[length] = 0;length++;
	 reply[length] = 0;length++;
	 reply[length] = 0;length++;
	 reply[length] = 0;length++;
	 reply[length] = cali_ret;length++;
	 reply[length] = signal_strength;length++;
	 reply[length] = getBatteryCapacity();length++;
	 reply[length] = num_charge;length++;
 	 result->msgLen =length;
	 memcpy(result->body_msg,reply,length);
	 module_message_buffer_push(result);
	 myFree(result);         
} 

void tracker_send_only_wifi_info(reply_channel data_channel,BOOL server_reply)
{
	U8 i=0;	 
	U16 wifi_channel;
	U16 wifi_freq;
	U16 wifi_signal;
	U8 wifi_channel_high;
	U8 wifi_channel_low;
	U8 wifi_freq_high;
	U8 wifi_freq_low;
	U8 wifi_signal_high;
	U8 wifi_signal_low;
	U8 nCountryCode_high;
	U8 nCountryCode_low;
	U8 nNetworkCode_high;
	U8 nNetworkCode_low;
	U8 nLac_high;
	U8 nLac_low;
	U8 nCellid_3;
	U8 nCellid_2;
	U8 nCellid_1;
	U8 nCellid_0;
	U8 reply[100]={0};
	int length = 0;	 
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg)); 
	if(data_channel == 0xff)
	data_channel = g_trk_var.g_user_settings.data_transfer_channel;
	reply[length] = REPLY_AGPS_BOOTSTRAP_REQUEST;length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	if(server_reply == FALSE)
	{
	    reply[length] = 0;length++;
	}
	else
	{
	    reply[length] = 1;length++;
	}  
	reply[length] = GetBatteryLevel();length++;              
	//×î¶à·¢3¸öwifi
	for(i=0;i<3;i++)
	{
	 reply[length] = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[0];length++;
	 reply[length] = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[1];length++;
	 reply[length] = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[2];length++;
	 reply[length] = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[3];length++;
	 reply[length] = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[4];length++;
	 reply[length] = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[5];length++;
	 wifi_channel = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].channel_number;
	 wifi_channel_high = wifi_channel>>8;
	 wifi_channel_low = wifi_channel&0xff;
	 wifi_freq = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].frequency;
	 wifi_freq_high = wifi_freq>>8;
	 wifi_freq_low = wifi_freq&0xff;
	 if(g_trk_var.g_trk_content_info.wifi_info.ap_list[i].rssi > 0)
	 { 
	     wifi_signal = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].rssi;
	 }
	 else
	 {
	    wifi_signal = 0 - g_trk_var.g_trk_content_info.wifi_info.ap_list[i].rssi; 
	 }         
	 wifi_signal_high = wifi_signal>>8;
	 wifi_signal_low = wifi_signal&0xff;      
	  reply[length] = wifi_channel_high;length++;
	  reply[length] = wifi_channel_low;length++;
	  reply[length] = wifi_freq_high;length++;
	  reply[length] = wifi_freq_low;length++;
	  reply[length] = wifi_signal_high;length++;
	  reply[length] = wifi_signal_low;length++;
	}
	nCountryCode_high = 0;//g_trk_var.g_trk_content_info.cell_info[0].mcc>>8;
	nCountryCode_low = 0;//g_trk_var.g_trk_content_info.cell_info[0].mcc&0xff;  

	nNetworkCode_high =  0;//g_trk_var.g_trk_content_info.cell_info[0].mnc>>8;
	nNetworkCode_low =  0;//g_trk_var.g_trk_content_info.cell_info[0].mnc&0xff;    
	   
	nLac_high = 0;//g_trk_var.g_trk_content_info.cell_info[0].lac>>8;
	nLac_low = 0;//g_trk_var.g_trk_content_info.cell_info[0].lac&0xff;      
	   
	nCellid_3 =  0;//g_trk_var.g_trk_content_info.cell_info[0].cellid/16777216;//256^3
	nCellid_2 = 0;//( g_trk_var.g_trk_content_info.cell_info[0].cellid - nCellid_3*16777216)/65536;
	nCellid_1 = 0;//( g_trk_var.g_trk_content_info.cell_info[0].cellid - nCellid_3*16777216 - nCellid_2*65536)/256;
	nCellid_0 =  0;//g_trk_var.g_trk_content_info.cell_info[0].cellid - nCellid_3*16777216 - nCellid_2*65536 - nCellid_1*256;      

	reply[length] = nCountryCode_high;length++;
	reply[length] = nCountryCode_low;length++;
	reply[length] = nNetworkCode_high;length++;
	reply[length] = nNetworkCode_low;length++;
	reply[length] = nLac_high;length++;
	reply[length] = nLac_low;length++;       
	reply[length] = nCellid_3;length++;
	reply[length] = nCellid_2;length++;     
	reply[length] = nCellid_1;length++;
	reply[length] = nCellid_0;length++;    
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);   
}	

void tracker_send_button_with_wifi_info(reply_channel data_channel)
{
	BOOL reply_need = FALSE;
	int  i;
	U16 wifi_channel;
	U16 wifi_freq;
	U16 wifi_signal;
	U8 wifi_channel_high;
	U8 wifi_channel_low;
	U8 wifi_freq_high;
	U8 wifi_freq_low;
	U8 wifi_signal_high;
	U8 wifi_signal_low;
	U8 nCountryCode_high;
	U8 nCountryCode_low;
	U8 nNetworkCode_high;
	U8 nNetworkCode_low;
	U8 nLac_high;
	U8 nLac_low;
	U8 nCellid_3;
	U8 nCellid_2;
	U8 nCellid_1;
	U8 nCellid_0;
	int length = 0; 
	U8 reply[100]={0};
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg)); 
	if(data_channel == 0xff)
	data_channel = g_trk_var.g_user_settings.data_transfer_channel;
	reply[length] = REPLY_BUTTON_WITH_WIFI_LOCATION;length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	reply[length] = g_trk_var.g_trk_device_status.botton_id;length++;
	reply[length] = GetBatteryLevel();length++;              

	//×î¶à·¢3¸öwifi
	for(i=0;i<3;i++)
	{
	reply[length] = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[0];length++;
	reply[length] = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[1];length++;
	reply[length] = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[2];length++;
	reply[length] = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[3];length++;
	reply[length] = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[4];length++;
	reply[length] = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[5];length++;
	wifi_channel = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].channel_number;
	wifi_channel_high = wifi_channel>>8;
	wifi_channel_low = wifi_channel&0xff;
	wifi_freq = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].frequency;
	wifi_freq_high = wifi_freq>>8;
	wifi_freq_low = wifi_freq - wifi_freq_high&0xff;
	if(g_trk_var.g_trk_content_info.wifi_info.ap_list[i].rssi > 0)
	{ 
	wifi_signal = g_trk_var.g_trk_content_info.wifi_info.ap_list[i].rssi;
	}
	else
	{
	wifi_signal = 0 - g_trk_var.g_trk_content_info.wifi_info.ap_list[i].rssi; 
	}         
	wifi_signal_high = wifi_signal>>8;
	wifi_signal_low = wifi_signal - wifi_signal_high&0xff;      
	reply[length] = wifi_channel_high;length++;
	reply[length] = wifi_channel_low;length++;
	reply[length] = wifi_freq_high;length++;
	reply[length] = wifi_freq_low;length++;
	reply[length] = wifi_signal_high;length++;
	reply[length] = wifi_signal_low;length++;
	}
	nCountryCode_high = 0;//g_trk_var.g_trk_content_info.cell_info[0].mcc>>8;
	nCountryCode_low = 0;//g_trk_var.g_trk_content_info.cell_info[0].mcc&0xff;  
	nNetworkCode_high =  0;//g_trk_var.g_trk_content_info.cell_info[0].mnc>>8;
	nNetworkCode_low =  0;//g_trk_var.g_trk_content_info.cell_info[0].mnc&0xff;    
	nLac_high = 0;//g_trk_var.g_trk_content_info.cell_info[0].lac>>8;
	nLac_low = 0;//g_trk_var.g_trk_content_info.cell_info[0].lac&0xff;      
	nCellid_3 =  0;//g_trk_var.g_trk_content_info.cell_info[0].cellid/16777216;//256^3
	nCellid_2 = 0;//( g_trk_var.g_trk_content_info.cell_info[0].cellid - nCellid_3*16777216)/65536;
	nCellid_1 = 0;//( g_trk_var.g_trk_content_info.cell_info[0].cellid - nCellid_3*16777216 - nCellid_2*65536)/256;
	nCellid_0 =  0;//g_trk_var.g_trk_content_info.cell_info[0].cellid - nCellid_3*16777216 - nCellid_2*65536 - nCellid_1*256;      
	reply[length] = nCountryCode_high;length++;
	reply[length] = nCountryCode_low;length++;
	reply[length] = nNetworkCode_high;length++;
	reply[length] = nNetworkCode_low;length++;
	reply[length] = nLac_high;length++;
	reply[length] = nLac_low;length++;       
	reply[length] = nCellid_3;length++;
	reply[length] = nCellid_2;length++;     
	reply[length] = nCellid_1;length++;
	reply[length] = nCellid_0;length++;   
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);   
}	

/*******************************************************
Message Id (int , 1 byte)
Location reply needed (Switch, 1 byte)
Battery status (int, 1 byte)
bssid1 (MacAddress, 6 bytes)
channel1 (int, 2 byte)
frequency1 (int, 2 byte)
signal1 (int, 2 byte)
bssid2 (MacAddress, 6 bytes)
channel2 (int, 2 byte)
frequency2 (int, 2 byte)
signal2 (int, 2 byte)
bssid3 (MacAddress, 6 bytes)
channel3 (int, 2 byte)
frequency3 (int, 2 byte)
signal3 (int, 2 byte)
MCC (int, 2 byte)
MNC (int, 2 byte)
LAC (int, 2 byte)
CellId (int, 4 byte)
********************************************************/
void tracker_send_wifi_info_to_server(reply_channel data_channel)
{
	if(wifi_read_from[WIFI_READ_FROM_NO_SERVER_REPLY] == TRUE)
	{
		main_printf("WIFI_READ_FROM_NO_SERVER_REPLY\r\n");
		wifi_read_from[WIFI_READ_FROM_NO_SERVER_REPLY] = FALSE;
		//Ã»ÊÕµ½wifi
		if(!isScan())
		{
		main_printf("g_trk_var.g_trk_content_info.wifi_info.ap_list_num == 0, return\r\n");
		}
		else
		{	
		tracker_send_only_wifi_info(data_channel,FALSE);
		}   
	}

	if(wifi_read_from[WIFI_READ_FROM_NEED_SERVER_REPLY] == TRUE)
	{
		main_printf("WIFI_READ_FROM_NEED_SERVER_REPLY\r\n");
		wifi_read_from[WIFI_READ_FROM_NEED_SERVER_REPLY] = FALSE;
		//Ã»ÊÕµ½wifi
		if(!isScan())
		{
		main_printf("g_trk_var.g_trk_content_info.wifi_info.ap_list_num == 0, return\r\n");
		}
		else
		{	
		tracker_send_only_wifi_info(data_channel,TRUE);
		} 
	}		

	if(wifi_read_from[WIFI_READ_FROM_GET_GPS] == TRUE)
	{
		//Í£ÄÇ±ßµÄtimer
		stopMyTimer(TRACKER_WIFI_TIMER_2);
		main_printf("WIFI_READ_FROM_GET_GPS\r\n");
		wifi_read_from[WIFI_READ_FROM_GET_GPS] = FALSE;
		//Ã»ÊÕµ½wifi
		if(!isScan())
		{
		main_printf("g_trk_var.g_trk_content_info.wifi_info.ap_list_num == 0\r\n");
		#ifdef SUPPORT_MY_WIFI
		if(TRUE == Is_get_gps_on())
		{	
		//tracker_protocol_send_deal(REPLY_GSM_LOCATION_REPORT,data_channel);                      
		//tracker_protocol_send_deal(REPLY_MULTI_GSM_LOCATION_REPORT_1,data_channel);
		}  
		#endif
		}
		else
		{	
		tracker_send_only_wifi_info(data_channel,FALSE);
		}

	}	

	if(wifi_read_from[WIFI_READ_FROM_BOTTON] == TRUE)
	{	
		//Í£ÄÇ±ßµÄtimer
		stopMyTimer(TRACKER_WIFI_TIMER_3);
		main_printf("WIFI_READ_FROM_BOTTON\r\n");
		wifi_read_from[WIFI_READ_FROM_BOTTON] = FALSE;
		if(!isScan())
		{
		//tracker_send_button_without_wifi(data_channel);
		}
		else
		{
		tracker_send_button_with_wifi_info(data_channel);
		}
	} 
}	  		

void tracker_server_msg_analyze_status_dynamic(reply_channel data_channel)
{
	U8 reply[100]={0};
	int length = 0;
	int i;
	U16 number = 0;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));   

	reply[length] = REPLY_DYNAMIC_STATUS_REPORT;length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	//AGPS1 ONOFF
	reply[length] = get_uint16_byte_high(DYNAMIC_ID_AGPS1_EPO);length++;
	reply[length] = get_uint16_byte_low(DYNAMIC_ID_AGPS1_EPO);length++;

	reply[length] = g_trk_var.g_user_settings.agps1_enable;length++;

	//AGPS2 ONOFF
	reply[length] = get_uint16_byte_high(DYNAMIC_ID_AGPS2_GPS_IN);length++;
	reply[length] = get_uint16_byte_low(DYNAMIC_ID_AGPS2_GPS_IN);length++;

	reply[length] = g_trk_var.g_user_settings.agps2_enable;length++;

	//AGPS1_EPO_THIS_POWERUP
	reply[length] = get_uint16_byte_high(DYNAMIC_ID_AGPS1_EPO_THIS_POWERUP);length++;
	reply[length] = get_uint16_byte_low(DYNAMIC_ID_AGPS1_EPO_THIS_POWERUP);length++;

	reply[length] = get_epo_power_up_status();length++;	  

	#ifdef __TRACKIMO_SMART_WIFI__
	//smart wifi onoff	  
	reply[length] = get_uint16_byte_high(DYNAMIC_ID_SMART_WIFI_ONOFF);length++;
	reply[length] = get_uint16_byte_low(DYNAMIC_ID_SMART_WIFI_ONOFF);length++;

	reply[length] = g_trk_var.g_user_settings.smart_wifi_enable[0];length++;	  
	#endif


	//DYNAMIC_ID_SERVER_CONTROL_DEVICEID	  
	reply[length] = get_uint16_byte_high(DYNAMIC_ID_SERVER_CONTROL_DEVICEID);length++;
	reply[length] = get_uint16_byte_low(DYNAMIC_ID_SERVER_CONTROL_DEVICEID);length++;

	reply[length] = g_trk_var.g_user_settings.service_control_deviceid_enable;length++;	  

	#ifdef __SIM_DEFAULT_CHANNEL_DATA__
	//DYNAMIC_ID_DATA_ACK_MINUTES	  
	reply[length] = get_uint16_byte_high(DYNAMIC_ID_DATA_ACK_MINUTES);length++;
	reply[length] = get_uint16_byte_low(DYNAMIC_ID_DATA_ACK_MINUTES);length++;

	reply[length] = g_trk_var.g_user_settings.check_ack_timer_in_min;length++;

	//DYNAMIC_ID_DEVICE_ACK_DISABLE	  
	reply[length] = get_uint16_byte_high(DYNAMIC_ID_DEVICE_ACK_DISABLE);length++;
	reply[length] = get_uint16_byte_low(DYNAMIC_ID_DEVICE_ACK_DISABLE);length++;

	reply[length] = g_trk_var.g_user_settings.check_ack_timer_enable;length++;	  
	#endif    
	 result->msgLen =length;
	 memcpy(result->body_msg,reply,length);
	 module_message_buffer_push(result);
	 myFree(result); 	 	
}

void tracker_device_send_get_ack(reply_channel data_channel)
{

	U8 reply[100]={0};
	int length = 0;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));  
	reply[length] = REPLY_GET_ACK;length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);  
} 

void tracker_reply_server_msg_error(reply_channel data_channel)
{
	U8 reply[100]={0};
	int length = 0;
	tracker_struct_msg *result;
	result = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memset(result,0,sizeof(tracker_struct_msg));  
	reply[length]  = REPLY_HEADER_ERROR;length++;
	//dataÒª¼Ódevice id
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[0];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[1];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[2];length++;
	reply[length] = g_trk_var.g_trk_device_status.deviceid_4G[3];length++;
	reply[length] = g_trk_var.g_trk_content_info.command;length++;
	result->msgLen =length;
	memcpy(result->body_msg,reply,length);
	module_message_buffer_push(result);
	myFree(result);   
}

