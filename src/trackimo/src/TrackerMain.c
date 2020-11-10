#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "TrackerMain.h"
#include "TrackerCfg.h"
#include "MyDefine.h"
#include "MyLog.h"
#include "MainTask.h"
#include "MyCommon.h"
#include "MyLed.h"
#include "MyTimer.h"
#include "MyBuzzer.h"
#include "MyGPS.h"
#include "MyWifi.h"
#include "TrackerDecodeIn.h"
#include "TrackimoFrame.h"
#include "My4G.h"
#include "MyFile.h"
trk_variable g_trk_var = {0};
static unsigned int tracker_work_teps=0;
static unsigned int tracker_network_teps=0;
int auto_reboot_timer_in_hour=0;
BOOL agps1_epo_on = FALSE;
BOOL apgs2_gps_in = FALSE;

/************************************
0:  AGPS1_EPO - OFF, AGPS2_GPS_IN - OFF 
1:  AGPS1_EPO - ON, AGPS2_GPS_IN - OFF 
2:  AGPS1_EPO - OFF, AGPS2_GPS_IN - ON 
3:  AGPS1_EPO - ON, AGPS2_GPS_IN - ON 
*************************************/
void agps_setting_init(void)
{
	 U8 agps_setting = g_trk_var.g_user_settings.agps_setting[0];
	 if(g_trk_var.g_user_settings.agps1_enable> 0)   
	 {
	    agps1_epo_on = TRUE;
	    maintask_printf("agps1_epo_on = KAL_TRUE\r\n");
	 }
	 else
	 {
	    agps1_epo_on = FALSE;
	    maintask_printf("agps1_epo_on = KAL_FALSE\r\n");
	 }
	 
	 if(g_trk_var.g_user_settings.agps2_enable> 0)
	 {
	    apgs2_gps_in = TRUE;
	    maintask_printf("apgs2_gps_in = KAL_TRUE\r\n");
	 }
	 else
	 {
	    apgs2_gps_in = FALSE;
	    maintask_printf("apgs2_gps_in = KAL_FALSE;r\n");
	 }             
}  

void mmi_tracker_set_deviceid_by_imei(char* imei,tracker_work_mode_type_enum work_mode)
{
	U8 i=0;
	char device_id[10];
	U8 device_id_number[10] = {0,0,0,0,0,0,0,0,0,0};
	U32 deviceid_final=0; 
	U8 deviceid1=0; 
	U8 deviceid2=0; 
	U8 deviceid3=0; 
	U8 deviceid4=0; 
	if(work_mode==TRACKER_MODE_SWITCH_4G)
	{
		device_id[0] = DEVICEID_4G_NUMBER_0;
		device_id[1] = DEVICEID_4G_NUMBER_1;
		device_id[2] = DEVICEID_4G_NUMBER_2;
		device_id[3] = imei[8];
		device_id[4] = imei[9];
		device_id[5] = imei[10];
		device_id[6] = imei[11];
		device_id[7] = imei[12];
		device_id[8] = imei[13];
		for(i=0;i<10;i++)
		{
		char temp_str[2];
		temp_str[0] = device_id[i];
		temp_str[1] = 0;
		device_id_number[i] = atoi(temp_str);                 
		deviceid_final = 999217949; //DEVICEID_HEADER_4G + (U32)device_id_number[3]* 100000 + (U32)device_id_number[4]* 10000 + (U32)device_id_number[5]* 1000 + (U32)device_id_number[6]* 100 + (U32)device_id_number[7]* 10 + (U32)device_id_number[8];     
		}
		maintask_printf("deviceid_final = %d\r\n",deviceid_final);
		deviceid1 = deviceid_final>>24;
		deviceid2 = (deviceid_final>>16)&0xff;
		deviceid3 = (deviceid_final>>8)&0xff;
		deviceid4 = deviceid_final&0xff;  

		//荷兰
		//999217988  0x3B8EDB44
		//999217989  0x3B8EDB45

		if(g_trk_var.g_trk_device_status.deviceid_4G[0]!=deviceid1&&
		g_trk_var.g_trk_device_status.deviceid_4G[1]!=deviceid2&&
		g_trk_var.g_trk_device_status.deviceid_4G[2]!=deviceid3&&
		g_trk_var.g_trk_device_status.deviceid_4G[3]!=deviceid4)
		{     
		g_trk_var.g_trk_device_status.deviceid_4G[0]=deviceid1;
		g_trk_var.g_trk_device_status.deviceid_4G[1]=deviceid2;
		g_trk_var.g_trk_device_status.deviceid_4G[2]=deviceid3;
		g_trk_var.g_trk_device_status.deviceid_4G[3]=deviceid4;
		}
		g_trk_var.g_user_settings.deviceid[0]= g_trk_var.g_trk_device_status.deviceid_4G[0];
		g_trk_var.g_user_settings.deviceid[1]= g_trk_var.g_trk_device_status.deviceid_4G[1];
		g_trk_var.g_user_settings.deviceid[2]= g_trk_var.g_trk_device_status.deviceid_4G[2];
		g_trk_var.g_user_settings.deviceid[3]= g_trk_var.g_trk_device_status.deviceid_4G[3];
	}
}

void idle_lte_set_imei_callback_func(char* imei,int len)
{
	int i;
	memset(g_trk_var.g_trk_device_status.imei_4G,0,TRK_CFG_IMEI_LENGTH);
	for(i=0;i<len;i++)
	{
	   g_trk_var.g_trk_device_status.imei_4G[i]=imei[i];
	}
	mmi_tracker_set_deviceid_by_imei(g_trk_var.g_trk_device_status.imei_4G,TRACKER_MODE_SWITCH_4G);
	maintask_printf("idle_lte_set_imei_callback_func imei=%s\r\n", g_trk_var.g_trk_device_status.imei_4G);
}

void idle_lte_sim_status_callback_func(char* imsi,int len)
{
	int i;
	memset(g_trk_var.g_trk_device_status.imsi_4G,0,TRK_CFG_IMSI_LENGTH);
	for(i=0;i<len;i++)
	{
	   g_trk_var.g_trk_device_status.imsi_4G[i]=imsi[i];
	}
	maintask_printf("idle_lte_sim_status_callback_func imsi=%s\r\n", g_trk_var.g_trk_device_status.imsi_4G);
}


//默认平台设置    //主要是一些平台数据
void tracker_global_default_set()
{
	maintask_printf("tracker_global_default_set\r\n");
	int i;
	g_trk_var.g_trk_settings.bInit = TRUE;
	g_trk_var.g_user_settings.platform_url.host.ip[0] = TRACKER_IP_DEFAULT_IP0;
	g_trk_var.g_user_settings.platform_url.host.ip[1] = TRACKER_IP_DEFAULT_IP1;
	g_trk_var.g_user_settings.platform_url.host.ip[2] = TRACKER_IP_DEFAULT_IP2;
	g_trk_var.g_user_settings.platform_url.host.ip[3] = TRACKER_IP_DEFAULT_IP3;
	g_trk_var.g_user_settings.platform_url.port = TRACKER_HTTP_PORT;
	strcpy(g_trk_var.g_user_settings.platform_url.apn,TRACKER_IP_DEFAULT_APN);
	g_trk_var.g_user_settings.data_transfer_channel= REPLY_CHANNEL_DATA_MODULE;  //默认2G跟CAT-M1一起工作    //REPLY_CHANNEL_DATA_MODULE  REPLY_CHANNEL_DATA_UDP
	g_trk_var.g_user_settings.beeper_enable=1;
	g_trk_var.g_user_settings.tracking_alarm_in_10s=5;
	g_trk_var.g_user_settings.no_heart_pulse_reboot_in_min=2;
	g_trk_var.g_user_settings.bt_enable=1;
	g_trk_var.g_user_settings.current_session_max_bytes=100;
	g_trk_var.g_user_settings.check_ack_timer_in_min=5;
	g_trk_var.g_user_settings.ussd_lock_timer=10;
	g_trk_var.g_user_settings.gps_backup_enable=1;
	g_trk_var.g_trk_device_status.power_on_status =1;
	if(g_trk_var.g_user_settings.data_transfer_channel==REPLY_CHANNEL_DATA_MODULE)
	{
		g_trk_var.g_trk_settings.track_channel = g_trk_var.g_user_settings.data_transfer_channel;
		g_trk_var.g_trk_settings.sample_time=5;  //系统默认5分钟开一次GPS
		g_trk_var.g_trk_settings.report_time=1; 
		g_trk_var.g_trk_settings.location_accuracy=0;
		g_trk_var.g_trk_settings.track_setting=0x9c; 
	}
	//*126*
	g_trk_var.g_trk_settings.gps_ussd_prefix_read[0]=0x2A;  //ascii 对应*
	g_trk_var.g_trk_settings.gps_ussd_prefix_read[1]=0x31;  //ascii 对应1
	g_trk_var.g_trk_settings.gps_ussd_prefix_read[2]=0x32;  //ascii 对应2
	g_trk_var.g_trk_settings.gps_ussd_prefix_read[3]=0x36;  //ascii 对应6
	g_trk_var.g_trk_settings.gps_ussd_prefix_read[4]=0x2A;  //ascii 对应*

	g_trk_var.g_trk_settings.gps_moving_alarm_setting[3]=0xff;
	g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[3]=0xff;
	g_trk_var.g_trk_settings.gps_moving_alarm_setting[2] = 21;
	g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[2] = 20;
	g_trk_var.g_trk_settings.gps_moving_alarm_setting[4] = 1;
	g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[4]= 0;
	for(i=0;i<5;i++)
	{ 
		g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][2] = 40;
		g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][4] = 1;
		g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][3] = 0xff;  
		g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][6] = 20;
		g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][7] = 1;    //  1是超速
		
		//g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][3]=0xff;
		g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][3]=0xff;
		g_trk_var.g_trk_settings.gps_battery_alarm_setting[i][2]=31;
		g_trk_var.g_trk_settings.gps_battery_alarm_setting[i][4]=1;
		g_trk_var.g_trk_settings.gps_battery_alarm_setting[i][6]=20;

		g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][3]=0xff;
		g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][2]=62;
		g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][4] = 1;
		g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][6] = 2;
		g_trk_var.g_trk_settings.gps_fance_cordinates[4*i][0] = "22.535401";
		g_trk_var.g_trk_settings.gps_fance_cordinates[4*i+1][1] = "114.042103";
		g_trk_var.g_trk_settings.gps_fance_cordinates[4*i+2][2] = "22.531402";
		g_trk_var.g_trk_settings.gps_fance_cordinates[4*i+3][3] = "114.038103";
	}
}

void tracker_global_init()
{
	tracker_global_default_set();
	tracker_settings_record_file_init(NVDM_SETTING_ALL);
}

void tracker_agps2_gps_in_start(void)
{
	if(apgs2_gps_in == TRUE)
	{
		#ifdef SUPPORT_MY_WIFI
		tracker_read_wifi_and_send(WIFI_READ_FROM_NEED_SERVER_REPLY);
		#endif
	}
	else
	{
		maintask_printf("tracker_agps2_gps_in_start apgs2_gps_in = KAL_FALSE,return\r\n");
	}     
}		 

/**
搜到网络后开始工作
**/
void lte_network_status_callback_func(void)
{
	main_task_post_message(MAIN_MSG_POWERSTEP_EVENT, POWER_STEP_WELCOM, 0);
}

void tracker_start_work(void)
{
	maintask_printf("tracker_work_times=%d\r\n", tracker_work_teps);
	if(tracker_work_teps == TRACKER_SENDWFIF_START)
	{
		//3分钟之后发基站wifi信息，服务器回复经纬度和utc
		tracker_agps2_gps_in_start();
	}
	else if(tracker_work_teps == TRACKER_WORK_START_TIME)
	{
		main_task_post_message(MAIN_MSG_POWERSTEP_EVENT, POWER_STEP_TRACKIMO_WORK, 0);
	}
	tracker_work_teps++;
	if(tracker_work_teps < 3)  
		startMyTimer(TRACKER_WORK_STATUS_TIMER, TRACKIMO_WORKSTATUS_REPORT_TIME, tracker_start_work);
	else
		stopMyTimer(TRACKER_WORK_STATUS_TIMER);  
}

BOOL PWRUP_TRACKING_STARTED = FALSE;
void powerup_tracking_start(void)
{
	if(PWRUP_TRACKING_STARTED == FALSE)
	{
		if(g_trk_var.g_trk_settings.sample_time>0)
		{
			PWRUP_TRACKING_STARTED = TRUE;
			tracking_with_measurement_start_new();
		}
	}
}   

int check_battery_times=0;
void tracker_work_status_in_min(void)
{
	if(check_battery_times == 5)
	   {
		   check_battery_times=0;
		   check_battery_alarm_start(); 
	   }
	smart_moving_alarm_checking();
	check_battery_times++;
	powerup_tracking_start();  		
	startMyTimer(TRACKER_WORK_MIN_TIMER, 60*1000, tracker_work_status_in_min);//分钟计算
}  

void tracker_work_status(void)
{
      tracker_work_status_in_min();
     // tracker_work_status_in_hour();
}

void powerup_commands_start(void)
{
	maintask_printf("powerup_commands_start\r\n"); 
	tracker_work_status();
	//smart_moving_alarm_start();
} 

/**
开机网络上传工作
**/
void Tracker_network_start(void)
{
	maintask_printf(" Tracker_network_start=%d\r\n", tracker_network_teps);
	if(tracker_network_teps == 1)
	{
		tracker_protocol_send_deal(REPLY_HEADER_POWERUP, 0);
	}
	else if(tracker_network_teps == 2)
	{
		if(check_ack_setting_bit(ACK_BIT_Status_Request) == TRUE)
		{
		g_trk_var.g_trk_content_info.command=COMMAND_STATUS;        
		tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
		}
	}
	else if(tracker_network_teps == 3)
	{
		tracker_protocol_send_deal(REPLY_HEADER_STATUS_REPORT_1,g_trk_var.g_user_settings.data_transfer_channel);
	}
	else if(tracker_network_teps == 4)
	{
		tracker_protocol_send_deal(REPLY_HEADER_STATUS_REPORT_2,g_trk_var.g_user_settings.data_transfer_channel);
	}
	else if(tracker_network_teps == 5)
	{
		tracker_protocol_send_deal(REPLY_HEADER_STATUS_REPORT_3,g_trk_var.g_user_settings.data_transfer_channel);
	}
	else if(tracker_network_teps == 6)
	{
		tracker_protocol_send_deal(REPLY_HEADER_STATUS_REPORT_4,g_trk_var.g_user_settings.data_transfer_channel);
	}
	else if(tracker_network_teps == 7)
	{
		//tracker_protocol_send_deal(REPLY_DYNAMIC_STATUS_REPORT,g_trk_var.g_user_settings.data_transfer_channel);
	}
	else if(tracker_network_teps == 8)
	{
		//single_gsm_gps_location_report();
	}
	else if(tracker_network_teps == 11)
	{
		//powerup_commands_start();
	}
	tracker_network_teps++;
	if(tracker_network_teps < 12)  
	startMyTimer(TRACKER_NET_WORK_TIMER, 5*1000, Tracker_network_start);
	else
	stopMyTimer(TRACKER_NET_WORK_TIMER);  
}

/**
初始化函数
**/
void Tracker_init(void)
{
	maintask_printf("Tracker_init\r\n");
	tracker_global_init();
	module_message_buffer_init();
} 


