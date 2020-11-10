#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "MyCommon.h"
#include "TrackerCfg.h"
#include "My4G.h"
#include "MyGPS.h"
#include "TrackimoFrame.h"
#include "TrackerQueue.h"
#include "MyMemory.h"
#include "TrackerMain.h"
#include "TrackerEncodeOut.h"
#include "TrackerDecodeIn.h"
#include "MyTimer.h"
#include "MygSensor.h"
#include "MyFile.h"
#include "MyLog.h"
#define MOTION_COUNT_DEFAULT 5;
#define COMPRESSED_MSG_LEN 2048//注意，跟data那边一样
#define CHECK_SPEED_FENCE_TIMER_SEC 30
U8 history_current_time[6] = {0,0,0,0,0,0};
U8 get_history_time_from[6] = {0,0,0,0,0,0};
U8 get_history_time_to[6] = {0,0,0,0,0,0};
U8 speed_alarm_time_lock[5] = {0,0,0,0,0};
U8 fence_alarm_time_lock[5] = {0,0,0,0,0};
static U8 OLD[5]={0xff,0xff,0xff,0xff,0xff};
char compressed_message[COMPRESSED_MSG_LEN];
U8 button_id_bak = 0;
U8 button_resend_times = 0;
U8 timer_minutes = 0;
U16 fix_sending_times = 0;
U16 no_fix_sending = 0;
U16 xmode_send_times = 0;
U16 fence_speed_moving_fix = 0;
U16 fence_speed_moving_no_fix = 0;
int fence_speed_indoor_times = 0;
U8 send_zero_count = 0;
U8 send_zero_total = 0;
U8 no_fix_1_min_checking = 0;
//用来计算写到哪了
int current_byte = 0;
U8 current_bit = 0;
U8 first_pulse = 0;
static unsigned int tracker_with_measuremet_steps=0;
int xmode_send_with_measuremet_times = 0;
BOOL gps_xmode_fixed = FALSE;
BOOL gps_xmode_sent = FALSE;
BOOL GET_HISTORY_PROCESSING = FALSE;
BOOL key_long_pressed = FALSE;	
BOOL smart_accelerator = TRUE;//可开关accelerator
BOOL smart_gps = FALSE;//定到位可关gps，默认定到位关gps
BOOL smart_gsm = TRUE;//可开gps模块，关gsm
BOOL default_glp_on = TRUE; //开gps就直接开glp
BOOL tracking_high_speed_turnoff_gps = TRUE;//超过30km/h关gps
BOOL GPS_FENCE_SPEED_FIXED = FALSE;
BOOL fence_speed_gps_special_indoor = FALSE;
BOOL gps_moving_in_last_min = FALSE;//前一分钟动过
BOOL gps_driving_mode = FALSE;
//大于1分钟的tracking
BOOL tracking_with_measurement_fixed = FALSE;
BOOL tracking_with_measurement_sent = FALSE;
BOOL movie_alarm_enable = FALSE;
BOOL moving_alarm_moving = FALSE;
BOOL pwrup_get_gps = FALSE;
BOOL gsm_service_stable=TRUE;
reply_channel get_history_channel = 0xff;
extern U8 compression_buffer[100];
extern trackergps_backup_data_struct gps_backup[GPS_BACKUP_MAX_NUMBER];//保存500个坐标,共40*500 = 20000byte,20k byte
extern trackergps_backup_data_struct gps_backup_fixed_history_buffer[GPS_BACKUP_MAX_NUMBER];//根据时间的fix坐标，最多一样500个，20k byte
typedef struct{
int id;
pfTrackimoReceivsFunction fun;
}structReceiveMapFun;

typedef struct{
int id;
pfTrackimoSendFunction fun;
}structSendMapFun;

//static pfDefaultFunction static_arrayReceive[COMMAND_MAX];
static pfTrackimoSendFunction static_arraySend[REPLY_MAX_ID];
static const structReceiveMapFun static_receives[]={
	{COMMAND_ACK, server_msg_analyze_ack},
	{COMMAND_TRACKING_MODE_START, server_msg_analyze_tracking_mode_start},  
	{COMMAND_TRACKING_MODE_STOP, server_msg_analyze_tracking_mode_stop},
	{COMMAND_GENERAL_SETUP, server_msg_analyze_general_setup},
	{COMMAND_TESTMODE, server_msg_analyze_testmode},
	{COMMAND_STATUS, server_msg_analyze_status},

	{COMMAND_DELETE_FILE, server_msg_analyze_delete_file},
	{COMMAND_BEEP, server_msg_analyze_beep},
	{COMMAND_REBOOT, server_msg_analyze_reboot},
	{COMMAND_BACKUP_ENABLE, server_msg_analyze_backup_enable},
	{COMMAND_GPS_SEARCHMODE, server_msg_analyze_gps_search_mode},
	{COMMAND_GPS_FIX_WHEN_MOVING, NULL},
	{COMMAND_CLEAR_ALL_ALARMS, server_msg_analyze_clear_all_alarm},

	{COMMAND_GET_HISTORY, server_msg_analyze_get_history},
	{COMMAND_VOICE_CALL, server_msg_analyze_voice_call},
	{COMMAND_GET_CARRIERS, server_msg_analyze_get_carriers},
	{COMMAND_GET_GPS, server_msg_analyze_get_gps},

	{COMMAND_FENCE_ALARM, server_msg_analyze_fence_alarm},
	{COMMAND_FENCE_ALARM_ONOFF, server_msg_analyze_fence_alarm_onoff},
	{COMMAND_SPEED_ALARM, server_msg_analyze_speed_alarm},
	{COMMAND_MOVING_ALARM, server_msg_analyze_moving_alarm},
	{COMMAND_BATTERY_ALARM, server_msg_analyze_battery_alarm},
	{COMMAND_SET_APN, server_msg_analyze_set_apn},
	{COMMAND_SET_ACTIVE_CARRIER, server_msg_analyze_set_active_carrier},
	{COMMAND_SESSION_MAX_BYTES, server_msg_analyze_set_session_max_bytes},
	{COMMAND_OTA_START, server_msg_analyze_ota_start},
	{COMMAND_ACK_SETTING, server_msg_analyze_ack_setting},  
	{COMMAND_GET_SINGLE_MEASUREMENT, NULL},
	{COMMAND_START_TRACKING_MODE_WITH_MEASUREMENT, server_msg_analyze_start_tracking_mode_with_measurement},
	{COMMAND_SCHEDULE_SLEEP, server_msg_analyze_schedule_sleep},
	{COMMAND_SCHEDULE_SLEEP_OFF, server_msg_analyze_schedule_sleep_off},
	{COMMAND_GENERAL_SETUP_WITH_NET_IDENTIFIERS, server_msg_analyze_general_setup_with_net_identifiers},  
	//{COMMAND_TEST_COMPONENT, server_msg_analyze_test_component},
	{COMMAND_AGPS_BOOTSTRAP_LOCATION, server_msg_analyze_agps_bootstrap_location},
	{COMMAND_SET_COMPONENTS_DYNAMIC, server_msg_analyze_set_components_dynamic},  

	//{COMMAND_SET_SMART_WIFI, server_msg_analyze_set_smart_wifi}, 
	//{COMMAND_SET_SMART_WIFI_TABLE, server_msg_analyze_set_smart_wifi_table},
	{COMMAND_RESET_SESSION, server_msg_analyze_reset_session},
	{COMMAND_MY_COMMAND_USSD_Q_TIMER, server_msg_analyze_ussd_q_timer},
	{COMMAND_MY_COMMAND_MOTION_COUNT, server_msg_analyze_motion_count},
	{COMMAND_AUTO_CHANGE_CARRIER, server_msg_analyze_auto_change_carrier},  
};
static const structSendMapFun static_sends[]={
	{REPLY_HEADER_SUCCESS, tracker_reply_server_msg_success},
	{REPLY_HEADER_STATUS_REPORT_1,  tracker_server_msg_analyze_status_1},
	{REPLY_HEADER_ALARM_TRIGGERED,tracker_alm_msg_send},
	{REPLY_HEADER_REPORT_IMSI,tracker_server_msg_analyze_get_imsi},
	{REPLY_HEADER_REPORT_CARRIERS,tracker_sent_carrier},
	{REPLY_HEADER_REPORT_CARRIERS_ERROR,tracker_sent_carrier_error}, 
	{REPLY_HEADER_GET_GPS,tracker_send_gps_info_to_server},
	{REPLY_GSM_LOCATION_REPORT,tracker_send_first_tower_info},

	{REPLY_HEADER_STATUS_REPORT_2, tracker_server_msg_analyze_status_2},
	{REPLY_HEADER_POWERUP, tracker_protocol_encode_powerup},
	{REPLY_HEADER_HISTORYREPORT,tracker_send_history_data},
	{REPLY_HEADER_CALLERID,tracker_caller_id_notify},
	{REPLY_HEADER_SESSION_START,tracker_data_session_start_msg},
	{REPLY_HEADER_SESSION_STOP,tracker_data_session_stop_msg},
	{REPLY_HEADER_PING,tracker_heart_pulse_send},
	{REPLY_MULTI_GSM_LOCATION_REPORT_1,trakcer_send_multi_tower_info},
	{REPLY_HEADER_SHUTDOWN,tracker_shutdown_message_send},
	{REPLY_BT_MAC_REPORT,tracker_server_msg_analyze_get_bt_mac},

	{REPLY_HEADER_STATUS_REPORT_3,tracker_server_msg_analyze_status_3},
	{REPLY_BUTTON_WITH_IDENTIFIERS_GPS,tracker_send_button_without_wifi},

	{REPLY_HEADER_STATUS_REPORT_4,tracker_server_msg_analyze_status_4},
	{REPLY_ACK_SEQUENTIAL,tracker_reply_server_sequence_success},
	{REPLY_UNIVERSAL_TEST,tracker_server_msg_analyze_test_component},
	{REPLY_AGPS_BOOTSTRAP_REQUEST,tracker_send_wifi_info_to_server},  
	{REPLY_DYNAMIC_STATUS_REPORT,tracker_server_msg_analyze_status_dynamic},
	{REPLY_BUTTON_WITH_WIFI_LOCATION,tracker_send_wifi_info_to_server},
	{REPLY_GET_ACK,tracker_device_send_get_ack},
	{REPLY_HEADER_ERROR,tracker_reply_server_msg_error},	
};

void tracker_dynamic_analyze(U8 id1,U8 id2,U8 dynamic_item)    
{
	U16 dynamic_id = id1*256+id2;
	maintask_printf("tracker_dynamic_analyze:%d --- %d",dynamic_id,dynamic_item);
	if(dynamic_id == DYNAMIC_ID_AGPS1_EPO)
	{
		g_trk_var.g_user_settings.agps1_enable = dynamic_item;
		agps_setting_init();
	}
	else if(dynamic_id == DYNAMIC_ID_AGPS2_GPS_IN)		
	{
		g_trk_var.g_user_settings.agps2_enable= dynamic_item;
		agps_setting_init();
	}	
	else if(dynamic_id == DYNAMIC_ID_AGPS1_EPO_THIS_POWERUP)	
	{
		;//do nothing
	}
	#ifdef __TRACKIMO_SMART_WIFI__
  	else if(dynamic_id == DYNAMIC_ID_SMART_WIFI_ONOFF)		
	{
		g_trk_var.g_user_settings.smart_wifi_enable[0]= dynamic_item;
	}
	#endif
	#ifdef __SIM_DEFAULT_CHANNEL_DATA__
 	 else if(dynamic_id == DYNAMIC_ID_SERVER_CONTROL_DEVICEID)		
	{
		g_trk_var.g_user_settings.service_control_deviceid_enable = dynamic_item;
	}
	else if(dynamic_id == DYNAMIC_ID_DATA_ACK_MINUTES)		
	{
		g_trk_var.g_user_settings.check_ack_timer_in_min= dynamic_item;
	}
	else if(dynamic_id == DYNAMIC_ID_DEVICE_ACK_DISABLE)		
	{
		g_trk_var.g_user_settings.check_ack_timer_enable= dynamic_item;
	}
	#endif
}
		
void schedule_sleep_do_power_off(void)
{
	U16 SleepPeriodMinutes = g_trk_var.g_trk_settings.gps_schedule_sleep[3]*256 + g_trk_var.g_trk_settings.gps_schedule_sleep[4]; 
	if(g_trk_var.g_trk_settings.gps_schedule_sleep[0] == 0)
	{
		maintask_printf("schedule_sleep_do_power_off stop");
		stopMyTimer(MY_TIMER_54);
	return;
	}
	if(SleepPeriodMinutes > 24*60)
	{
		maintask_printf("SleepPeriodMinutes too big");
		return;
	}	
	//充电的时候关机也没用(is_charging() == KAL_TRUE)
	if(0)
	{
		maintask_printf("is_charging");
		return;
	}		
	maintask_printf("schedule_sleep_do_power_off: after %d min, power up",SleepPeriodMinutes);	
	g_trk_var.g_trk_status.shutdown_reason=3;
	g_trk_var.g_trk_status.wake_total_min=SleepPeriodMinutes;
	tracker_protocol_send_deal(REPLY_HEADER_SHUTDOWN,g_trk_var.g_user_settings.data_transfer_channel);
	//tracker_write_power_up_sig(3);	
	//set_auto_pwron_time(SleepPeriodMinutes);
	//do_power_off();
}  


void get_gps_check_schedule_sleep(void)
{
	if(pwrup_get_gps == TRUE)
	{
	pwrup_get_gps = FALSE;
	maintask_printf("get_gps_check_schedule_sleep GO");
	if(g_trk_var.g_trk_settings.gps_schedule_sleep[7] == 0)
	{
		maintask_printf("get_gps_check_schedule_sleep -- send GPS and sleep -- off");
		return;
	}	   	  		

	if(g_trk_var.g_trk_settings.gps_schedule_sleep[0] == 0)
	{
		maintask_printf("get_gps_check_schedule_sleep off");
		return;
	}

	if(g_trk_var.g_trk_settings.gps_schedule_sleep[8] == 0)
	{
		maintask_printf("get_gps_check_schedule_sleep repeat off");
		return;
	}
	stopMyTimer(MY_TIMER_55);
	startMyTimer(MY_TIMER_55, 120*1000, schedule_sleep_do_power_off);
	}    
} 

void schedule_sleep_start(void)
{
	 maintask_printf("schedule_sleep_start %x,%x,%x,%x,%x,%x,%x,%x,%x\n",
        g_trk_var.g_trk_settings.gps_schedule_sleep[0],g_trk_var.g_trk_settings.gps_schedule_sleep[1],g_trk_var.g_trk_settings.gps_schedule_sleep[2],
        g_trk_var.g_trk_settings.gps_schedule_sleep[3],g_trk_var.g_trk_settings.gps_schedule_sleep[4],g_trk_var.g_trk_settings.gps_schedule_sleep[5],
        g_trk_var.g_trk_settings.gps_schedule_sleep[6],g_trk_var.g_trk_settings.gps_schedule_sleep[7],g_trk_var.g_trk_settings.gps_schedule_sleep[8]);
	 if(g_trk_var.g_trk_settings.gps_schedule_sleep[0]== 0)
	 {
		  maintask_printf("schedule_sleep_start stop");
		  stopMyTimer(MY_TIMER_54);
	 }
	 else
	 {
	 	  U16 StartDiffMinutes = g_trk_var.g_trk_settings.gps_schedule_sleep[1]*256 + g_trk_var.g_trk_settings.gps_schedule_sleep[2];
	 	  maintask_printf("after %d min, poweroff",StartDiffMinutes);
	 	  if(StartDiffMinutes == 0)
	 	  {
		   	schedule_sleep_do_power_off();
	 	  }
	 	  else
	 	  {	 		
	 	      stopMyTimer(MY_TIMER_54);
                    startMyTimer(MY_TIMER_54, StartDiffMinutes*60*1000, schedule_sleep_do_power_off);
                }    
        }  
}

void schedule_sleep_stop(void)
{
	stopMyTimer(MY_TIMER_54);
}	

void GET_GPS_command_finish(void)
{
	 stopMyTimer(TRACKER_GPS_FIX_COMMAND_WAIT);
	 stopMyTimer(TRACKER_GPS_START_COMMAND);
}
/*********************************************
Latitude (5 bytes)
Longitude (5 bytes)
Speed (1 byte)
Number Of Satelites (1 byte)
Signal Strength (1 byte)
Is Fix (1 byte)
Battery Percent Status (1 byte)
************************************************/
void get_gps_command_fix_wait(void)
{
	U8 bat = 0;
	U8 length=0;
	//如果现在有gps信号，现在发
	if(TRUE == gps_2d_3d_fix_with_hdop_and_fix_sec(10))
	{	
		tracker_protocol_send_deal(REPLY_HEADER_GET_GPS,g_trk_var.g_user_settings.data_transfer_channel);
		//get_gps_check_schedule_sleep();
		GET_GPS_command_finish();
		GET_GPS_COMMAND_TURN_OFF_GPS();	    
	}
	else //没fix,等
	{
		maintask_printf("get_gps_command_fix_wait NO FIX!\r\n");
		//10S看一次
		stopMyTimer(TRACKER_GPS_FIX_COMMAND_WAIT);
		startMyTimer(TRACKER_GPS_FIX_COMMAND_WAIT, 5000, get_gps_command_fix_wait);
	}
}

void BACKEND_GPS_POWEROFF_SPECIAL(void)
{
 	 //get_gps_check_schedule_sleep();
 	 GET_GPS_COMMAND_TURN_OFF_GPS();
 	 GET_GPS_command_finish();
}

void get_gps_command_1min_check(void)
{
	BOOL in_door = FALSE;
	U8 max_snr = g_trk_var.g_trk_content_info.gps_info.max_snr;
	maintask_printf("get_gps_command_1min_check GPSDW_sate_number= %d, g_trk_var.g_trk_content_info.gps_info.max_snr=%d\n",g_trk_var.g_trk_content_info.gps_info.satellite_visable_num,max_snr);	
	//如果室内在动，信号很差，开1分钟关2分钟
	if(Is_gps_searching() == TRUE && g_trk_var.g_trk_content_info.gps_info.satellite_visable_num == 0 && max_snr == 0) 
	{
		maintask_printf("get_gps_command_1min_check indoor! send 0 directly");
		BACKEND_GPS_POWEROFF_SPECIAL();
	}	   	   
} 
		
void BACKEND_GPS_POWERON_SPECIAL(void)
{
	GET_GPS_COMMAND_TURN_ON_GPS();
	maintask_printf("BACKEND_GPS_POWERON_SPECIAL\r\n");
	//10S看一次
    get_gps_command_fix_wait(); 
	//5分钟后强制关
	stopMyTimer(TRACKER_GPS_START_COMMAND);
	startMyTimer(TRACKER_GPS_START_COMMAND, 5*60*1000, BACKEND_GPS_POWEROFF_SPECIAL);
}	

//不判断gps，必须发, first 跟 multi1是同一个
void get_gps_send_gsm_1(void)
{
	tracker_protocol_send_deal(REPLY_MULTI_GSM_LOCATION_REPORT_1,g_trk_var.g_user_settings.data_transfer_channel);
}

//震动报警
void moving_alm_msg_send(void)
{
	//send
	reply_channel rr_channel;
	rr_channel = g_trk_var.g_trk_settings.gps_moving_alarm_setting[3];
	if(rr_channel == 0xff)
	rr_channel = g_trk_var.g_user_settings.data_transfer_channel;
	g_trk_var.g_trk_status.alarm_type=g_trk_var.g_trk_settings.gps_moving_alarm_setting[2];
	tracker_protocol_send_deal(REPLY_HEADER_ALARM_TRIGGERED,rr_channel);
}

void stop_moving_alm_msg_send(void)
{
	char reply[100];
	U8 length=0;
	reply_channel rr_channel=0xff;
	//onoff
	if(g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[4] == 0)
	{
	maintask_printf("sstop_moving_alarm_check off\r\n");
	return;
	} 

	//如果gps在fix状态而且speed不为0，不能发stopmoving报警
	if((TRUE == gps_2d_3d_fix_with_hdop_and_fix_sec(10))&& g_trk_var.g_trk_content_info.gps_info.speed_km > 5)
	{
	maintask_printf("stop_moving_alarm_check, speed ok , return\r\n");
	return;
	}  
	//send
	rr_channel = g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[3];
	if(rr_channel == 0xff)
	rr_channel = g_trk_var.g_user_settings.data_transfer_channel;
	g_trk_var.g_trk_status.alarm_type=g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[2];
	tracker_protocol_send_deal(REPLY_HEADER_ALARM_TRIGGERED,rr_channel);
}	 


void smart_moving_alarm_checking(void)//每分钟测一次
{
	//震动报警
	if(g_trk_var.g_trk_settings.gps_moving_alarm_setting[4]== 1)
	{	
		if(getViburatorWorkmode())
		{
		moving_alm_msg_send();
		}	
	}

	//静止报警
	if(g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[4]==1)
	{	
		if(!getViburatorWorkmode())
		{
		stop_moving_alm_msg_send();
		}	
	}	
	setViburatorWorkmode(FALSE);
	//moving_alarm_turnon_gsensor();
	//CheckTurnOnOffGsensor();
}  


void smart_moving_alarm_start(void)
{
	//off
	if(g_trk_var.g_trk_settings.gps_moving_alarm_setting[4]==0&&g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[4]==0)
	{
		maintask_printf("smart_moving_alarm_start off");
		#ifdef SUPPORT_MY_GSENSOR
		moving_alarm_turnoff_gsensor();
		CheckTurnOnOffGsensor();
		#endif
		movie_alarm_enable=FALSE;
	}
	else//on
	{
		#ifdef SUPPORT_MY_GSENSOR
		maintask_printf("smart_moving_alarm_start on");
		moving_alarm_turnon_gsensor();
		CheckTurnOnOffGsensor();
		#endif
		movie_alarm_enable=TRUE;
		moving_alarm_moving = FALSE;
	
	}  
}


void do_heart_pulse_fail_reboot(void)
{
	maintask_printf("do_heart_pulse_fail_reboot %d min",g_trk_var.g_user_settings.no_heart_pulse_reboot_in_min);
	if(g_trk_var.g_user_settings.no_heart_pulse_reboot_in_min== 0)//0min，直接reset
	{ 
		g_trk_var.g_trk_status.shutdown_reason=4;
		g_trk_var.g_trk_status.wake_total_min=0;
		tracker_protocol_send_deal(REPLY_HEADER_SHUTDOWN,g_trk_var.g_user_settings.data_transfer_channel);
		//ResetOrderDo();
	}
	else
	{  	
		if(g_trk_var.g_user_settings.no_heart_pulse_reboot_in_min== 1)//至少2分钟
		{
		          g_trk_var.g_trk_status.shutdown_reason=4;
		          g_trk_var.g_trk_status.wake_total_min=2;
		          tracker_protocol_send_deal(REPLY_HEADER_SHUTDOWN,g_trk_var.g_user_settings.data_transfer_channel);
			// set_auto_pwron_time(2);
		}
		else
		{	
		         g_trk_var.g_trk_status.shutdown_reason=4;
		         g_trk_var.g_trk_status.wake_total_min=g_trk_var.g_user_settings.no_heart_pulse_reboot_in_min;
		         tracker_protocol_send_deal(REPLY_HEADER_SHUTDOWN,g_trk_var.g_user_settings.data_transfer_channel);
		         //set_auto_pwron_time(g_trk_var.g_user_settings.no_heart_pulse_reboot_in_min);
		     }    
		//do_power_off();
  }  	
}	


void set_pulse_timer_in_min(U8 pulse_min)
{
	maintask_printf("set_pulse_timer_in_min %d min",pulse_min);
	if(pulse_min > 0)
	{
		first_pulse = 1;
		stopMyTimer(MY_TIMER_84);
		startMyTimer(MY_TIMER_84, pulse_min*60*1000, heart_pulse_do);
  	}
}

void heart_pulse_do(void)
{
	//onoff
	if(g_trk_var.g_user_settings.tracking_heart_pulse== 0)
	{
		maintask_printf("heart_pulse_do nothing return\n");	
		return;
	}
		
  	if(first_pulse == 0)
	{
		maintask_printf("first_pulse == 0\n");	
	  	tracker_protocol_send_deal(REPLY_HEADER_PING,g_trk_var.g_user_settings.data_transfer_channel);
		//10min 内必须有回复，否则重启
		set_pulse_timer_in_min(10);
	}
  	else//10min内没有回复，关机开机
	{
		maintask_printf("first_pulse != 0\n");	
		//tracker_write_power_up_sig(10);
		do_heart_pulse_fail_reboot();
	}
}  	 	

	
void set_pulse_timer(U8 pulse_hour)
{
	maintask_printf("set_pulse_timer %d hour",pulse_hour);
	if(pulse_hour > 0)
	{
		 if(pulse_hour == 99)//测试模式,5分钟
		 {
        		stopMyTimer(MY_TIMER_84);
        		startMyTimer(MY_TIMER_84, 300*1000, heart_pulse_do);	
     		}
     		else
		{	   	 	  	
			stopMyTimer( MY_TIMER_84);
			startMyTimer(MY_TIMER_84, pulse_hour*60*60*1000, heart_pulse_do);
		}   
  	}
  	else
	{
	 	stopMyTimer(MY_TIMER_84);
	}
}  	 
			
void heart_pulse_start(void)
{
    if(g_trk_var.g_user_settings.tracking_heart_pulse==0)
        return;
	 first_pulse = 0;
	 set_pulse_timer(g_trk_var.g_user_settings.tracking_heart_pulse);
}	

U8 get_button_wait_sec(void)
{
	return g_trk_var.g_user_settings.press_ack_time_out_in_sec;
}

void resend_button_stop(void)		
{
	button_resend_times = 0;	
	stopMyTimer(MY_TIMER_88);
}

void resend_button(void)
{
	button_resend_times++;	
	if(button_resend_times < 3)
	{	
	  key_longpress_event_send_to_server(button_id_bak);
	}
	else //3次了，不发了，清0
	{
		button_resend_times = 0;
	}		  
} 

void send_button_with_wif_fail(void)
{
       g_trk_var.g_trk_device_status.botton_id=button_id_bak;
       tracker_protocol_send_deal(REPLY_BUTTON_WITH_IDENTIFIERS_GPS,g_trk_var.g_user_settings.data_transfer_channel);  
}

void key_long_press_sig_over(void)
{
	key_long_pressed = FALSE;
}

void key_longpress_event_send_to_server(U8 botton_id)
{
	U8 ack_wait_sec = 0;		
	if(TRUE == gps_2d_3d_fix_with_hdop_and_fix_sec(10))
	{	
		g_trk_var.g_trk_device_status.botton_id=botton_id;
		tracker_protocol_send_deal(REPLY_BUTTON_WITH_IDENTIFIERS_GPS,g_trk_var.g_user_settings.data_transfer_channel);  
	}
    else
	{
		//button_id_bak = botton_id;
		#ifdef SUPPORT_MY_WIFI
		tracker_read_wifi_and_send(WIFI_READ_FROM_BOTTON); 
		#endif
		//防止读wifi失败导致什么都没发
		stopMyTimer(TRACKER_WIFI_TIMER_3); 
		startMyTimer(TRACKER_WIFI_TIMER_3, 5000, send_button_with_wif_fail);
	}    
}

U8 get_uint16_byte_high(U16 uint16_number)    
{
	U8 ret = uint16_number/256;
	return ret;
}

U8 get_uint16_byte_low(U16 uint16_number)    
{
	U8 i;
	U8 ret;
	i = uint16_number/256;
	ret = uint16_number - i*256;
	return ret;
}

//no operation - 0/fail - 1/success - 2/going - 3
U8 get_epo_power_up_status(void)
{
	return AGPS1_EPO_THIS_POWERUP_NO_OPERATION;
}


void xmode_stop_all(void)			 	
{
	gps_xmode_fixed = FALSE; 
	gps_xmode_sent =  FALSE;
       stopMyTimer(MY_TIMER_31);//xmode_start_gps_timer_check_in_minutes的timer
	stopMyTimer(MY_TIMER_33);//gps_fix_wait的timer	
	stopMyTimer(MY_TIMER_34);//gps_always_on_keep_sending的timer
	stopMyTimer(MINI_MY_TIMER_10);//隔2分钟再开的timer
	stopMyTimer(MINI_MY_TIMER_14);//1分钟后的timer
	stopMyTimer(MINI_MY_TIMER_15);//定到位多久关gps的timer
	GPS_POWEROFF();//关gps
}	

U8 get_speed_for_turnoff_gps_in_tracking(void)
{
	U8 speed = g_trk_var.g_trk_settings.gps_search_control[7];
	if(speed == 0)
		speed = 30;	
	maintask_printf("get_speed_for_turnoff_gps_in_tracking %d\r\n",speed);		
	return speed;
}		

//向前跳n个bit
void jump_bit(int jump) 
{
	int total;
	
	total = current_bit + jump;
	
	current_byte = current_byte + total/8;
	
	current_bit = total - (total/8)*8;
}	

void set_bit_of_byte(char* buffer,int byte, U8 bit, U8 or_0_1)
{
	if(bit == 0)
	{
		if(or_0_1 == 1)
   		buffer[byte] = buffer[byte] | 0x80;//1000 0000
   	else
   		buffer[byte] = buffer[byte] & 0x7f;//0111 1111		
   	}
   	else if(bit == 1)
	{
	   	 if(or_0_1 == 1)
	   		buffer[byte] = buffer[byte] | 0x40;//0100 0000
	   	 else
	   		buffer[byte] = buffer[byte] & 0xbf;//1011 1111
	}			
	else if(bit == 2)
	{
		 if(or_0_1 == 1)
			buffer[byte] = buffer[byte] | 0x20;//0010 0000
		 else
			buffer[byte] = buffer[byte] & 0xdf;//1101 1111
	}	
	else if(bit == 3)
	{
		 if(or_0_1 == 1)
			buffer[byte] = buffer[byte] | 0x10;//0001 0000
		 else
			buffer[byte] = buffer[byte] & 0xef;//1110 1111
	}
	else if(bit == 4)
	{
		 if(or_0_1 == 1)
			buffer[byte] = buffer[byte] | 0x8;//0000 1000
		 else
			buffer[byte] = buffer[byte] & 0xf7;//1111 0111
	}
	else if(bit == 5)
	{
		 if(or_0_1 == 1)
			buffer[byte] = buffer[byte] | 0x4;//0000 0100
		 else
			buffer[byte] = buffer[byte] & 0xfb;//1111 1011
	}
	else if(bit == 6)
	{
		 if(or_0_1 == 1)
			buffer[byte] = buffer[byte] | 0x2;//0000 0010
		 else
			buffer[byte] = buffer[byte] & 0xfd;//1111 1101
	}
	else if(bit == 7)
	{
		 if(or_0_1 == 1)
			buffer[byte] = buffer[byte] | 0x1;//0000 0001
		 else
			buffer[byte] = buffer[byte] & 0xfe;//1111 1110
	}
} 

//给device id用
void bit_move_right_of_4byte(char* buffer, U8 move)
{
	 U8 temp = 0;
	 U8 temp_1 = 0;
	 char i=0; //必须是有符号
	 U8 j=0;
	 for(i=3;i>=0;i--)//一排一排移,移6排
	 {
	   temp = (U8)buffer[i];
	   for(j=0;j<move;j++)
	   {
	      if(j==0) //最后一位
	      {
	      	temp_1 = temp & 0x1;
	      }
	      else
	      {
	      	temp_1 = (temp >> j) & 0x1;
	      }				
	      set_bit_of_byte(buffer,(i+1),(move-j-1),temp_1);
	   }
	   
	   //然后本派平移move个bit
	   buffer[i] = temp >> move;
	 }
}	 

//右移n个bit，n小于8
void bit_move_right_of_7byte(char* buffer, U8 move)
{
	 U8 temp = 0;
	 U8 temp_1 = 0;
	 char i=0; //必须是有符号
	 U8 j=0;
	 
	 for(i=6;i>=0;i--)//一排一排移,移6排
	 {
	   temp = (U8)buffer[i];
	   for(j=0;j<move;j++)
	   {
	      if(j==0) //最后一位
	      {
	      	temp_1 = temp & 0x1;
	      }
	      else
	      {
	      	temp_1 = (temp >> j) & 0x1;
	      }				
	      set_bit_of_byte(buffer,(i+1),(move-j-1),temp_1);
	   } 
	   //然后本派平移move个bit
	   buffer[i] = temp >> move;
	 }
}

//右移n个bit，n小于8
void bit_move_right_of_9byte(char* buffer, U8 move)
{
	 U8 temp = 0;
	 U8 temp_1 = 0;
	 char i=0; //必须是有符号
	 U8 j=0;
	 for(i=8;i>=0;i--)//一排一排移,移9排
	 {
	   temp = (U8)buffer[i];
	   for(j=0;j<move;j++)
	   {
	      if(j==0) //最后一位
	      {
	      	temp_1 = temp & 0x1;
	      }
	      else
	      {
	      	temp_1 = (temp >> j) & 0x1;
	      }				
	      set_bit_of_byte(buffer,(i+1),(move-j-1),temp_1);
	   }
	   
	   //然后本派平移move个bit
	   buffer[i] = temp >> move;
	 }
}

void bit_move_right_of_n_byte(char* buffer, U8 move,U8 n_bytes_for_move)
{
	 U8 temp = 0;
	 U8 temp_1 = 0;
	 char i=0; //必须是有符号
	 U8 j=0;
	 for(i=(n_bytes_for_move-1);i>=0;i--)//一排一排移,移9排
	 {
		temp = (U8)buffer[i];
		for(j=0;j<move;j++)
		{
			temp_1 = (temp >> j) & 0x1;	
			set_bit_of_byte(buffer,(i+1),(move-j-1),temp_1);
		}
		//然后本派平移move个bit
		buffer[i] = temp >> move;
	 }
}	

//压缩完，"latlongspeedlatlongspeed....#",最多只能压缩254,put_deviceid为是否加device id
void creat_compressed_message(U8 compressed_number, char compressed_type,U8 put_deviceid)
{
	U16 i=0;
	U16 j=0;
	U16 actual_n=0;
	U16 num_1 = 0;
	U16 len = 0;
	U8 bat = 0;
	U8 speed = 0;
	char compressed_lat[4];
	char compressed_long[4];
	char fixed_compression_2_buffer[8]; //总共7位，多留一位串位用
	char fixed_compression_3_buffer[10]; //总共9位，多留一位串位用
	char* temp = NULL;
	char log[200];
	if(compressed_number == 0)
		return;	
	memset(compressed_lat,0,4);
       memset(compressed_long,0,4);		
       memset(fixed_compression_2_buffer,0,8);
       memset(fixed_compression_3_buffer,0,10);				
  	actual_n = compressed_number;			
  	num_1 = 3;//get_number_of_records_in_backup_database();	  
	if(actual_n > num_1)
		   actual_n = num_1;
	if(actual_n == 0)
		return;	
	if(compressed_type == 0)//3
	  {	
	   	memset(compressed_message,0,COMPRESSED_MSG_LEN);//2048	
	   	maintask_printf("GO compression_2\r\n");
	   	current_byte = 0;	
		current_bit = 0;
		print_log_for_gps_backup();
	   	//第一个bit是1，表示tracking message
	    	//先写后跳
	    	set_bit_of_byte(compressed_message,current_byte,current_bit,1);	
		jump_bit(1);
		if(put_deviceid == 1)
		{
			 U8 buffer_1[5];
			 memset(buffer_1,0,5);
		     if(g_trk_var.g_user_settings.data_transfer_channel==REPLY_CHANNEL_DATA_MODULE) 
		          {     
		               buffer_1[0] = g_trk_var.g_trk_device_status.deviceid_4G[0];
		               buffer_1[1] = g_trk_var.g_trk_device_status.deviceid_4G[1];
		               buffer_1[2] = g_trk_var.g_trk_device_status.deviceid_4G[2];
		               buffer_1[3] = g_trk_var.g_trk_device_status.deviceid_4G[3];
		           }
		            else
		            {
		                buffer_1[0] = g_trk_var.g_trk_device_status.deviceid_2G[0];
		                buffer_1[1] = g_trk_var.g_trk_device_status.deviceid_2G[1];
		                buffer_1[2] = g_trk_var.g_trk_device_status.deviceid_2G[2];
		                buffer_1[3] = g_trk_var.g_trk_device_status.deviceid_2G[3];
		            }
		 
			memset(log,0,200);
			sprintf(log,"add device id: %x,%x,%x,%x",buffer_1[0],buffer_1[1],buffer_1[2],buffer_1[3]);	 	  	 
			bit_move_right_of_4byte(buffer_1,current_bit);	 
			 compressed_message[0] = compressed_message[0] | buffer_1[0];
			 compressed_message[1] = buffer_1[1];
			 compressed_message[2] = buffer_1[2];
			 compressed_message[3] = buffer_1[3];
			 compressed_message[4] = buffer_1[4]; 
			 jump_bit(32);//跳4个byte 
		}	  	  		 
	   	for(i=0;i<actual_n;i++)
	   	{
	   		if(gps_backup[i].type == GPS_NO_FIX)
	   		{
	   			 //先写后跳
	   			 set_bit_of_byte(compressed_message,current_byte,current_bit,0);
	   			 jump_bit(1);			 
	   			 maintask_printf("GPS_NO_FIX: i=%d-- current_byte:%d,current_bit:%d",i,current_byte,current_bit);
	   		}
	   		else if(gps_backup[i].type == GPS_FIXED || gps_backup[i].type == GPS_BACKUP)
	   		{	
	   	     		compression_method_2_lat(gps_backup[i].latitude, compressed_lat, 4);
		       		compression_method_2_long(gps_backup[i].longitude, compressed_long, 4);
	   		
	   		   	fixed_compression_2_buffer[0] = compressed_lat[0] | 0x80; //前面第一个bit必须是1，表示有效坐标
	   		   	fixed_compression_2_buffer[1] = compressed_lat[1];
	   		   	fixed_compression_2_buffer[2] = compressed_lat[2];
	   		   	fixed_compression_2_buffer[3] = compressed_long[0];
	   		  	fixed_compression_2_buffer[4] = compressed_long[1];
	   		   	fixed_compression_2_buffer[5] = compressed_long[2];	
	  			if(gps_backup[i].speed > 255)
	  				gps_backup[i].speed = 255;		
	   	     	   	fixed_compression_2_buffer[6] = gps_backup[i].speed; 		
	   		   	fixed_compression_2_buffer[7] = 0; 	   
	   		   	//memset(log,0,200);
	   		   	//sprintf(log,"GPS_FIX 1: i=%d-- byte:%d,bit:%d,fixed_compression_2_buffer:%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,fixed_compression_2_buffer[0],fixed_compression_2_buffer[1],fixed_compression_2_buffer[2],fixed_compression_2_buffer[3],fixed_compression_2_buffer[4],fixed_compression_2_buffer[5],fixed_compression_2_buffer[6],fixed_compression_2_buffer[7]);
	                	//printf("%s",log);		   
	   		   	if(current_bit != 0)//整数8，什么都不做
		   		   {	
		   		   	  //必须move current_bit才能跟以前对的上
		   		   	  bit_move_right_of_7byte(fixed_compression_2_buffer,current_bit);
		   		   	 // memset(log,0,200);
		   		       //  sprintf(log,"GPS_FIX not 8: i=%d-- byte:%d,bit:%d,fixed_compression_2_buffer:%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,fixed_compression_2_buffer[0],fixed_compression_2_buffer[1],fixed_compression_2_buffer[2],fixed_compression_2_buffer[3],fixed_compression_2_buffer[4],fixed_compression_2_buffer[5],fixed_compression_2_buffer[6],fixed_compression_2_buffer[7]);
		            		 //  printf("%s",log);
		   		   	  //第一个字节，直接或，没问题,其他所有没填的bit都是0
		   		   	  compressed_message[current_byte] = compressed_message[current_byte] | fixed_compression_2_buffer[0];
		   		   	  //剩下直接copy
		   		   	  compressed_message[current_byte+1] = fixed_compression_2_buffer[1];
		   		   	  compressed_message[current_byte+2] = fixed_compression_2_buffer[2];
		   		   	  compressed_message[current_byte+3] = fixed_compression_2_buffer[3];
		   		   	  compressed_message[current_byte+4] = fixed_compression_2_buffer[4];
		   		   	  compressed_message[current_byte+5] = fixed_compression_2_buffer[5];
		   		   	  compressed_message[current_byte+6] = fixed_compression_2_buffer[6];
		   		   	  compressed_message[current_byte+7] = fixed_compression_2_buffer[7];
		   		   	  //memset(log,0,200);
		   		      // sprintf(log,"GPS_FIX not 8: i=%d-- byte:%d,bit:%d,compressed_message:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,compressed_message[0],compressed_message[1],compressed_message[2],compressed_message[3],compressed_message[4],compressed_message[5],compressed_message[6],compressed_message[7],compressed_message[8],compressed_message[9],compressed_message[10],compressed_message[11],compressed_message[12],compressed_message[13],compressed_message[14],compressed_message[15]);
		            		//printf(MOD_WAP,"%s",log);
		  		   } 
	  		   else//整数8，直接copy
		  		   {
		    		    	  compressed_message[current_byte] = fixed_compression_2_buffer[0];
		   		   	  compressed_message[current_byte+1] = fixed_compression_2_buffer[1];
		   		   	  compressed_message[current_byte+2] = fixed_compression_2_buffer[2];
		   		   	  compressed_message[current_byte+3] = fixed_compression_2_buffer[3];
		   		   	  compressed_message[current_byte+4] = fixed_compression_2_buffer[4];
		   		   	  compressed_message[current_byte+5] = fixed_compression_2_buffer[5];
		   		   	  compressed_message[current_byte+6] = fixed_compression_2_buffer[6];	   		   	  
		   		   	 // memset(log,0,200);
		   		     // sprintf(log,"GPS_FIX is 8: i=%d-- byte:%d,bit:%d,compressed_message:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,compressed_message[0],compressed_message[1],compressed_message[2],compressed_message[3],compressed_message[4],compressed_message[5],compressed_message[6],compressed_message[7],compressed_message[8],compressed_message[9],compressed_message[10],compressed_message[11],compressed_message[12],compressed_message[13],compressed_message[14],compressed_message[15]);
		            		//printf(MOD_WAP,"%s",log);
		   		   } 
	   		   //jumb 7个字节
	   		   jump_bit(56);  
	   		   maintask_printf("GPS_FIX:jump_bit(56)  i=%d-- current_byte:%d,current_bit:%d",i,current_byte,current_bit);     		   	 	 
	   		}		   		   
	   	}	
			
  	   }	
  	else if(compressed_type == 1 || compressed_type == 2)//4bit,comression_3	
	  {	
	   	memset(compressed_message,0,COMPRESSED_MSG_LEN);//2048	   	
	   	maintask_printf("GO compression_3");
	   	current_byte = 0;	
		current_bit = 0; 
		print_log_for_gps_backup(); 	
	   	//第一个bit是1，表示tracking message
	       set_bit_of_byte(compressed_message,current_byte,current_bit,1);	
		jump_bit(1); 
	       if(put_deviceid == 1)
		  {
		  	 U8 buffer_1[5];
		  	 memset(buffer_1,0,5);
	       	if(g_trk_var.g_user_settings.data_transfer_channel==REPLY_CHANNEL_DATA_MODULE) 
		             {     
		                  buffer_1[0] = g_trk_var.g_trk_device_status.deviceid_4G[0];
		                  buffer_1[1] = g_trk_var.g_trk_device_status.deviceid_4G[1];
		                  buffer_1[2] = g_trk_var.g_trk_device_status.deviceid_4G[2];
		                  buffer_1[3] = g_trk_var.g_trk_device_status.deviceid_4G[3];
		              }
	               else
		               {
		                   buffer_1[0] = g_trk_var.g_trk_device_status.deviceid_2G[0];
		                   buffer_1[1] = g_trk_var.g_trk_device_status.deviceid_2G[1];
		                   buffer_1[2] = g_trk_var.g_trk_device_status.deviceid_2G[2];
		                   buffer_1[3] = g_trk_var.g_trk_device_status.deviceid_2G[3];
		               }	  	 
		  	 //memset(log,0,200);
	   		// sprintf(log,"add device id: %x,%x,%x,%x",buffer_1[0],buffer_1[1],buffer_1[2],buffer_1[3]);
	      		// kal_prompt_trace(MOD_WAP,"%s",log);
		  	 bit_move_right_of_4byte(buffer_1,current_bit); 
		  	 compressed_message[0] = compressed_message[0] | buffer_1[0];
		  	 compressed_message[1] = buffer_1[1];
		  	 compressed_message[2] = buffer_1[2];
		  	 compressed_message[3] = buffer_1[3];
		  	 compressed_message[4] = buffer_1[4];	 
		  	 jump_bit(32);//跳4个byte 
		  }	 
		  
	   	for(i=0;i<actual_n;i++)
	   	{
	   		if(gps_backup[i].type == GPS_NO_FIX)
	   		{
	   			 set_bit_of_byte(compressed_message,current_byte,current_bit,0); 
	   			 jump_bit(1);
	   			 printf("GPS_NO_FIX: i=%d-- current_byte:%d,current_bit:%d",i,current_byte,current_bit);

	   		}
	   		else if(gps_backup[i].type == GPS_FIXED || gps_backup[i].type == GPS_BACKUP)
	   		{	
	   	     		compression_method_3_lat(gps_backup[i].latitude, compressed_lat, 4);
		       	compression_method_3_long(gps_backup[i].longitude, compressed_long, 4);
	   		   	fixed_compression_3_buffer[0] = compressed_lat[0] | 0x80; //前面第一个bit必须是1，表示有效坐标
	   		   	fixed_compression_3_buffer[1] = compressed_lat[1];
	   		   	fixed_compression_3_buffer[2] = compressed_lat[2];
	   		   	fixed_compression_3_buffer[3] = compressed_lat[3];
	   		   	fixed_compression_3_buffer[4] = compressed_long[0];
	   		   	fixed_compression_3_buffer[5] = compressed_long[1];
	   		  	fixed_compression_3_buffer[6] = compressed_long[2];
	   		  	fixed_compression_3_buffer[7] = compressed_long[3];
	   		   	if(gps_backup[i].speed > 255)
	   			   gps_backup[i].speed = 255;	
	   	     		fixed_compression_3_buffer[8] = gps_backup[i].speed; 		
	   		   	fixed_compression_3_buffer[9] = 0; 
	   		  // memset(log,0,200);
	   		 //  sprintf(log,"GPS_FIX 1: i=%d-- byte:%d,bit:%d,fixed_compression_3_buffer:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,fixed_compression_3_buffer[0],fixed_compression_3_buffer[1],fixed_compression_3_buffer[2],fixed_compression_3_buffer[3],fixed_compression_3_buffer[4],fixed_compression_3_buffer[5],fixed_compression_3_buffer[6],fixed_compression_3_buffer[7],fixed_compression_3_buffer[8],fixed_compression_3_buffer[9]);
	        	// kal_prompt_trace(MOD_WAP,"%s",log);
	   		   if(current_bit != 0)//整数8，什么都不做
		   		   {	
		   		   	  //必须move current_bit才能跟以前对的上
		   		   	  bit_move_right_of_9byte(fixed_compression_3_buffer,current_bit);
		   		   	  //memset(log,0,200);
		   		       //sprintf(log,"GPS_FIX not 8:i=%d-- byte:%d,bit:%d,fixed_compression_3_buffer:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,fixed_compression_3_buffer[0],fixed_compression_3_buffer[1],fixed_compression_3_buffer[2],fixed_compression_3_buffer[3],fixed_compression_3_buffer[4],fixed_compression_3_buffer[5],fixed_compression_3_buffer[6],fixed_compression_3_buffer[7],fixed_compression_3_buffer[8],fixed_compression_3_buffer[9]);
		            		//kal_prompt_trace(MOD_WAP,"%s",log);
		   		   	  //第一个字节，直接或，没问题,其他所有没填的bit都是0
		   		   	  compressed_message[current_byte] = compressed_message[current_byte] | fixed_compression_3_buffer[0];
		   		   	  //剩下直接copy
		   		   	  compressed_message[current_byte+1] = fixed_compression_3_buffer[1];
		   		   	  compressed_message[current_byte+2] = fixed_compression_3_buffer[2];
		   		   	  compressed_message[current_byte+3] = fixed_compression_3_buffer[3];
		   		   	  compressed_message[current_byte+4] = fixed_compression_3_buffer[4];
		   		   	  compressed_message[current_byte+5] = fixed_compression_3_buffer[5];
		   		   	  compressed_message[current_byte+6] = fixed_compression_3_buffer[6];
		   		   	  compressed_message[current_byte+7] = fixed_compression_3_buffer[7];
		   		   	  compressed_message[current_byte+8] = fixed_compression_3_buffer[8];
		   		   	  compressed_message[current_byte+9] = fixed_compression_3_buffer[9];
		   		   	  
		   		   	  //memset(log,0,200);
		   		     	  // sprintf(log,"GPS_FIX not 8: i=%d-- byte:%d,bit:%d,compressed_message:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,compressed_message[0],compressed_message[1],compressed_message[2],compressed_message[3],compressed_message[4],compressed_message[5],compressed_message[6],compressed_message[7],compressed_message[8],compressed_message[9],compressed_message[10],compressed_message[11],compressed_message[12],compressed_message[13],compressed_message[14],compressed_message[15]);
		            		 //kal_prompt_trace(MOD_WAP,"%s",log);
		  		   } 
	  		   else//整数8，直接copy
				{
					compressed_message[current_byte] = fixed_compression_2_buffer[0];
					compressed_message[current_byte+1] = fixed_compression_2_buffer[1];
					compressed_message[current_byte+2] = fixed_compression_2_buffer[2];
					compressed_message[current_byte+3] = fixed_compression_2_buffer[3];
					compressed_message[current_byte+4] = fixed_compression_2_buffer[4];
					compressed_message[current_byte+5] = fixed_compression_2_buffer[5];
					compressed_message[current_byte+6] = fixed_compression_2_buffer[6];
					compressed_message[current_byte+7] = fixed_compression_2_buffer[7];
					compressed_message[current_byte+8] = fixed_compression_2_buffer[8];

					//memset(log,0,200);
					//sprintf(log,"GPS_FIX is 8: i=%d-- byte:%d,bit:%d,compressed_message:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,compressed_message[0],compressed_message[1],compressed_message[2],compressed_message[3],compressed_message[4],compressed_message[5],compressed_message[6],compressed_message[7],compressed_message[8],compressed_message[9],compressed_message[10],compressed_message[11],compressed_message[12],compressed_message[13],compressed_message[14],compressed_message[15]);
					//kal_prompt_trace(MOD_WAP,"%s",log);
				} 
	   		   //jumb 9个字节
	   		   jump_bit(72);
	   		   sprintf("GPS_FIX:jump_bit(56)  i=%d-- current_byte:%d,current_bit:%d",i,current_byte,current_bit);   		   	 	 
	   		}   
	   	}
	  }	
  	maintask_printf("creat_compressed_message:%s",compressed_message);	
}
	
BOOL gps_history_all_no_fix(U16 history_number)	
{
	BOOL ret = TRUE;
	U16 i=0;
	U16 actual_n = history_number;
	U16 num_1 = get_number_of_records_in_backup_database();
	
 	 if(actual_n > num_1)
	   actual_n = num_1;
	   
	if(actual_n == 0)
	{
		maintask_printf("gps_history_all_no_fix TRUE, ALL 0");	
		return TRUE;
  	}		
	 for(i=0;i<actual_n;i++)
	   {
	   		if(gps_backup[i].type == GPS_FIXED || gps_backup[i].type == GPS_BACKUP) 
	   		{	
	   			maintask_printf("gps_history_all_no_fix KAL_FALSE, fix i=%d",i);
	   			return FALSE;
	   		}	
	   }
   
   maintask_printf("gps_history_all_no_fix KAL_TRUE");
   return TRUE;
}  


/**************************************************************************
delta
***************************************************************************/

BOOL Is_first_cordinates_available_for_delta(void)
{
	if(gps_backup[0].type == GPS_FIXED || gps_backup[0].type == GPS_BACKUP)
	{
			return TRUE;
	}
	else
	{
		  return FALSE;
	}
}		  			


void data_send_any_string(char* str_data, int length)
{ 
     tracker_struct_msg *message;         
     message = (tracker_struct_msg*)myMalloc(sizeof(tracker_struct_msg));
     memset(message , 0, sizeof(tracker_struct_msg));
     message->msgLen=length;
     memcpy(message->body_msg,(U8*)str_data,length);
     module_message_buffer_push(message);
     myFree(message);
}

/*********************************************************************
delta
maxVal = 0.5       (this is more than 10 kms)

numOfBits = 16?
***********************************************************************/
void delta_compression_method(double latitude, char* lat_1,U8 length)	
{
	double val_lat = latitude;
	double max_lat_val = 0.5;
	double scaled_val = 0;
	char log[200];
	double double_1=0;
	double uint64_1=0;
	double transmitted_val = 0;
	int final = 0;
	scaled_val = val_lat + (val_lat<0? 2*max_lat_val:0);
	double_1 = scaled_val/(2*max_lat_val);
	uint64_1 = 65535;//2的16次方减1
	transmitted_val = uint64_1*double_1;
	//四舍五入
	//final = (kal_uint32)(transmitted_val+0.5);
	//memset(log,0,200);
	//sprintf(log,"delta_compression_method ori:%f final:%d, %x",latitude,final,final);
	//kal_prompt_trace(MOD_WAP,"%s",log);
	memset(lat_1, 0, length);
	lat_1[0] = final/256;
	lat_1[1] = final - lat_1[0]*256;
	maintask_printf("delta_compression_method %x,%x",lat_1[0],lat_1[1]);
}  


//压缩完，"latlongspeedlatlongspeed....#",最多只能压缩254,put_deviceid为是否加device id
void delta_creat_compressed_message(U8 compressed_number, char compressed_type,U8 put_deviceid)
{
	U16 i=0;
	U16 j=0;
	U16 actual_n=0;
	U16 num_1 = 0;
	U16 len = 0;
	U8 bat = 0;
	U8 speed = 0;
	double lat_first = 0;
	double long_first = 0;
	double lat_difference = 0;
	double long_difference = 0;
	char compressed_lat[4];
	char compressed_long[4];
	char fixed_compression_2_buffer[8]; //总共7位，多留一位串位用
	char fixed_compression_3_buffer[10]; //总共9位，多留一位串位用
	char fixed_compression_delta_buffer[6];//总共5位，多留一位串位用
	char* temp = NULL;
	char log[200];
	
	if(compressed_number == 0)
		return;	
	if(Is_first_cordinates_available_for_delta() == FALSE)
		return;		
	memset(compressed_lat,0,4);
       memset(compressed_long,0,4);		
       memset(fixed_compression_2_buffer,0,8);
       memset(fixed_compression_3_buffer,0,10);			
  	actual_n = compressed_number;		
  	num_1 = get_number_of_records_in_backup_database();
	  if(actual_n > num_1)
		   actual_n = num_1;
 	 if(actual_n == 0)
 		return;
   	memset(compressed_message,0,COMPRESSED_MSG_LEN);//2048	
   	maintask_printf("GO compression_2");
   	current_byte = 0;	
	current_bit = 0;
	print_log_for_gps_backup();
   	//第一个bit是1，表示tracking message
    //先写后跳
    	set_bit_of_byte(compressed_message,current_byte,current_bit,1);	
	  jump_bit(1);
	  if(put_deviceid == 1)
	  {
	  	 U8 buffer_1[5];
	  	 memset(buffer_1,0,5);
	  	 
         if(g_trk_var.g_user_settings.data_transfer_channel==REPLY_CHANNEL_DATA_MODULE) 
              {     
                   buffer_1[0] = g_trk_var.g_trk_device_status.deviceid_4G[0];
                   buffer_1[1] = g_trk_var.g_trk_device_status.deviceid_4G[1];
                   buffer_1[2] = g_trk_var.g_trk_device_status.deviceid_4G[2];
                   buffer_1[3] = g_trk_var.g_trk_device_status.deviceid_4G[3];
               }
                else
                {
                    buffer_1[0] = g_trk_var.g_trk_device_status.deviceid_2G[0];
                    buffer_1[1] = g_trk_var.g_trk_device_status.deviceid_2G[1];
                    buffer_1[2] = g_trk_var.g_trk_device_status.deviceid_2G[2];
                    buffer_1[3] = g_trk_var.g_trk_device_status.deviceid_2G[3];
                }
	  	 
	  	// memset(log,0,200);
   		// sprintf(log,"add device id: %x,%x,%x,%x",buffer_1[0],buffer_1[1],buffer_1[2],buffer_1[3]);
       	//kal_prompt_trace(MOD_WAP,"%s",log);
	  	 bit_move_right_of_4byte(buffer_1,current_bit);
	  	 compressed_message[0] = compressed_message[0] | buffer_1[0];
	  	 compressed_message[1] = buffer_1[1];
	  	 compressed_message[2] = buffer_1[2];
	  	 compressed_message[3] = buffer_1[3];
	  	 compressed_message[4] = buffer_1[4];
	  	 jump_bit(32);//跳4个byte 
	  }	
	  
	  //首个cordinates
	if(compressed_type == 0)//3bit,comression_2	
	{	
		i=0;
	   compression_method_2_lat(gps_backup[i].latitude, compressed_lat, 4);
	   compression_method_2_long(gps_backup[i].longitude, compressed_long, 4);
		
	   fixed_compression_2_buffer[0] = compressed_lat[0] | 0x80; //前面第一个bit必须是1，表示有效坐标
	   fixed_compression_2_buffer[1] = compressed_lat[1];
	   fixed_compression_2_buffer[2] = compressed_lat[2];
		
	   fixed_compression_2_buffer[3] = compressed_long[0];
	   fixed_compression_2_buffer[4] = compressed_long[1];
	   fixed_compression_2_buffer[5] = compressed_long[2];
		
	   if(gps_backup[i].speed > 255)
		    gps_backup[i].speed = 255;
	   fixed_compression_2_buffer[6] = gps_backup[i].speed; 		
	   fixed_compression_2_buffer[7] = 0; 
	  // memset(log,0,200);
	  // sprintf(log,"GPS_FIX 1: i=%d-- byte:%d,bit:%d,fixed_compression_2_buffer:%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,fixed_compression_2_buffer[0],fixed_compression_2_buffer[1],fixed_compression_2_buffer[2],fixed_compression_2_buffer[3],fixed_compression_2_buffer[4],fixed_compression_2_buffer[5],fixed_compression_2_buffer[6],fixed_compression_2_buffer[7]);
	  // kal_prompt_trace(MOD_WAP,"%s",log);
	   if(current_bit != 0)//整数8，什么都不做
	   {	
	  	  //必须move current_bit才能跟以前对的上
	  	  bit_move_right_of_7byte(fixed_compression_2_buffer,current_bit);
	  	  //memset(log,0,200);
	    	 // sprintf(log,"GPS_FIX not 8: i=%d-- byte:%d,bit:%d,fixed_compression_2_buffer:%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,fixed_compression_2_buffer[0],fixed_compression_2_buffer[1],fixed_compression_2_buffer[2],fixed_compression_2_buffer[3],fixed_compression_2_buffer[4],fixed_compression_2_buffer[5],fixed_compression_2_buffer[6],fixed_compression_2_buffer[7]);
	     	 //kal_prompt_trace(MOD_WAP,"%s",log);
	  	  //第一个字节，直接或，没问题,其他所有没填的bit都是0
	  	  compressed_message[current_byte] = compressed_message[current_byte] | fixed_compression_2_buffer[0];
	  	  //剩下直接copy
	  	  compressed_message[current_byte+1] = fixed_compression_2_buffer[1];
	  	  compressed_message[current_byte+2] = fixed_compression_2_buffer[2];
	  	  compressed_message[current_byte+3] = fixed_compression_2_buffer[3];
	  	  compressed_message[current_byte+4] = fixed_compression_2_buffer[4];
	  	  compressed_message[current_byte+5] = fixed_compression_2_buffer[5];
	  	  compressed_message[current_byte+6] = fixed_compression_2_buffer[6];
	  	  compressed_message[current_byte+7] = fixed_compression_2_buffer[7];
	  	//memset(log,0,200);
	     	//sprintf(log,"GPS_FIX not 8: i=%d-- byte:%d,bit:%d,compressed_message:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,compressed_message[0],compressed_message[1],compressed_message[2],compressed_message[3],compressed_message[4],compressed_message[5],compressed_message[6],compressed_message[7],compressed_message[8],compressed_message[9],compressed_message[10],compressed_message[11],compressed_message[12],compressed_message[13],compressed_message[14],compressed_message[15]);
	     	//kal_prompt_trace(MOD_WAP,"%s",log);
	 } 
	 else//整数8，直接copy
	 {
		  compressed_message[current_byte] = fixed_compression_2_buffer[0];
	  	  compressed_message[current_byte+1] = fixed_compression_2_buffer[1];
	  	  compressed_message[current_byte+2] = fixed_compression_2_buffer[2];
	  	  compressed_message[current_byte+3] = fixed_compression_2_buffer[3];
	  	  compressed_message[current_byte+4] = fixed_compression_2_buffer[4];
	  	  compressed_message[current_byte+5] = fixed_compression_2_buffer[5];
	  	  compressed_message[current_byte+6] = fixed_compression_2_buffer[6];
		//memset(log,0,200);
		//sprintf(log,"GPS_FIX is 8: i=%d-- byte:%d,bit:%d,compressed_message:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,compressed_message[0],compressed_message[1],compressed_message[2],compressed_message[3],compressed_message[4],compressed_message[5],compressed_message[6],compressed_message[7],compressed_message[8],compressed_message[9],compressed_message[10],compressed_message[11],compressed_message[12],compressed_message[13],compressed_message[14],compressed_message[15]);
		//kal_prompt_trace(MOD_WAP,"%s",log);
	  } 
	  
	  //jumb 7个字节
	  jump_bit(56);
	  maintask_printf("GPS_FIX:jump_bit(56)  i=%d-- current_byte:%d,current_bit:%d",i,current_byte,current_bit);
	}
  	else if(compressed_type == 1 || compressed_type == 2)//4bit,comression_3	
	  {	
	    i=0;
	    compression_method_3_lat(gps_backup[i].latitude, compressed_lat, 4);
	    compression_method_3_long(gps_backup[i].longitude, compressed_long, 4);
	    fixed_compression_3_buffer[0] = compressed_lat[0] | 0x80; //前面第一个bit必须是1，表示有效坐标
	    fixed_compression_3_buffer[1] = compressed_lat[1];
	    fixed_compression_3_buffer[2] = compressed_lat[2];
	    fixed_compression_3_buffer[3] = compressed_lat[3];
	    fixed_compression_3_buffer[4] = compressed_long[0];
	    fixed_compression_3_buffer[5] = compressed_long[1];
	    fixed_compression_3_buffer[6] = compressed_long[2];
	    fixed_compression_3_buffer[7] = compressed_long[3];
	    if(gps_backup[i].speed > 255)
	 	   gps_backup[i].speed = 255;
	    fixed_compression_3_buffer[8] = gps_backup[i].speed; 		
	    fixed_compression_3_buffer[9] = 0; 
	    //memset(log,0,200);
	    //sprintf(log,"GPS_FIX 1: i=%d-- byte:%d,bit:%d,fixed_compression_3_buffer:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,fixed_compression_3_buffer[0],fixed_compression_3_buffer[1],fixed_compression_3_buffer[2],fixed_compression_3_buffer[3],fixed_compression_3_buffer[4],fixed_compression_3_buffer[5],fixed_compression_3_buffer[6],fixed_compression_3_buffer[7],fixed_compression_3_buffer[8],fixed_compression_3_buffer[9]);
	    //kal_prompt_trace(MOD_WAP,"%s",log);
	    if(current_bit != 0)//整数8，什么都不做
	    {	
	    	  //必须move current_bit才能跟以前对的上
	    	  bit_move_right_of_9byte(fixed_compression_3_buffer,current_bit);
	    	  //memset(log,0,200);
	        //sprintf(log,"GPS_FIX not 8:i=%d-- byte:%d,bit:%d,fixed_compression_3_buffer:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,fixed_compression_3_buffer[0],fixed_compression_3_buffer[1],fixed_compression_3_buffer[2],fixed_compression_3_buffer[3],fixed_compression_3_buffer[4],fixed_compression_3_buffer[5],fixed_compression_3_buffer[6],fixed_compression_3_buffer[7],fixed_compression_3_buffer[8],fixed_compression_3_buffer[9]);
	        //kal_prompt_trace(MOD_WAP,"%s",log);
	    	  //第一个字节，直接或，没问题,其他所有没填的bit都是0
	    	  compressed_message[current_byte] = compressed_message[current_byte] | fixed_compression_3_buffer[0];
	    	  
	    	  //剩下直接copy
	    	  compressed_message[current_byte+1] = fixed_compression_3_buffer[1];
	    	  compressed_message[current_byte+2] = fixed_compression_3_buffer[2];
	    	  compressed_message[current_byte+3] = fixed_compression_3_buffer[3];
	    	  compressed_message[current_byte+4] = fixed_compression_3_buffer[4];
	    	  compressed_message[current_byte+5] = fixed_compression_3_buffer[5];
	    	  compressed_message[current_byte+6] = fixed_compression_3_buffer[6];
	    	  compressed_message[current_byte+7] = fixed_compression_3_buffer[7];
	    	  compressed_message[current_byte+8] = fixed_compression_3_buffer[8];
	    	  compressed_message[current_byte+9] = fixed_compression_3_buffer[9];
	    	  
	    	 // memset(log,0,200);
	      // sprintf(log,"GPS_FIX not 8: i=%d-- byte:%d,bit:%d,compressed_message:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,compressed_message[0],compressed_message[1],compressed_message[2],compressed_message[3],compressed_message[4],compressed_message[5],compressed_message[6],compressed_message[7],compressed_message[8],compressed_message[9],compressed_message[10],compressed_message[11],compressed_message[12],compressed_message[13],compressed_message[14],compressed_message[15]);
	       //kal_prompt_trace(MOD_WAP,"%s",log);
	    } 
	    else//整数8，直接copy
	    {
	 	  compressed_message[current_byte] = fixed_compression_2_buffer[0];
	    	  compressed_message[current_byte+1] = fixed_compression_2_buffer[1];
	    	  compressed_message[current_byte+2] = fixed_compression_2_buffer[2];
	    	  compressed_message[current_byte+3] = fixed_compression_2_buffer[3];
	    	  compressed_message[current_byte+4] = fixed_compression_2_buffer[4];
	    	  compressed_message[current_byte+5] = fixed_compression_2_buffer[5];
	    	  compressed_message[current_byte+6] = fixed_compression_2_buffer[6];
	    	  compressed_message[current_byte+7] = fixed_compression_2_buffer[7];
	    	  compressed_message[current_byte+8] = fixed_compression_2_buffer[8];
	    	  
	    	  //memset(log,0,200);
	       // sprintf(log,"GPS_FIX is 8: i=%d-- byte:%d,bit:%d,compressed_message:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,compressed_message[0],compressed_message[1],compressed_message[2],compressed_message[3],compressed_message[4],compressed_message[5],compressed_message[6],compressed_message[7],compressed_message[8],compressed_message[9],compressed_message[10],compressed_message[11],compressed_message[12],compressed_message[13],compressed_message[14],compressed_message[15]);
	       // kal_prompt_trace(MOD_WAP,"%s",log);
	    } 
	    //jumb 9个字节
	    jump_bit(72);
	    maintask_printf("GPS_FIX:jump_bit(56)  i=%d-- current_byte:%d,current_bit:%d",i,current_byte,current_bit);
	  }
  	else
	  {
	  	maintask_printf("compressed_type=%d return",compressed_type);
	  	return;
	  }	
	  //lat_first = gps_backup[i].latitude;
	 // long_first = gps_backup[i].longitude;
	//memset(log,0,200);
	//sprintf(log,"lat_first = %f, long_first = %f",lat_first,long_first);
	//kal_prompt_trace(MOD_WAP,"%s",log);
   	for(i=1;i<actual_n;i++)
   	{
   		if(gps_backup[i].type == GPS_NO_FIX)
   		{
   			 //先写后跳
   			 set_bit_of_byte(compressed_message,current_byte,current_bit,0);
   			 jump_bit(1);
   			 maintask_printf("GPS_NO_FIX: i=%d-- current_byte:%d,current_bit:%d",i,current_byte,current_bit);
   		}
   		else if(gps_backup[i].type == GPS_FIXED || gps_backup[i].type == GPS_BACKUP)
   		{	
   			// lat_difference = gps_backup[i].latitude - lat_first;
   			// long_difference = gps_backup[i].longitude - long_first;
   			// memset(log,0,200);
         		//sprintf(log,"fix:lat=%f,long=%f,lat_difference=%f,long_difference=%f",gps_backup[i].latitude,gps_backup[i].longitude,lat_difference,long_difference);
         		//(MOD_WAP,"%s",log);
   	     		delta_compression_method(lat_difference, compressed_lat, 4);
	       	delta_compression_method(long_difference , compressed_long, 4);
	       //不需要正负bit，直接压，压缩里面有正负
   			 fixed_compression_delta_buffer[0] = compressed_lat[0];
   			 fixed_compression_delta_buffer[1] = compressed_lat[1];  
   			 fixed_compression_delta_buffer[2] = compressed_long[0]; 
   			 fixed_compression_delta_buffer[3] = compressed_long[1]; 
   			 fixed_compression_delta_buffer[4] = gps_backup[i].speed; //speed
   			 fixed_compression_delta_buffer[5] = 0; 
   			 
   			 //有效要写一个1
   			// set_bit_of_byte(compressed_message,current_byte,current_bit,1);
   			// jump_bit(1);
   			// memset(log,0,200);
   		   	//sprintf(log,"GPS_FIX before move: i=%d-- byte:%d,bit:%d,fixed_compression_delta_buffer:%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,fixed_compression_delta_buffer[0],fixed_compression_delta_buffer[1],fixed_compression_delta_buffer[2],fixed_compression_delta_buffer[3],fixed_compression_delta_buffer[4],fixed_compression_delta_buffer[5]);
         		//kal_prompt_trace(MOD_WAP,"%s",log);
            
   			 if(current_bit != 0)//整数8，什么都不做
			{	
			  //必须move current_bit才能跟以前对的上
			 // bit_move_right_of_5byte(fixed_compression_delta_buffer,current_bit);
		        //  memset(log,0,200);
			//sprintf(log,"GPS_FIX not 8 after: i=%d-- byte:%d,bit:%d,fixed_compression_delta_buffer:%x,%x,%x,%x,%x,%x",i,current_byte,current_bit,fixed_compression_delta_buffer[0],fixed_compression_delta_buffer[1],fixed_compression_delta_buffer[2],fixed_compression_delta_buffer[3],fixed_compression_delta_buffer[4],fixed_compression_delta_buffer[5]);
			//kal_prompt_trace(MOD_WAP,"%s",log);
			  //第一个字节，直接或，没问题,其他所有没填的bit都是0
			  compressed_message[current_byte] = compressed_message[current_byte] | fixed_compression_delta_buffer[0];
			  //剩下直接copy
			  compressed_message[current_byte+1] = fixed_compression_delta_buffer[1];
			  compressed_message[current_byte+2] = fixed_compression_delta_buffer[2];
			  compressed_message[current_byte+3] = fixed_compression_delta_buffer[3];
			  compressed_message[current_byte+4] = fixed_compression_delta_buffer[4];
			  compressed_message[current_byte+5] = fixed_compression_delta_buffer[5]; 
			} 
  		    else//整数8，直接copy
			{
			compressed_message[current_byte] = fixed_compression_2_buffer[0];
			compressed_message[current_byte+1] = fixed_compression_2_buffer[1];
			compressed_message[current_byte+2] = fixed_compression_2_buffer[2];
			compressed_message[current_byte+3] = fixed_compression_2_buffer[3];
			compressed_message[current_byte+4] = fixed_compression_2_buffer[4];
			} 
   		   //jumb 5个字节
   		   jump_bit(40);
       }
    }      		   

   	//最后，如果跳到一个byte头上，退到上一个byte
   	if(current_bit == 0 && current_byte>0)
   	{
   		current_byte--;
   	}	
  maintask_printf("creat_compressed_message:%s",compressed_message);
}

BOOL gps_xmode_send(void) 	  	
{
	BOOL ret = FALSE;
	if(g_trk_var.g_trk_settings.Xmode_report_time==0)
	{
	maintask_printf("gps_xmode_send error :g_trk_var.g_trk_settings.Xmode_report_time =0\n");
	return FALSE;
	}
	xmode_send_times++;
	maintask_printf("gps_xmode_send xmode_send_times=%d,g_trk_var.g_trk_settings.Xmode_report_time=%d\n",xmode_send_times,g_trk_var.g_trk_settings.Xmode_report_time); 
	if(xmode_send_times >= g_trk_var.g_trk_settings.Xmode_report_time)
	{	
		xmode_send_times = 0;		
		ret = TRUE;		
		//不允许走sms，如果设成sms，直接切成data    
		if(g_trk_var.g_trk_settings.Xmode_channel == 1)
			g_trk_var.g_trk_settings.Xmode_channel = 2;
		
		if(g_trk_var.g_trk_settings.Xmode_location_accuracy== 0 || g_trk_var.g_trk_settings.Xmode_location_accuracy== 1 || g_trk_var.g_trk_settings.Xmode_location_accuracy == 2)
		{	
		   if(g_trk_var.g_trk_settings.Xmode_channel== 2) //走data
			{
			 maintask_printf("gps_xmode_send data: delta:%d, add device id:%d, multi tracks:%d, compression:%d\n",
			                                       g_trk_var.g_trk_settings.Xmode_delta_compression,
			                                       g_trk_var.g_trk_settings.Xmode_transmit_deviceid,
			                                       g_trk_var.g_trk_settings.Xmode_report_time,
			                                       g_trk_var.g_trk_settings.Xmode_location_accuracy);
			  
			 //-	Delta Compression (0 – off/1 – on) (1 byte) 
			 if(g_trk_var.g_trk_settings.Xmode_delta_compression== 0)//no delta
			 {	
			       //-	Transmit Device ID (0 – off/1 – on) (1 byte) 
			       if(g_trk_var.g_trk_settings.Xmode_transmit_deviceid == 1) 
				{
				//此处用来发送单条信息单多条历史坐标
					creat_compressed_message(g_trk_var.g_trk_settings.Xmode_report_time,g_trk_var.g_trk_settings.Xmode_location_accuracy,1); //2是一次发几条，4是compression type
				}
				else
				{
				 	creat_compressed_message(g_trk_var.g_trk_settings.Xmode_report_time,g_trk_var.g_trk_settings.Xmode_location_accuracy,0); //2是一次发几条，4是compression type	  
				}
			       
				if(FALSE == gps_history_all_no_fix(g_trk_var.g_trk_settings.Xmode_report_time))
				{	
					data_send_any_string(compressed_message,(current_byte+1));
				}    
			  }
			  else //delta enable
	    			   {
	    			   	   if(Is_first_cordinates_available_for_delta() == TRUE)
	    					{	
	    					//-	Transmit Device ID (0 – off/1 – on) (1 byte) 
	    					if(g_trk_var.g_trk_settings.Xmode_transmit_deviceid== 1) 
	    					{
	    					//此处用来发送单条信息单多条历史坐标
	    						delta_creat_compressed_message(g_trk_var.g_trk_settings.Xmode_report_time,g_trk_var.g_trk_settings.Xmode_location_accuracy,1); //2是一次发几条，4是compression type
	    					 }
	    					 else
	    					 {
	    						delta_creat_compressed_message(g_trk_var.g_trk_settings.Xmode_report_time,g_trk_var.g_trk_settings.Xmode_location_accuracy,0); //2是一次发几条，4是compression type	  
	    					 }
	    					 
	    					 if(FALSE == gps_history_all_no_fix(g_trk_var.g_trk_settings.Xmode_report_time))
	    					 {	
	    					    data_send_any_string(compressed_message,(current_byte+1));
	    					 }   
	    					}
	    				      else
	    				      {	
	    				      	 maintask_printf("gps_xmode_send data: delta , first cordinates no fix, don't send");
	    				      }	   
	    			   }        		       	   		   
			 
			}
	 	else if(g_trk_var.g_trk_settings.Xmode_channel == 0) //走ussd
		 {		     
		  	     maintask_printf("gps_xmode_send data: delta:%d, add device id:%d, multi tracks:%d, compression:%d\n",
		                                               g_trk_var.g_trk_settings.Xmode_delta_compression,
		                                               g_trk_var.g_trk_settings.Xmode_transmit_deviceid,
		                                               g_trk_var.g_trk_settings.Xmode_report_time,
		                                               g_trk_var.g_trk_settings.Xmode_location_accuracy);
		         
			     //-	Delta Compression (0 – off/1 – on) (1 byte) 
			     if(g_trk_var.g_trk_settings.Xmode_delta_compression == 0)//no delta
			     {	
			        //此处用来发送单条信息单多条历史坐标
			          creat_compressed_message(g_trk_var.g_trk_settings.Xmode_report_time,g_trk_var.g_trk_settings.Xmode_location_accuracy,0); //2是一次发几条，4是compression type
			       }
			       else
			       {
			       	  if(Is_first_cordinates_available_for_delta() == TRUE)
			       	  {	
			           //此处用来发送单条信息单多条历史坐标
			             delta_creat_compressed_message(g_trk_var.g_trk_settings.Xmode_report_time,g_trk_var.g_trk_settings.Xmode_location_accuracy,0); //2是一次发几条，4是compression type
			          }
			          else
			          {
			          	maintask_printf("gps_xmode_send ussd: delta , first cordinates no fix, don't send");
			          	return FALSE;//没成功
			          }		   
			       }	   	   

		     if(FALSE == gps_history_all_no_fix(g_trk_var.g_trk_settings.Xmode_report_time))
		     {	
		 	     if(2*(current_byte+1) < 140)//choice sim一次最多发送140到150bytes
		 	     {
		 	     	  U8 len=0; 	
		 	        data_send_any_string(compressed_message,(current_byte+1));
		 	     }
		 	     else
		 	     {
		                
		                g_trk_var.g_trk_content_info.command=COMMAND_TRACKING_MODE_START;
		                tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		 	     }
		 	     
			     }	 	  

		 }
	 	check_no_gsm_history(g_trk_var.g_trk_settings.Xmode_report_time);   
	  }	   		
	}
	if(ret == TRUE)
	{	
		maintask_printf("gps_xmode_send TRUE");
	}
	else
	{
		maintask_printf("gps_xmode_send FALSE");
	}	 	  
	return ret;
}    
void gps_always_on_keep_sending(void)
{
	if(timer_minutes == 99)//99就是gps常开，每10s一次，不用backup
	{	
	 if(TRUE == gps_2d_3d_fix_with_hdop_and_fix_sec(10))
	 {
		maintask_printf("99 FIXED");  	  	
		gps_back_up_push_fixed();//保存至backup
		gps_xmode_send();
	 }
 	else
	 {
		maintask_printf("99 NO FIXED");
		gps_back_up_push_no_fix();
		gps_xmode_send();
	 }	
	}
	stopMyTimer(MY_TIMER_34);
	startMyTimer(MY_TIMER_34, 20*1000, gps_always_on_keep_sending);
}

BOOL fence_or_speed_alarm_on(void)
{
	//5个fence,5个speed,都记录在4里面
	if((g_trk_var.g_trk_settings.gps_fance_alarm_setting[0][4] == 0 &&
		  g_trk_var.g_trk_settings.gps_fance_alarm_setting[1][4] == 0 &&
		  g_trk_var.g_trk_settings.gps_fance_alarm_setting[2][4] == 0 &&
		  g_trk_var.g_trk_settings.gps_fance_alarm_setting[3][4] == 0 &&
		  g_trk_var.g_trk_settings.gps_fance_alarm_setting[4][4] == 0 )
		  && 
		  ( g_trk_var.g_trk_settings.gps_speed_alarm_setting[0][4] == 0 &&
		    g_trk_var.g_trk_settings.gps_speed_alarm_setting[1][4] == 0 &&
		    g_trk_var.g_trk_settings.gps_speed_alarm_setting[2][4] == 0 &&
		    g_trk_var.g_trk_settings.gps_speed_alarm_setting[3][4] == 0 &&
		    g_trk_var.g_trk_settings.gps_speed_alarm_setting[4][4] == 0 ))
	{
		 maintask_printf("fence_or_speed_alarm_on KAL_FALSE");
		 return FALSE;
	}
	else
	{
		 maintask_printf("fence_or_speed_alarm_on KAL_TRUE");
		 return TRUE;
	}
}		 	
 		  	
U8 get_fence_speed_checking_timer_in_sec(void)
{
	U8 speed_fence_timer = 0;
	if(g_trk_var.g_trk_settings.gps_search_control[2]== 0)
	{
		speed_fence_timer = CHECK_SPEED_FENCE_TIMER_SEC;
	}
	else
	{
		speed_fence_timer = g_trk_var.g_trk_settings.gps_search_control[2];
	}
	maintask_printf("get_fence_speed_checking_timer_in_sec:%d\r\n",speed_fence_timer);
	return speed_fence_timer;
}

BOOL current_speed_less(U8 limit)
{
	if(g_trk_var.g_trk_content_info.gps_info.speed_km /1000 < limit)
	{	
	 return TRUE;
	}
  	else
      {  
      	 return FALSE;	
      }	    
}

BOOL current_speed_more(U8 limit)
{
     maintask_printf("get_fence_speed_checking_timer_in_sec:%d\r\n",(U8)(g_trk_var.g_trk_content_info.gps_info.speed_km /1000));
    if((U8)(g_trk_var.g_trk_content_info.gps_info.speed_km /1000) >= limit)
  {	
        return TRUE;
  }
  else
  {  
   	return FALSE;	
  }
} 

void speed_alarm_0_time_unlock(void)
{
	speed_alarm_time_lock[0] = 0;
}	

void speed_alarm_1_time_unlock(void)
{
	speed_alarm_time_lock[1] = 0;
}	

void speed_alarm_2_time_unlock(void)
{
	speed_alarm_time_lock[2] = 0;
}	

void speed_alarm_3_time_unlock(void)
{
	speed_alarm_time_lock[3] = 0;
}

void speed_para_reset(void)
{
	speed_alarm_time_lock[0] = 0;
	speed_alarm_time_lock[1] = 0;
	speed_alarm_time_lock[2] = 0;
	speed_alarm_time_lock[3] = 0;
	speed_alarm_time_lock[4] = 0;
}

void fence_para_reset_sig(U8 fence_number)
{
	fence_alarm_time_lock[fence_number] = 0;
}

void fence_alarm_0_time_unlock(void)
{
	fence_alarm_time_lock[0] = 0;
}	

void fence_alarm_1_time_unlock(void)
{
	fence_alarm_time_lock[1] = 0;
}	

void fence_alarm_2_time_unlock(void)
{
	fence_alarm_time_lock[2] = 0;
}	

void fence_alarm_3_time_unlock(void)
{
	fence_alarm_time_lock[3] = 0;
}	

void fence_alarm_4_time_unlock(void)
{
	fence_alarm_time_lock[4] = 0;
}	

void speed_alarm_4_time_unlock(void)
{
	speed_alarm_time_lock[4] = 0;
}

void fence_para_reset(void)
{
	fence_alarm_time_lock[0] = 0;
	fence_alarm_time_lock[1] = 0;
	fence_alarm_time_lock[2] = 0;
	fence_alarm_time_lock[3] = 0;
	fence_alarm_time_lock[4] = 0;
}

//shlomo要求改成比那个值低就要报警
BOOL bat_alarm_sent[5] = {FALSE,FALSE,FALSE,FALSE,FALSE};
void bat_alarm_sent_sig_reset(void)
{
	bat_alarm_sent[0] = FALSE;
	bat_alarm_sent[1] = FALSE;
	bat_alarm_sent[2] = FALSE;
	bat_alarm_sent[3] = FALSE;
	bat_alarm_sent[4] = FALSE;
}	

void reset_bat_alarm_sig(U8 bat_num)
{
	bat_alarm_sent[bat_num] = FALSE;
}

void check_speed_alarm(void)
{
	maintask_printf("check_speed_alarm al111111111arm on\r\n");
	U16 speed_max;
	U8 i=0;
	reply_channel rr_channel;
	//if((g_trk_var.g_trk_content_info.gps_info.gps_state!= GPS_STATUS_ON_2D_FIX)||(g_trk_var.g_trk_content_info.gps_info.gps_state!= GPS_STATUS_ON_3D_FIX))
	//return;

	//if(g_trk_var.g_trk_content_info.gps_info.speed_km==0)
	//return;

	//onoff
	//if( g_trk_var.g_trk_settings.gps_speed_alarm_setting[0][4] == 0 && 
	//g_trk_var.g_trk_settings.gps_speed_alarm_setting[1][4] == 0 && 
	//g_trk_var.g_trk_settings.gps_speed_alarm_setting[2][4] == 0 && 
	//g_trk_var.g_trk_settings.gps_speed_alarm_setting[3][4] == 0 && 
	//g_trk_var.g_trk_settings.gps_speed_alarm_setting[4][4] == 0) 
	//{
	//printf("check_speed_alarm alarm off\r\n");
	//return;
	//}

	//speed threshold
	if((g_trk_var.g_trk_settings.gps_speed_alarm_setting[0][6]==0) && 
	(g_trk_var.g_trk_settings.gps_speed_alarm_setting[1][6]==0) && 
	(g_trk_var.g_trk_settings.gps_speed_alarm_setting[2][6]==0) && 
	(g_trk_var.g_trk_settings.gps_speed_alarm_setting[3][6]==0) && 
	(g_trk_var.g_trk_settings.gps_speed_alarm_setting[4][6] ==0))
	{
	maintask_printf("check_speed_alarm threshold 0\r\n");
	return;
	}

	maintask_printf(" check_speed_alarm threshold %d,%d,%d,%d,%d\r\n",
	g_trk_var.g_trk_settings.gps_speed_alarm_setting[0][6],
	g_trk_var.g_trk_settings.gps_speed_alarm_setting[1][6],
	g_trk_var.g_trk_settings.gps_speed_alarm_setting[2][6],
	g_trk_var.g_trk_settings.gps_speed_alarm_setting[3][6],
	g_trk_var.g_trk_settings.gps_speed_alarm_setting[4][6]);
	if(( g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][7] == 0 && TRUE == current_speed_less( g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][6])) ||
	( g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][7] == 1 && TRUE == current_speed_more( g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][6])))
	{
	rr_channel = g_trk_var.g_user_settings.data_transfer_channel;                
	g_trk_var.g_trk_status.alarm_type=g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][2];
	tracker_protocol_send_deal(REPLY_HEADER_ALARM_TRIGGERED,rr_channel);
	}
	#if 0
	for(i=0;i<5;i++)
	{
		//cooldown time 没过
		if(speed_alarm_time_lock[i] == 1)
		continue;
		//用算法，多判断几个速度	 
		//if(( g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][7] == 0 && TRUE == current_speed_less( g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][6])) ||
		//( g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][7] == 1 && TRUE == current_speed_more( g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][6])))
		if(1)
		{
			//rr_channel = g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][3];	             
			//if(rr_channel == 0xff)
			rr_channel = g_trk_var.g_user_settings.data_transfer_channel;                
			g_trk_var.g_trk_status.alarm_type=g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][2];
			tracker_protocol_send_deal(REPLY_HEADER_ALARM_TRIGGERED,rr_channel);
			//cooldown time
			speed_alarm_time_lock[i] = 1;
			switch(i)
			{
			case 0:
			if( g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][0] > 0)
			{
			stopMyTimer(MY_TIMER_56);
			startMyTimer(MY_TIMER_56,  g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][0]*60*1000, speed_alarm_0_time_unlock);
			}   
			break;

			case 1:
			if( g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][0] > 0)
			{
			stopMyTimer(MY_TIMER_57);
			startMyTimer(MY_TIMER_57,  g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][0]*60*1000, speed_alarm_1_time_unlock);
			}   
			break;

			case 2:
			if( g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][0] > 0)
			{
			stopMyTimer(MY_TIMER_58);
			startMyTimer(MY_TIMER_58,  g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][0]*60*1000, speed_alarm_2_time_unlock);
			}   
			break;

			case 3:
			if( g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][0] > 0)
			{
			stopMyTimer(MY_TIMER_59);
			startMyTimer(MY_TIMER_59,  g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][0]*60*1000, speed_alarm_3_time_unlock);
			}   
			break;

			case 4:
			if( g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][0] > 0)
			{
			stopMyTimer(MY_TIMER_60);
			startMyTimer(MY_TIMER_60,  g_trk_var.g_trk_settings.gps_speed_alarm_setting[i][0]*60*1000, speed_alarm_4_time_unlock);
			}   
			break;

			default:
			break;
			}  	  
		} 
	}   
	#endif
}

void check_battery_alarm_start(void)
{
	U8 i=0;
	U8 bat_level = 0;
	U8 bat_level_persentage = 0;
	U8 reply[100];
	int length=0;
	//check_if_send_battery_status();
	//onoff,没开默认10分钟看一次
	maintask_printf("check_battery_alarm_start\r\n");
	if(g_trk_var.g_trk_settings.gps_battery_alarm_setting[0][4] == 0 && 
	g_trk_var.g_trk_settings.gps_battery_alarm_setting[1][4] == 0 && 
	g_trk_var.g_trk_settings.gps_battery_alarm_setting[2][4] == 0 && 
	g_trk_var.g_trk_settings.gps_battery_alarm_setting[3][4] == 0 && 
	g_trk_var.g_trk_settings.gps_battery_alarm_setting[4][4] == 0) 
	{
	return;
	}
	//充电中不发任何低电报警
	maintask_printf("check_battery_alarm_start %d\r\n",is_charging());
	if(is_charging() == TRUE)
	{
	return;
	}
	//battery threshold
	if(g_trk_var.g_trk_settings.gps_battery_alarm_setting[0][6] == 0 && 
	g_trk_var.g_trk_settings.gps_battery_alarm_setting[1][6] == 0 && 
	g_trk_var.g_trk_settings.gps_battery_alarm_setting[2][6] == 0 && 
	g_trk_var.g_trk_settings.gps_battery_alarm_setting[3][6] == 0 && 
	g_trk_var.g_trk_settings.gps_battery_alarm_setting[4][6] == 0) 
	{
	return;
	}
	bat_level = GetBatteryLevel();
	if(bat_level == 0) 
	{	
	bat_level_persentage = 0;
	}
	else if(bat_level == 1) 
	{	
	bat_level_persentage = 5;
	}
	else if(bat_level == 2)
	{
	bat_level_persentage = 10;
	}		
	else if(bat_level == 3)
	{
	bat_level_persentage = 35;
	}	
	else if(bat_level == 4)
	{
	bat_level_persentage = 65;
	}
	else
	{
	bat_level_persentage = 100;
	}
	if(bat_level_persentage <= g_trk_var.g_trk_settings.gps_battery_alarm_setting[0][6])
	{
		bat_alarm_sent[0] = TRUE;
		g_trk_var.g_trk_status.alarm_type=g_trk_var.g_trk_settings.gps_battery_alarm_setting[0][2];
		tracker_protocol_send_deal(REPLY_HEADER_ALARM_TRIGGERED, g_trk_var.g_user_settings.data_transfer_channel);
	}
}          

BOOL cordinates_in_fence(double lat,double lon,double fence_lat1,double fence_lon1,double fence_lat2,double fence_lon2)
{
	double abc=0;
	//先排序，确保fence坐标从小到大
	if(fence_lat1 > fence_lat2)
	{
		abc =  fence_lat1;
		fence_lat1 = fence_lat2;
		fence_lat2 = abc;
	}
	
	if(fence_lon1 > fence_lon2)
	{
		 abc = fence_lon1;
		 fence_lon1 = fence_lon2;
		 fence_lon2 = abc;
	}
	
	if(lat >= fence_lat1 && lat <= fence_lat2 && lon >= fence_lon1 && lon <= fence_lon2)
	{
		maintask_printf("cordinates_in_fence KAL_TRUE!");
		return TRUE;
	}
	else
	{
		maintask_printf("cordinates_in_fence KAL_FALSE!");
		return FALSE;
	}
}					 	

void check_fence_alarm(void) 	
{
	static double Olat1[5];
	static double Olon1[5]; 
	static double Olat2[5];
	static double Olon2[5];
	U8 dd=0;
	U8 old=0;
	U8 IN=0;	 
	U8 i=0;
	char log[100];
	reply_channel rr_channel;

	BOOL send = FALSE;
	maintask_printf("check_fence_alarmSS\r\n");
	if(g_trk_var.g_trk_content_info.gps_info.gps_state!= GPS_STATUS_ON_3D_FIX)
	{
		maintask_printf("check_fence_alarm no fix, return\r\n");	
		return;
	}  
	for(i=0;i<5;i++)
	{
		//cooldown time 没过
		if(fence_alarm_time_lock[i] == 1)
		{
			maintask_printf("check_fence_alarm %d:lock, continue\r\n",i);	
			continue;
		}	
		if(g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][4] == 1)
		{
			double lat1 = atof(&(g_trk_var.g_trk_settings.gps_fance_cordinates[4*i][0]));
			double lon1 = atof(&(g_trk_var.g_trk_settings.gps_fance_cordinates[4*i+1][0]));
			double lat2 = atof(&(g_trk_var.g_trk_settings.gps_fance_cordinates[4*i+2][0]));
			double lon2 = atof(&(g_trk_var.g_trk_settings.gps_fance_cordinates[4*i+3][0]));

			send = FALSE;			
			dd=	g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][6];
			old=OLD[i];

			IN=0;
			if (cordinates_in_fence((double)g_trk_var.g_trk_content_info.gps_info.latitude / 1000000,(double)g_trk_var.g_trk_content_info.gps_info.longitude / 1000000,lat1,lon1,lat2,lon2) == TRUE)
			IN=1;		
			maintask_printf("check_fence_alarm Fence = %d, IN = %d, OLD=%d\r\n" ,i,IN,old);	
			if(IN==1)//在fence里面
			{		
				//前面也是里面，continue				 
				if (old==1)
				continue;
				else //前面是外面
				{
				//direction, 0:out to in, 1:in to out, 2:both
				if ((dd == 0 || dd>1) && old != 0xFF)
				{	
				send = TRUE;//没问题，发出去
				maintask_printf("#++++++++++++  OUT to IN Alarm = %d ++++++++++++\r\n", i);
				}
				OLD[i]=1;//把现在的置old，1				 	
				}	
			}
			else
			{				
				if (old==0)
				continue;
				else {
				if ((dd == 1 || dd>1)&& old !=0xFF) 
				{
				send = TRUE;
				maintask_printf("---------------  IN to OUT Alarm = %d -------------\r\n", i);
				}
				OLD[i]=0;					
				}
			}
			if(send == TRUE)
			{
				do_cordinates_compression((double)g_trk_var.g_trk_content_info.gps_info.latitude / 1000000,(double)g_trk_var.g_trk_content_info.gps_info.longitude / 1000000,(double)g_trk_var.g_trk_content_info.gps_info.speed_km / 1000,COMPRESSION_1);//bcd
				rr_channel = g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][3];
				if(rr_channel == 0xff)
				rr_channel = g_trk_var.g_user_settings.data_transfer_channel;
				g_trk_var.g_trk_status.alarm_type= g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][2];
				tracker_protocol_send_deal(REPLY_HEADER_ALARM_TRIGGERED,rr_channel);
				//cooldown time
				fence_alarm_time_lock[i] = 1;
				switch(i)
				{
					case 0:
					if(g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][0] > 0)
					{
					stopMyTimer(MY_TIMER_63);
					startMyTimer(MY_TIMER_63, g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][0]*60*1000, fence_alarm_0_time_unlock);
					}   
					break;

					case 1:
					if(g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][0] > 0)
					{
					stopMyTimer(MY_TIMER_64);
					startMyTimer(MY_TIMER_64, g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][0]*60*1000, fence_alarm_1_time_unlock);
					}   
					break;

					case 2:
					if(g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][0] > 0)
					{
					stopMyTimer(MY_TIMER_65);
					startMyTimer(MY_TIMER_65, g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][0]*60*1000, fence_alarm_2_time_unlock);
					}   
					break;

					case 3:
					if(g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][0] > 0)
					{
					stopMyTimer(MY_TIMER_66);
					startMyTimer(MY_TIMER_66, g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][0]*60*1000, fence_alarm_3_time_unlock);
					}   
					break;

					case 4:
					if(g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][0] > 0)
					{
					stopMyTimer(MY_TIMER_67);
					startMyTimer(MY_TIMER_67, g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][0]*60*1000, fence_alarm_4_time_unlock);
					}   
					break;
					default:
					break;
				}  	  
			}
		}
	} 
}	

void fence_speed_operation_reset(void)
{
    	fence_speed_moving_fix = 0;
    	fence_speed_moving_no_fix = 0;
    	fence_speed_indoor_times = 0;
  	stopMyTimer(MINI_MY_TIMER_9);
  	GPS_FENCE_SPEED_FIXED = FALSE;
  	stopMyTimer(MY_TIMER_51);
  	//关中断
  	#ifdef SUPPORT_MY_GSENSOR
 	 fence_speed_alarm_turnoff_gsensor();
  	CheckTurnOnOffGsensor();
	#endif
} 

U8 get_indoor_max_sat_number(void)
{
	U8 max_sat = g_trk_var.g_trk_settings.gps_search_control[5];
	if(max_sat == 0)
	max_sat = 2;
	maintask_printf("get_indoor_max_sat_number %d",max_sat);
	return 	max_sat;
}


U8 get_indoor_max_sat_signal(void)
{
	U8 max_signal = g_trk_var.g_trk_settings.gps_search_control[6];
	if(max_signal == 0)
	max_signal = 20;
  	maintask_printf("get_indoor_max_sat_signal %d",max_signal);			
	return 	max_signal;
}				

void smart_gps_fence_speed_wait(void)
{
	int uart_onoff_total_sec =  get_gps_onoff_total_sec();
	if((TRUE == gps_2d_3d_fix_with_hdop_and_fix_sec(10)) /*&& ((GN_GPS_2D_fix+GN_GPS_3D_fix)*uart_onoff_total_sec) >= gps_wait_sec*/)
		{
			maintask_printf("smart_gps_fence_speed_wait:FIX!");
			check_speed_alarm();
			check_fence_alarm();
			if(g_trk_var.g_trk_content_info.gps_info.speed_km > 10)//有速度认为在动，不需要加速度传感器
			{
			// 关gps
			FENCE_OR_SPEED_COMMAND_TURN_OFF_GPS();
			//当成动了
			fence_speed_moving_fix = 0;
			fence_speed_moving_no_fix = 0;
			}
			else
			{
			// 关gps
			FENCE_OR_SPEED_COMMAND_TURN_OFF_GPS();
			fence_speed_moving_fix++;
			fence_speed_moving_no_fix = 0;
			}
			//开中断
			#ifdef SUPPORT_MY_GSENSOR
			fence_speed_alarm_turnon_gsensor();
			CheckTurnOnOffGsensor();
			#endif
			GPS_FENCE_SPEED_FIXED = TRUE;   	   
		}
	else
		{
			maintask_printf("smart_gps_fence_speed_wait:NO FIX!");
			stopMyTimer(MINI_MY_TIMER_9);
			startMyTimer(MINI_MY_TIMER_9, 2000, smart_gps_fence_speed_wait);
		}	

}

//fence和speed是同一个函数，1分钟check一次
void check_fence_or_speed_start(void)
{
	//5个fence,5个speed,都记录在4里面
	if((g_trk_var.g_trk_settings.gps_fance_alarm_setting[0][4] == 0 &&
		  g_trk_var.g_trk_settings.gps_fance_alarm_setting[1][4] == 0 &&
		  g_trk_var.g_trk_settings.gps_fance_alarm_setting[2][4] == 0 &&
		  g_trk_var.g_trk_settings.gps_fance_alarm_setting[3][4] == 0 &&
		  g_trk_var.g_trk_settings.gps_fance_alarm_setting[4][4] == 0 )
		  && 
		  ( g_trk_var.g_trk_settings.gps_speed_alarm_setting[0][4] == 0 &&
		    g_trk_var.g_trk_settings.gps_speed_alarm_setting[1][4] == 0 &&
		    g_trk_var.g_trk_settings.gps_speed_alarm_setting[2][4] == 0 &&
		    g_trk_var.g_trk_settings.gps_speed_alarm_setting[3][4] == 0 &&
		    g_trk_var.g_trk_settings.gps_speed_alarm_setting[4][4] == 0 ))
		{
			 maintask_printf("check_fence_or_speed_stop!!!");
			 fence_speed_operation_reset();
			 FENCE_OR_SPEED_COMMAND_TURN_OFF_GPS();
		}
		else
		{			 
			  U8 fence_speed_check_sec = get_fence_speed_checking_timer_in_sec();  
			  maintask_printf("check_fence_or_speed_start fence_speed_moving_fix=%d,fence_speed_moving_no_fix=%d",fence_speed_moving_fix,fence_speed_moving_no_fix);
	     		 //fence and speed, 保持gps常开，除非静止了，5分钟后关 
	     		  if(fence_speed_moving_fix > 0 || fence_speed_moving_no_fix > 6)//3分钟
				{
					maintask_printf("turnoff gps,wait!");
					FENCE_OR_SPEED_COMMAND_TURN_OFF_GPS();
					//关掉smart_gps_fence_speed_wait的timer
					stopMyTimer(MINI_MY_TIMER_9);
					#ifdef SUPPORT_MY_GSENSOR
					fence_speed_alarm_turnon_gsensor();
					CheckTurnOnOffGsensor();
					#endif
				}
		    	else
				{
					if(GPS_FENCE_SPEED_FIXED == FALSE)
					{
						BOOL in_door = FALSE;
						U8 max_indoor_sat_number = get_indoor_max_sat_number();
						U8 max_indoor_sat_signal = get_indoor_max_sat_signal();

					//如果室内在动，信号很差，开1分钟关2分钟
						if( /*(fence_speed_moving_fix == 0 && fence_speed_moving_no_fix == 0) && *///之前动了
						(gps_moving_in_last_min == TRUE) &&
						(Is_fence_speed_gps_on() == TRUE && Is_gps_searching() == TRUE && g_trk_var.g_trk_content_info.gps_info.satellite_visable_num <= max_indoor_sat_number && g_trk_var.g_trk_content_info.gps_info.max_snr< max_indoor_sat_signal)) //搜星状态，没fix
						{
							fence_speed_indoor_times++;
							if(fence_speed_indoor_times == 2)
							{	
							in_door = TRUE;
							fence_speed_indoor_times = 0;
							}    
						}
						else
						{
							fence_speed_indoor_times = 0;
						}	  	
						fence_speed_moving_fix = 0;
						fence_speed_moving_no_fix++;
						if((fence_speed_moving_no_fix == 3 && Is_fence_speed_gps_on() == TRUE && Is_gps_searching() == TRUE && g_trk_var.g_trk_content_info.gps_info.satellite_visable_num == 0 && g_trk_var.g_trk_content_info.gps_info.max_snr == 0) //搜星状态，没fix,全0
						)
						{
						maintask_printf("make fence_speed_moving_no_fix = 7");
						fence_speed_moving_no_fix = 7;
						}

						if(in_door == TRUE)
						{
							maintask_printf("FENCE SPEED INDOOR MOVING !!! STOP 5 MIN");
							fence_speed_operation_reset();
							FENCE_OR_SPEED_COMMAND_TURN_OFF_GPS();
							stopMyTimer(MINI_MY_TIMER_11);
							if(gps_driving_mode == TRUE)
								{	
									startMyTimer(MINI_MY_TIMER_11, 2*60*1000, check_fence_or_speed_start);
								}  
							else
								{	
								}    
							return;
						}
						else
						{
							maintask_printf("FENCE SPEED OUTDOOR MOVING !!! DO NOTHING");
						}	 	  
					}	 	
					GPS_FENCE_SPEED_FIXED = FALSE;
					FENCE_OR_SPEED_COMMAND_TURN_ON_GPS();
					//开中断，否则如果一直没定位动也没用了
					#ifdef SUPPORT_MY_GSENSOR
					fence_speed_alarm_turnon_gsensor();
					CheckTurnOnOffGsensor();
					#endif
					stopMyTimer(MINI_MY_TIMER_9);
					startMyTimer(MINI_MY_TIMER_9, 2000, smart_gps_fence_speed_wait); 
				}	   	
	        //30s看一次
	        		stopMyTimer(MY_TIMER_51);
	          		startMyTimer(MY_TIMER_51, fence_speed_check_sec*1000, check_fence_or_speed_start);	        
	    }  		 
}	
   	  	 		  
void check_smart_power_saving_setting(void)
{
	U8 smart_setting = g_trk_var.g_user_settings.smart_save_power_enable;
	maintask_printf("check_smart_power_saving_setting %x",smart_setting);
	if(bit_of_byte(smart_setting,7) == 0)//最高位为0，不管怎样全都开
	{
		smart_accelerator = TRUE;
		smart_gps = FALSE;//KAL_TRUE;  默认gps定到位不关，防止漂移
		smart_gsm = TRUE;
		default_glp_on = TRUE;//开gps就直接开glp
		tracking_high_speed_turnoff_gps = TRUE;//超过30km/h关gps
		maintask_printf("check_smart_power_saving_setting 1 all on!");
	}
	else //最高位为1
	{
		//第0位，最低位为smart_accelerator
		if(bit_of_byte(smart_setting,0) == 0)
		{
		maintask_printf("smart_accelerator = KAL_FALSE;");
		smart_accelerator = FALSE;
		}
		else
		{
		maintask_printf("smart_accelerator = KAL_TRUE;");
		smart_accelerator = TRUE;
		}

		//第1位为smart_gps
		if(bit_of_byte(smart_setting,1) == 0)
		{
		maintask_printf("smart_gps = KAL_FALSE;");
		smart_gps = FALSE;
		}
		else
		{
		maintask_printf("smart_gps = KAL_TRUE;");
		smart_gps = TRUE;
		}
		//第2位为smart_gsm
		if(bit_of_byte(smart_setting,2) == 0)
		{
		maintask_printf("smart_gsm = KAL_FALSE;");
		smart_gsm = FALSE;
		}
		else
		{
		maintask_printf("smart_gsm = KAL_TRUE;");
		smart_gsm = TRUE;
		}	

		//第3位为default_glp
		if(bit_of_byte(smart_setting,3) == 0)
		{
		maintask_printf("default_glp_on = KAL_FALSE;");
		default_glp_on = FALSE;
		}
		else
		{
		maintask_printf("default_glp_on = KAL_TRUE;");
		default_glp_on = TRUE;
		}		

		//第4位tracking_high_speed_turnoff_gps
		if(bit_of_byte(smart_setting,4) == 0)
		{
		maintask_printf("tracking_high_speed_turnoff_gps = KAL_FALSE;");
		tracking_high_speed_turnoff_gps = FALSE;
		}
		else
		{
		maintask_printf("tracking_high_speed_turnoff_gps = KAL_TRUE;");
		tracking_high_speed_turnoff_gps = TRUE;
		}			  	  
	}	  	  	  						
}

BOOL Is_smart_gps(void)
{
	if(smart_gps == TRUE)
	{
		maintask_printf("Is_smart_gps KAL_TRUE;");
		return TRUE;
	}
	else
	{
		maintask_printf("Is_smart_gps KAL_FALSE;");
		return FALSE;
	}
}

BOOL Is_smart_gsm(void)
{
	if(smart_gsm == TRUE)
	{
		maintask_printf("Is_smart_gsm KAL_TRUE;");
		return TRUE;
	}
	else
	{
		maintask_printf("Is_smart_gsm KAL_FALSE;");
		return FALSE;
	}
}

void xmode_gps_off_after_fix_check(void)
{
	  int uart_onoff_total_sec =  get_gps_onoff_total_sec();
  	  if(gps_2d_3d_fix_with_hdop_and_fix_sec(30))//发不了就得等1分钟了,最多1分钟
	  	{	
	  		maintask_printf("xmode_gps_off_after_fix_check go XMODE_COMMAND_TURN_OFF_GPS\r\n");
	  	    // 关gps
	  	      XMODE_COMMAND_TURN_OFF_GPS();
	  	    //开中断
	  	   //  xmode_turnon_gsensor();
	  	  //   CheckTurnOnOffGsensor();
	  	}
  	else
	  	{
	  		  maintask_printf("xmode_gps_off_after_fix_check wait\r\n");	  
	  		  stopMyTimer(MINI_MY_TIMER_15);
	  		  startMyTimer(MINI_MY_TIMER_15, 2000, xmode_gps_off_after_fix_check);
	  	}
}  		  

void xmode_gps_no_fix_process(void)
{
	maintask_printf("xmode_gps_no_fix_process\r\n");
	gps_back_up_push_no_fix();
	//check_if_send_wifi();
	if(TRUE == gps_xmode_send())
	{
	 gps_xmode_sent = TRUE;
	}
	else
	{
	 gps_xmode_sent = FALSE;
	}
	fix_sending_times = 0;//本次没fix，置0
	no_fix_sending++;
	maintask_printf("after 1min,send 0 and stop tracking...\r\n");
	stopMyTimer(MY_TIMER_33);//此处应该关掉gps_fix_wait的timer，全停
	stopMyTimer(MINI_MY_TIMER_14);//check 1min的timer
	stopMyTimer(MINI_MY_TIMER_15);//定到位看什么时候关gps的timer	
	XMODE_COMMAND_TURN_OFF_GPS();//此处必须关gps，否则没地方关
	#ifdef SUPPORT_MY_GSENSOR
	xmode_turnon_gsensor();
	CheckTurnOnOffGsensor();
	#endif
}	
	 	
void gps_fix_wait(void)
{
   if(TRUE == gps_2d_3d_fix_with_hdop_and_fix_sec(10))
	{
		maintask_printf("gps_fix_wait:FIX!");
		gps_back_up_push_fixed();//压栈，保存至backup，发的时候再从backup里面取
		//重新校准fence那边，防止两边撞一起，费电
		if(TRUE == fence_or_speed_alarm_on())
		{
		U8 fence_speed_check_sec = get_fence_speed_checking_timer_in_sec();
		maintask_printf("gps_fix_wait:check fence speed");
		//防止跟那边在同一秒撞上
		check_speed_alarm(); //speed 可以
		check_fence_alarm();
		stopMyTimer(MINI_MY_TIMER_11);//这是fence speed 休眠5分钟的timer，这里要关
		stopMyTimer(MY_TIMER_51);
		startMyTimer(MY_TIMER_51, fence_speed_check_sec*1000, check_fence_or_speed_start);	
		}   

		if(TRUE == gps_xmode_send())
		{
			maintask_printf("gps_fix_wait:gps_xmode_sent = KAL_TRUE;!");
			gps_xmode_sent = TRUE;

			if(g_trk_var.g_trk_content_info.gps_info.speed_km> 10)//有速度认为在动，不需要加速度传感器
			{
			//当成动了
			fix_sending_times=0;
			no_fix_sending=0;
			}
			else
			{
			fix_sending_times++;
			no_fix_sending=0;
			}       	  
		}
		else
		{
			maintask_printf("gps_fix_wait:gps_xmode_sent = KAL_FALSE!");
			gps_xmode_sent = FALSE;
		}
		//gps要保持一段时间
		if(Is_smart_gps() == TRUE)//1
			{	
				xmode_gps_off_after_fix_check();
			}
		else//smart_gps = KAL_FALSE
			{
				if(tracking_high_speed_turnoff_gps == TRUE)	
				{
				 U8 speed_off = get_speed_for_turnoff_gps_in_tracking();
				 
				 if(g_trk_var.g_trk_content_info.gps_info.speed_km > speed_off)
					 {
					    xmode_gps_off_after_fix_check();
					 }
				}
			}	  	 			
			gps_xmode_fixed = TRUE;//不管有没有真发，这里都必须认为是真发了，否则那边会认为没法压一个no fix 进去  
			//开中断,此处必须开，否则fix_sending_times++了，之前gsensor被关了，没法跑中断，下次再检测tracking_with_measurement_in_min就会把gps关掉
			#ifdef SUPPORT_MY_GSENSOR
			xmode_turnon_gsensor();
			CheckTurnOnOffGsensor();   
			#endif
		}
	else
	 	{
	 	  //没fix，10s看一次
	 	  maintask_printf("gps_fix_wait:NO GPS FIX!"); 
		  gps_xmode_fixed = FALSE;
	 	  stopMyTimer(MY_TIMER_33);
		  startMyTimer(MY_TIMER_33, 2000, gps_fix_wait);
	 }
}	

void xmode_send_zero_every_1_min(void)
{
	send_zero_count++;
	maintask_printf("xmode_send_zero_every_1_min %d --- %d",send_zero_count,send_zero_total);
	gps_back_up_push_no_fix();
	//check_if_send_wifi();
	if(TRUE == gps_xmode_send())
	{
		maintask_printf("push 0, gps_xmode_sent = KAL_TRUE");
		gps_xmode_sent = TRUE;
	}	
	else
	{
		maintask_printf("push 0, gps_xmode_sent = KAL_FALSE");
		gps_xmode_sent = FALSE;
	}
	if(send_zero_count >= send_zero_total)
	{
		stopMyTimer(MINI_MY_TIMER_10);
		XmodeStart();
	}	 	
	else
	{	
		stopMyTimer(MINI_MY_TIMER_10);
		startMyTimer(MINI_MY_TIMER_10, 60*1000, xmode_send_zero_every_1_min);
	}  
}

void xmode_wait_for_minutes_before_restart(U8 number_min)
{
	maintask_printf("xmode_send_zero_for_minutes_every_1_min %d",number_min);
	send_zero_total = number_min;
	send_zero_count = 0;
	stopMyTimer(MINI_MY_TIMER_10);
	startMyTimer(MINI_MY_TIMER_10, 60*1000, xmode_send_zero_every_1_min);
}
	
void xmode_start_gps_timer_check_in_minutes(void)
{
	maintask_printf("xmode_start_gps_timer_check_in_minutes timer:%d, min,fix_sending_times:%d,no_fix_sending:%d",timer_minutes,fix_sending_times,no_fix_sending);
	if(fix_sending_times > 0 || no_fix_sending > 3)//4分钟
		{
			if(gps_xmode_sent == TRUE)//之前发过了
			{
				maintask_printf("gps_xmode_sent == KAL_TRUE, stop tracking and return...");
				stopMyTimer(MY_TIMER_33);//此处应该关掉gps_fix_wait的timer，全停
				stopMyTimer(MINI_MY_TIMER_15);//关掉gps定位后多久关gps的timer
				XMODE_COMMAND_TURN_OFF_GPS();//此处必须关gps，否则没地方关
				#ifdef SUPPORT_MY_GSENSOR
				xmode_turnon_gsensor();
				CheckTurnOnOffGsensor();
				#endif
				stopMyTimer(MY_TIMER_31);
				startMyTimer(MY_TIMER_31, timer_minutes*60*1000, xmode_start_gps_timer_check_in_minutes);
			} 
		else//之前没发过
			{
				maintask_printf("gps_xmode_sent == KAL_FALSE, push 0 and wait...");
				xmode_gps_no_fix_process();
				stopMyTimer(MY_TIMER_31);
				startMyTimer(MY_TIMER_31, timer_minutes*60*1000, xmode_start_gps_timer_check_in_minutes);
			} 
		return;    
		} 
	//timer内没fix
	if(gps_xmode_fixed == FALSE)
	{
		BOOL in_door = FALSE;
		U8 max_snr = g_trk_var.g_trk_content_info.gps_info.max_snr;
		U8 max_indoor_sat_number = get_indoor_max_sat_number();
		U8 max_indoor_sat_signal = get_indoor_max_sat_signal();
		if(gps_moving_in_last_min == TRUE)
		{	
		maintask_printf("GPSDW_sate_number= %d, g_trk_var.g_trk_content_info.gps_info.max_snr=%d, gps_moving_in_last_min=KAL_TRUE",g_trk_var.g_trk_content_info.gps_info.satellite_visable_num,max_snr);
		}
		else
		{
		maintask_printf("GPSDW_sate_number= %d, g_trk_var.g_trk_content_info.gps_info.max_snr=%d, gps_moving_in_last_min=KAL_FALSE",g_trk_var.g_trk_content_info.gps_info.satellite_visable_num,max_snr);
		}  

	if(gps_driving_mode == TRUE)
		{	
		maintask_printf("gps_driving_mode=KAL_TRUE");
		}
	else
		{
		maintask_printf("gps_driving_mode=KAL_FALSE");
		} 	

	//如果室内在动，信号很差，开1分钟关2分钟
	if( (fix_sending_times == 0 && no_fix_sending == 0) && //之前动了
	(gps_moving_in_last_min == TRUE) &&
	(Is_xmode_gps_on() == TRUE && Is_gps_searching() == TRUE && g_trk_var.g_trk_content_info.gps_info.satellite_visable_num <= max_indoor_sat_number && max_snr < max_indoor_sat_signal)) //搜星状态，没fix
		{
		in_door = TRUE;
		maintask_printf("in_door = TRUE");
		}	
	gps_back_up_push_no_fix();
	//check_if_send_wifi();
	if(TRUE == gps_xmode_send())
	{
		maintask_printf("push 0, gps_xmode_sent = KAL_TRUE");
		gps_xmode_sent = TRUE;
		fix_sending_times = 0;//本次没fix，置0
		no_fix_sending++;	
	}	
	else
	{
		maintask_printf("push 0, gps_xmode_sent = KAL_FALSE");
		gps_xmode_sent = FALSE;
	}
	if((no_fix_sending == 2 && Is_xmode_gps_on() == TRUE && Is_gps_searching() == TRUE && g_trk_var.g_trk_content_info.gps_info.satellite_visable_num == 0 && max_snr  == 0) //搜星状态，没fix,全0
	)
	{
		maintask_printf("make no_fix_sending = 4");
		no_fix_sending = 4;
		maintask_printf("stop tracking and return...");
		stopMyTimer(MY_TIMER_33);//此处应该关掉gps_fix_wait的timer，全停
		stopMyTimer(MINI_MY_TIMER_15);//关掉gps定位后多久关gps的timer
		XMODE_COMMAND_TURN_OFF_GPS();//此处必须关gps，否则没地方关
		#ifdef SUPPORT_MY_GSENSOR
		xmode_turnon_gsensor();
		CheckTurnOnOffGsensor();
		#endif
		stopMyTimer(MY_TIMER_31);
		startMyTimer(MY_TIMER_31, timer_minutes*60*1000, xmode_start_gps_timer_check_in_minutes);
		return;
	}
	if(in_door == TRUE)
		{
			if(g_trk_var.g_trk_content_info.gps_info.satellite_visable_num == 0 && max_snr == 0)
				{
					maintask_printf("XMODE INDOOR MOVING, NOTHING SATELLITE !!! STOP 5 MIN");	 
					//关所有
					xmode_start_sending_timer(0);
					if(gps_driving_mode == TRUE)
					{	
						xmode_wait_for_minutes_before_restart(2);//2分钟
					}
					else
					{
					}	  	   
				}
			else
				{
					maintask_printf("XMODE INDOOR MOVING, SOME LOW SATELLITE !!! STOP 2 MIN");	 
					//关所有
					xmode_start_sending_timer(0);
					xmode_wait_for_minutes_before_restart(2);//分钟
				}
			return;
		}
		else
		{ 
			maintask_printf("XMODE OUTDOOR MOVING !!! DO NOTHING");	
		}	     
	}
	else//fix 发过了
	{
	//gps_fix_wait那边已经做完了
	}	  	
	//重来一遍	
	gps_xmode_fixed = FALSE;	
	XMODE_COMMAND_TURN_ON_GPS();
	//开中断，否则如果一直没定位动也没用了
	#ifdef SUPPORT_MY_GSENSOR
	xmode_turnon_gsensor();
	CheckTurnOnOffGsensor();
	#endif
	stopMyTimer(MINI_MY_TIMER_15);//关掉gps定位后多久关gps的timer
	stopMyTimer(MY_TIMER_31);
	startMyTimer(MY_TIMER_31, timer_minutes*60*1000, xmode_start_gps_timer_check_in_minutes);
	stopMyTimer(MY_TIMER_33);
	startMyTimer(MY_TIMER_33, 2000, gps_fix_wait);
} 

void gps_fix_wait_more_than_1_min(void)
{
	if(TRUE == gps_2d_3d_fix_with_hdop_and_fix_sec(10))
	{
		maintask_printf("gps_fix_wait_more_than_1_min:FIX!\r\n");
		gps_back_up_push_fixed();//压栈，保存至backup，发的时候再从backup里面取
		//重新校准fence那边，防止两边撞一起，费电
		if(TRUE == fence_or_speed_alarm_on())
		{
			U8 fence_speed_check_sec = get_fence_speed_checking_timer_in_sec();
			maintask_printf("gps_fix_wait:check fence speed");
			//防止跟那边在同一秒撞上
			check_speed_alarm(); //speed 可以
			check_fence_alarm();
			stopMyTimer(MINI_MY_TIMER_11);//这是fence speed 休眠5分钟的timer，这里要关
			stopMyTimer(MY_TIMER_51);
			startMyTimer(MY_TIMER_51, fence_speed_check_sec*1000, check_fence_or_speed_start);    
		}   
		if(TRUE == gps_xmode_send())
		{
			gps_xmode_sent = TRUE;
			maintask_printf("gps_fix_wait_more_than_1_min:gps_xmode_sent = KAL_TRUE!");
			if(g_trk_var.g_trk_content_info.gps_info.speed_km > 10)//有速度认为在动，不需要加速度传感器
			{
				//当成动了
				fix_sending_times=0;
				no_fix_sending=0;
			}
			else
			{
				fix_sending_times++;
				no_fix_sending=0;
			}   
		}
		else
		{
			gps_xmode_sent = FALSE;
			maintask_printf("gps_fix_wait_more_than_1_min:gps_xmode_sent = KAL_FALSE!");
		}	  		  
		//gps要保持一段时间
		xmode_gps_off_after_fix_check();
		gps_xmode_fixed = TRUE;//不管有没有真发，这里都必须认为是真发了，否则那边会认为没法压一个no fix 进去  
		//开中断,此处必须开，否则fix_sending_times++了，之前gsensor被关了，没法跑中断，下次再检测tracking_with_measurement_in_min就会把gps关掉
		#ifdef SUPPORT_MY_GSENSOR
		xmode_turnon_gsensor();
		CheckTurnOnOffGsensor();   
		#endif
	}
	else
	{
		//没fix，10s看一次
		maintask_printf("gps_fix_wait:NO GPS FIX!");
		gps_xmode_fixed = FALSE;
		stopMyTimer(MY_TIMER_33);
		startMyTimer(MY_TIMER_33, 2000, gps_fix_wait_more_than_1_min);
	}
}

void xmode_start_gps_timer_check_in_more_than_1_minutes_after_1min(void)
{
	maintask_printf("xmode_start_gps_timer_check_in_more_than_1_minutes_after_1min");
	if(gps_xmode_sent == TRUE)//之前定到位也发出了
	{
		maintask_printf("xmode_start_gps_timer_check_in_more_than_1_minutes_after_1min,gps_xmode_sent == TRUE, do nothing");	
		return;
	}	
	else//没发出去有2种可能，1种是没定到位，一种是定到位了
	{
		if(gps_xmode_fixed == TRUE)//定到了，压栈了，但是多数据没发出去，也什么都不用做
		{
			maintask_printf("xmode_start_gps_timer_check_in_more_than_1_minutes_after_1min,gps_xmode_sent == KAL_FALSE,gps_xmode_fixed == KAL_TRUE, do nothing");	
			return;
		} 
		else//没定到位也没发出去，看信号强度
		{
			U8 max_snr = g_trk_var.g_trk_content_info.gps_info.max_snr;
			U8 max_indoor_sat_number = get_indoor_max_sat_number();
			U8 max_indoor_sat_signal = get_indoor_max_sat_signal();
			if(Is_xmode_gps_on() == TRUE && Is_gps_searching() == TRUE && g_trk_var.g_trk_content_info.gps_info.satellite_visable_num <= max_indoor_sat_number && max_snr < max_indoor_sat_signal) //搜星状态，没fix
			{
				maintask_printf("in_door = KAL_TRUE");
				xmode_gps_no_fix_process();
				return;
			}     	       
			no_fix_1_min_checking++;
			maintask_printf("no_fix_1_min_checking=%d",no_fix_1_min_checking);
			if(no_fix_1_min_checking > 3)//4分钟未定位
			{
				maintask_printf("no_fix_1_min_checking=%d > 3, turn off gps",no_fix_1_min_checking);	
				xmode_gps_no_fix_process();
				return;
			}	
			//其他情况就是还在searching，继续等
			stopMyTimer(MINI_MY_TIMER_14);
			startMyTimer(MINI_MY_TIMER_14, 60000, xmode_start_gps_timer_check_in_more_than_1_minutes_after_1min);	
		}
	}       
}	    		

void xmode_start_gps_timer_check_in_more_than_1_minutes(void)
{
	maintask_printf("xmode_start_gps_timer_check_in_more_than_1_minutes timer:%d, min,fix_sending_times:%d,no_fix_sending:%d",timer_minutes,fix_sending_times,no_fix_sending);
	if(gps_xmode_sent == TRUE)//之前不管定没定发过了
	{
		maintask_printf("xmode_start_gps_timer_check_in_more_than_1_minutes gps_xmode_sent == KAL_TRUE");
		if(fix_sending_times > 0 || no_fix_sending > 0)
			{ 
				maintask_printf("stop tracking and return...");
				stopMyTimer(MY_TIMER_33);//此处应该关掉gps_fix_wait的timer，全停
				stopMyTimer(MINI_MY_TIMER_14);//gps check 1分钟的timer
				stopMyTimer(MINI_MY_TIMER_15);//gps定到位后多久关gps的timer
				XMODE_COMMAND_TURN_OFF_GPS();//此处必须关gps，否则没地方关
				#ifdef SUPPORT_MY_GSENSOR
				xmode_turnon_gsensor();
				CheckTurnOnOffGsensor();
				#endif
				stopMyTimer(MY_TIMER_31);
				startMyTimer(MY_TIMER_31, timer_minutes*60*1000, xmode_start_gps_timer_check_in_more_than_1_minutes);
			}
      		else//之前发了，但是动了，重来
			{
				gps_xmode_sent = FALSE;	
				maintask_printf("restart...");
				XMODE_COMMAND_TURN_ON_GPS();
				//开中断，否则如果一直没定位动也没用了
				#ifdef SUPPORT_MY_GSENSOR
				xmode_turnon_gsensor();
				CheckTurnOnOffGsensor();
				#endif
				stopMyTimer(MINI_MY_TIMER_14);//gps check 1分钟的timer
				stopMyTimer(MINI_MY_TIMER_15);//gps定到位后多久关gps的timer
				stopMyTimer(MY_TIMER_33);
				startMyTimer(MY_TIMER_33, 2000, gps_fix_wait_more_than_1_min); 
				stopMyTimer(MY_TIMER_31);
				startMyTimer(MY_TIMER_31, timer_minutes*60*1000, xmode_start_gps_timer_check_in_more_than_1_minutes);
				no_fix_1_min_checking = 0;
				stopMyTimer(MINI_MY_TIMER_14);
				startMyTimer(MINI_MY_TIMER_14, 60000, xmode_start_gps_timer_check_in_more_than_1_minutes_after_1min);
			}

  	}
  	else//多点，之前没发出去
	{
		maintask_printf("xmode_start_gps_timer_check_in_more_than_1_minutes gps_xmode_sent == KAL_FALSE"); 
		if(fix_sending_times > 0 || no_fix_sending > 0)//没动，压一个0进去
		{ 
			maintask_printf("no moving, stop tracking and return...");
			xmode_gps_no_fix_process();
			stopMyTimer(MY_TIMER_31);
			startMyTimer(MY_TIMER_31, timer_minutes*60*1000, xmode_start_gps_timer_check_in_more_than_1_minutes);
		}
		else//动了，重新开始
		{
			gps_xmode_sent = FALSE;	
			maintask_printf("restart...");
			XMODE_COMMAND_TURN_ON_GPS();
			//开中断，否则如果一直没定位动也没用了
			#ifdef SUPPORT_MY_GSENSOR
			xmode_turnon_gsensor();
			CheckTurnOnOffGsensor();
			#endif
			stopMyTimer(MINI_MY_TIMER_14);//gps check 1分钟的timer
			stopMyTimer(MINI_MY_TIMER_15);//gps定到位后多久关gps的timer
			stopMyTimer(MY_TIMER_33);
			startMyTimer(MY_TIMER_33, 2000, gps_fix_wait_more_than_1_min); 
			stopMyTimer(MY_TIMER_31);
			startMyTimer(MY_TIMER_31, timer_minutes*60*1000, xmode_start_gps_timer_check_in_more_than_1_minutes);
			no_fix_1_min_checking = 0;
			stopMyTimer(MINI_MY_TIMER_14);
			startMyTimer(MINI_MY_TIMER_14, 60000, xmode_start_gps_timer_check_in_more_than_1_minutes_after_1min);
		}
	}
}      

void tracking_with_measurement_fix_wait_more_than_1_min(void)
{
	if(TRUE == gps_2d_3d_fix_with_hdop_and_fix_sec(10))
	{
		maintask_printf("tracking_with_measurement_fix_wait_more_than_1_min:FIX!\r\n");
		gps_back_up_push_fixed();//压栈，保存至backup，发的时候再从backup里面取
		//重新校准fence那边，防止两边撞一起，费电
		if(TRUE == fence_or_speed_alarm_on())
		{
			U8 fence_speed_check_sec = get_fence_speed_checking_timer_in_sec();
			maintask_printf("tracking_with_measurement_fix_wait_more_than_1_min:check fence speed");
			//防止跟那边在同一秒撞上
			check_speed_alarm(); //speed 可以
			check_fence_alarm();
			stopMyTimer(MINI_MY_TIMER_11);//这是fence speed 休眠5分钟的timer，这里要关
			stopMyTimer(MY_TIMER_51);
			startMyTimer(MY_TIMER_51, fence_speed_check_sec*1000, check_fence_or_speed_start);    
		}   
		if(TRUE == gps_xmode_send_with_measuremet())
		{
			tracking_with_measurement_sent = TRUE;
			maintask_printf("gps_fix_wait_more_than_1_min:gps_xmode_sent = KAL_TRUE!");
			if(g_trk_var.g_trk_content_info.gps_info.speed_km>10)//有速度认为在动，不需要加速度传感器
			{
			//当成动了
			fix_sending_times=0;
			no_fix_sending=0;
			}
			else
			{
			fix_sending_times++;
			no_fix_sending=0;
			}  
		}
		else
		{
			tracking_with_measurement_sent = FALSE;
			maintask_printf("gps_fix_wait_more_than_1_min:gps_xmode_sent = KAL_FALSE!");
		}	  		  
		//gps要保持一段时间
		xmode_gps_off_after_fix_check();
		tracking_with_measurement_fixed = TRUE;//不管有没有真发，这里都必须认为是真发了，否则那边会认为没法压一个no fix 进去  
		//开中断,此处必须开，否则fix_sending_times++了，之前gsensor被关了，没法跑中断，下次再检测tracking_with_measurement_in_min就会把gps关掉
		#ifdef SUPPORT_MY_GSENSOR
		xmode_turnon_gsensor();
		CheckTurnOnOffGsensor();
		#endif
	}
	else
	{
		//没fix，10s看一次
		maintask_printf("tracking_with_measurement_fix_wait_more_than_1_min:NO GPS FIX!");
		tracking_with_measurement_fixed = FALSE;
		stopMyTimer(MY_TIMER_62);
		startMyTimer(MY_TIMER_62, 2000, tracking_with_measurement_fix_wait_more_than_1_min);
	}
}

void tracking_with_measurement_in_more_than_1_minutes_after_1min(void)
{
	maintask_printf("tracking_with_measurement_in_more_than_1_minutes_after_1min");
	if(tracking_with_measurement_sent == TRUE)//之前定到位也发出了
	{
		maintask_printf("tracking_with_measurement_in_more_than_1_minutes_after_1min,tracking_with_measurement_sent == TRUE, do nothing");	
		return;
	}	
	else//没发出去有2种可能，1种是没定到位，一种是定到位了
	{
		if(tracking_with_measurement_fixed == TRUE)//定到了，压栈了，但是多数据没发出去，也什么都不用做
		{
			maintask_printf("tracking_with_measurement_in_more_than_1_minutes_after_1min,tracking_with_measurement_sent == KAL_FALSE,tracking_with_measurement_fixed == KAL_TRUE, do nothing");	
			return;
		} 
		else//没定到位也没发出去，看信号强度
		{
			U8 max_snr = g_trk_var.g_trk_content_info.gps_info.max_snr;
			U8 max_indoor_sat_number = get_indoor_max_sat_number();
			U8 max_indoor_sat_signal = get_indoor_max_sat_signal();
			if(Is_xmode_gps_on() == TRUE && Is_gps_searching() == TRUE && g_trk_var.g_trk_content_info.gps_info.satellite_visable_num <= max_indoor_sat_number && max_snr < max_indoor_sat_signal) //搜星状态，没fix
			{
				maintask_printf("in_door = KAL_TRUE");
				tracking_with_measurement_no_fix_process();
				return;
			}     	       
			no_fix_1_min_checking++;
			if(no_fix_1_min_checking > 3)//4分钟未定位
			{
				maintask_printf("no_fix_1_min_checking=%d > 3, turn off gps",no_fix_1_min_checking);	
				tracking_with_measurement_no_fix_process();
				return;
			}	
			//其他情况就是还在searching，继续等
			stopMyTimer(MINI_MY_TIMER_14);
			startMyTimer(MINI_MY_TIMER_14, 60000, tracking_with_measurement_in_more_than_1_minutes_after_1min);	
		}
	}       
}	    		


void tracking_with_measurement_in_more_than_1_min(void)	
{
	maintask_printf("tracking_with_measurement_in_more_than_1_min timer:%d, min,fix_sending_times:%d,no_fix_sending:%d",g_trk_var.g_trk_settings.sample_time,fix_sending_times,no_fix_sending);
	if(tracking_with_measurement_sent == TRUE)//之前不管定没定发过了
	{
		maintask_printf("tracking_with_measurement_in_more_than_1_min tracking_with_measurement_sent == KAL_TRUE");
		if(fix_sending_times > 0 || no_fix_sending > 0)
		{ 
			maintask_printf("stop tracking and return...");
			stopMyTimer(MY_TIMER_62);//此处应该关掉gps_fix_wait的timer，全停
			stopMyTimer(MINI_MY_TIMER_14);//gps check 1分钟的timer
			stopMyTimer(MINI_MY_TIMER_15);//gps定到位后多久关gps的timer
			XMODE_COMMAND_TURN_OFF_GPS();//此处必须关gps，否则没地方关
			#ifdef SUPPORT_MY_GSENSOR
			xmode_turnon_gsensor();
			CheckTurnOnOffGsensor();
			#endif
			stopMyTimer(MY_TIMER_61);
			startMyTimer(MY_TIMER_61, g_trk_var.g_trk_settings.sample_time*60*1000, tracking_with_measurement_in_more_than_1_min);
		}
		else//之前发了，但是动了，重来
		{
			tracking_with_measurement_sent = FALSE;	
			maintask_printf("restart...");
			XMODE_COMMAND_TURN_ON_GPS();
			//开中断，否则如果一直没定位动也没用了
			#ifdef SUPPORT_MY_GSENSOR
			xmode_turnon_gsensor();
			CheckTurnOnOffGsensor();
			#endif
			stopMyTimer(MINI_MY_TIMER_14);//gps check 1分钟的timer
			stopMyTimer(MINI_MY_TIMER_15);//gps定到位后多久关gps的timer
			stopMyTimer(MY_TIMER_62);
			startMyTimer(MY_TIMER_62, 2000, tracking_with_measurement_fix_wait_more_than_1_min); 
			stopMyTimer(MY_TIMER_61);
			startMyTimer(MY_TIMER_61, g_trk_var.g_trk_settings.sample_time*60*1000, tracking_with_measurement_in_more_than_1_min);
			no_fix_1_min_checking = 0;
			stopMyTimer(MINI_MY_TIMER_14);
			startMyTimer(MINI_MY_TIMER_14, 60000, tracking_with_measurement_in_more_than_1_minutes_after_1min);
		}
	}
	else//多点，之前没发出去
	{
		maintask_printf("tracking_with_measurement_in_more_than_1_min tracking_with_measurement_sent == KAL_FALSE"); 
		if(fix_sending_times > 0 || no_fix_sending > 0)//没动，压一个0进去
		{ 
			maintask_printf("no moving, stop tracking and return...");
			tracking_with_measurement_no_fix_process();
			stopMyTimer(MY_TIMER_61);
			startMyTimer(MY_TIMER_61, g_trk_var.g_trk_settings.sample_time*60*1000, tracking_with_measurement_in_more_than_1_min);
		}
		else//动了，重新开始
		{
			tracking_with_measurement_sent = FALSE;	
			maintask_printf("restart...");
			XMODE_COMMAND_TURN_ON_GPS();
			//开中断，否则如果一直没定位动也没用了
			#ifdef SUPPORT_MY_GSENSOR
			xmode_turnon_gsensor();
			CheckTurnOnOffGsensor();
			#endif
			stopMyTimer(MINI_MY_TIMER_14);//gps check 1分钟的timer
			stopMyTimer(MINI_MY_TIMER_15);//gps定到位后多久关gps的timer
			stopMyTimer(MY_TIMER_62);
			startMyTimer(MY_TIMER_62, 2000, tracking_with_measurement_fix_wait_more_than_1_min); 
			stopMyTimer(MY_TIMER_61);
			startMyTimer(MY_TIMER_61, g_trk_var.g_trk_settings.sample_time*60*1000, tracking_with_measurement_in_more_than_1_min);
			no_fix_1_min_checking = 0;
			stopMyTimer(MINI_MY_TIMER_14);
			startMyTimer(MINI_MY_TIMER_14, 60000, tracking_with_measurement_in_more_than_1_minutes_after_1min);
		}
	}
}                   		

/***************************************************************************************
	g_trk_var.g_trk_settings.track_channel - "Tracking Channel" as in the old message.
	g_trk_var.g_trk_settings.sample_time  - "Sample Time (mins)" as in the old message.
	g_trk_var.g_trk_settings.report_time - "Report Time (mins)" as in the old message.
	 g_trk_var.g_trk_settings.location_accuracy  - "Location Accuracy" as in the old message.
	
	g_trk_var.g_trk_settings.track_setting
	1 bit - "Send Device ID" NOT AS IN THE OLD MESSAGE.
	1 bit - "Delta Compression" NOT AS IN THE OLD MESSAGE.
  1 bit - Send Altitude measurement
  1 bit - Send Battery measurement
	1 bit - Send Speed measurement
	1 bit - Sample time in minutes? (if not then it is in seconds)
	2 bits - Padding

********************************************************************/

/********************************************************************

	3 bit - Set to 011 for identification
	1 bit - Sent Device ID
	1 bit - Data Compressed
	1 bit - Sent Speed measurement
1 bit - Sent Altitude measurement
	1 bit - Sent Battery measurement
	4 bits - "Location Accuracy"
1 bit - Sample time in minutes? (if not then it is in seconds)
3 bits - padding (111)
	8 bits - Sample Time
	8 bits - Samples Per Report
	----------------------------------------------------  29 bits so far
	32 bits (optional) - Device Id if requested
	X bits - payload, much the same as now only with option to add other measurements at the end.

********************************************************************/
void creat_compressed_message_with_measuremet(U8 compressed_number, char compressed_type)
{
	U16 i=0;
	U16 j=0;
	U16 actual_n=0;
	U16 num_1 = 0;
	U16 len = 0;
	char compressed_lat[10];
	char compressed_long[10];
	char fixed_compression_buffer[20]; 
	char log[200];
	U8 bit_send_deviceid = bit_of_byte(g_trk_var.g_trk_settings.track_setting,7);
	U8 bit_data_compression = bit_of_byte(g_trk_var.g_trk_settings.track_setting,6);
	U8 bit_send_altitude = bit_of_byte(g_trk_var.g_trk_settings.track_setting,5);
	U8 bit_send_battery = bit_of_byte(g_trk_var.g_trk_settings.track_setting,4);
	U8 bit_send_speed = bit_of_byte(g_trk_var.g_trk_settings.track_setting,3);
	U8 bit_sample_in_min = bit_of_byte(g_trk_var.g_trk_settings.track_setting,2);
	U8 Location_Accuracy =  g_trk_var.g_trk_settings.location_accuracy;
	U8 sample_times = g_trk_var.g_trk_settings.sample_time;
	U8 report_times = g_trk_var.g_trk_settings.report_time;
	maintask_printf("creat_compressed_message_with_measuremet compressed_number=%d,compressed_type=%d\r\n",compressed_number,compressed_type);
	if(compressed_number == 0)
	return;
	actual_n = compressed_number;		
	num_1 = get_number_of_records_in_backup_database();
	if(actual_n > num_1)
	actual_n = num_1;  
	if(actual_n == 0)
	return;
	memset(compressed_message,0,COMPRESSED_MSG_LEN);//2048	
	compressed_message[0] = 0x60;//0110
	set_bit_of_byte(compressed_message,0,7-4,bit_send_deviceid);
	set_bit_of_byte(compressed_message,0,7-3,bit_data_compression);
	set_bit_of_byte(compressed_message,0,7-2,bit_send_speed);
	set_bit_of_byte(compressed_message,0,7-1,bit_send_altitude);
	set_bit_of_byte(compressed_message,0,7-0,bit_send_battery);
	compressed_message[1] = Location_Accuracy << 4;
	set_bit_of_byte(compressed_message,1,7-3,bit_sample_in_min);
	set_bit_of_byte(compressed_message,1,7-2,1);
	set_bit_of_byte(compressed_message,1,7-1,1);
	set_bit_of_byte(compressed_message,1,7-0,1);
	compressed_message[2] = sample_times;
	compressed_message[3] = report_times;
	current_byte = 4;	
	current_bit = 0;
	if(bit_send_deviceid==1)
	{
		//此处是整数   
		compressed_message[4] = g_trk_var.g_trk_device_status.deviceid_4G[0];
		compressed_message[5] = g_trk_var.g_trk_device_status.deviceid_4G[1];
		compressed_message[6] = g_trk_var.g_trk_device_status.deviceid_4G[2];
		compressed_message[7] = g_trk_var.g_trk_device_status.deviceid_4G[3];
		jump_bit(32);//跳4个byte 
	}	 
	memset(log,0,200);
	sprintf(log,"add device id: %x,%x,%x,%x",compressed_message[4],compressed_message[5],compressed_message[6],compressed_message[7]);
	maintask_printf("%s",log);
	//print_log_for_gps_backup();
	maintask_printf("creat_compressed_message_with_measuremet actual_n=%d\r\n",actual_n);
	for(i=0;i<actual_n;i++)
	{
		if(gps_backup[i].type == GPS_NO_FIX)//这里不用加电池信息么
		{
		//先写后跳
		set_bit_of_byte(compressed_message,current_byte,current_bit,0);
		jump_bit(1);
		maintask_printf("GPS_NO_FIX: i=%d-- current_byte:%d,current_bit:%d\r\n",i,current_byte,current_bit);
		}
		else if(gps_backup[i].type == GPS_FIXED || gps_backup[i].type == GPS_BACKUP)
		{	
		U8 buffer_lenth = 0;
		memset(compressed_lat,0,10);
		memset(compressed_long,0,10);		
		memset(fixed_compression_buffer,0,20);
		if(compressed_type == 0)
		{	 
			compression_method_2_lat(gps_backup[i].latitude, compressed_lat, 4);
			compression_method_2_long(gps_backup[i].longitude, compressed_long, 4);
			fixed_compression_buffer[buffer_lenth] = compressed_lat[0]|0x80;buffer_lenth++; //前面第一个bit必须是1，表示有效坐标
			fixed_compression_buffer[buffer_lenth] = compressed_lat[1];buffer_lenth++;
			fixed_compression_buffer[buffer_lenth] = compressed_lat[2];buffer_lenth++;
			fixed_compression_buffer[buffer_lenth] = compressed_long[0];buffer_lenth++;
			fixed_compression_buffer[buffer_lenth] = compressed_long[1];buffer_lenth++;
			fixed_compression_buffer[buffer_lenth] = compressed_long[2];buffer_lenth++;
		}
		else//4
		{
			compression_method_3_lat(gps_backup[i].latitude, compressed_lat, 4);
			compression_method_3_long(gps_backup[i].longitude, compressed_long, 4);
			fixed_compression_buffer[buffer_lenth] = compressed_lat[0]|0x80;buffer_lenth++; //前面第一个bit必须是1，表示有效坐标
			fixed_compression_buffer[buffer_lenth] = compressed_lat[1];buffer_lenth++;
			fixed_compression_buffer[buffer_lenth] = compressed_lat[2];buffer_lenth++;
			fixed_compression_buffer[buffer_lenth] = compressed_lat[3];buffer_lenth++;
			fixed_compression_buffer[buffer_lenth] = compressed_long[0];buffer_lenth++;
			fixed_compression_buffer[buffer_lenth] = compressed_long[1];buffer_lenth++;
			fixed_compression_buffer[buffer_lenth] = compressed_long[2];buffer_lenth++;
			fixed_compression_buffer[buffer_lenth] = compressed_long[3];buffer_lenth++;
		}      
		//速度现在是2个byte
		if(bit_send_speed == 1)
		{
			U16 speed_i = 0;
			U8 speed_i_high = 0;
			U8 speed_i_low = 0;
			//先换成uint16类型
			if(gps_backup[i].speed > 65535)
			{	
				speed_i = 65535;
			}
			else
			{
				speed_i = gps_backup[i].speed;
			}
			speed_i_high = speed_i/256;
			speed_i_low = speed_i-speed_i_high*256;  		  
			fixed_compression_buffer[buffer_lenth] = speed_i_high;buffer_lenth++;
			fixed_compression_buffer[buffer_lenth] = speed_i_low;buffer_lenth++;
		}
		//altitude 也是2bytes
		if(bit_send_altitude == 1)
		{
			U16 altitude_i = 0;
			U8 altitude_i_high = 0;
			U8 altitude_i_low = 0;
			//先换成uint16类型
			if(gps_backup[i].altitude > 65535)
			{	
				altitude_i = 65535;
			}
			else
			{
				altitude_i = gps_backup[i].altitude;
			}
			altitude_i_high=altitude_i/256;
			altitude_i_low=altitude_i-altitude_i_high*256;  		  
			fixed_compression_buffer[buffer_lenth] = altitude_i_high;buffer_lenth++;
			fixed_compression_buffer[buffer_lenth] = altitude_i_low;buffer_lenth++;
		}
		//battery 1byte     		   		 	 
		if(bit_send_battery == 1)
		{
			fixed_compression_buffer[buffer_lenth] = gps_backup[i].battery;buffer_lenth++;
		} 

		if(current_bit != 0)//整数8，什么都不做
		{	
			bit_move_right_of_n_byte(fixed_compression_buffer,current_bit,buffer_lenth);
			for(j=0;j<=buffer_lenth;j++)
			{
				if(j==0)
				{	
				//第一个字节，直接或，没问题,其他所有没填的bit都是0
				compressed_message[current_byte] = compressed_message[current_byte] | fixed_compression_buffer[0];
				}
				else
				{
				compressed_message[current_byte+j] = fixed_compression_buffer[j];
				}
			}
		} 
		else//整数8，直接copy
		{
			for(j=0;j<=(buffer_lenth-1);j++)
			{
				compressed_message[current_byte+j] = fixed_compression_buffer[j];
			}
		} 
		//jumb 7个字节
		jump_bit(buffer_lenth*8);
		}
	}
	//最后，如果跳到一个byte头上，退到上一个byte
	if(current_bit == 0 && current_byte>0)
	{
		current_byte--;
	}	
}

void delta_creat_compressed_message_with_measurement(U8 compressed_number, char compressed_type)
{
	U16 i=0;
	U16 j=0;
	U16 actual_n=0;
	U16 num_1 = 0;
	U16 len = 0;
	int  cordinate_base = -1;
	double lat_first = 0;
	double long_first = 0;
	double lat_difference = 0;
	double long_difference = 0;
	char compressed_lat[10];
	char compressed_long[10];
	char fixed_compression_buffer[20]; 
	char log[200];
	U8 buffer_lenth = 0;
	U8 bit_send_deviceid = bit_of_byte(g_trk_var.g_trk_settings.track_setting,7);
	U8 bit_data_compression = bit_of_byte(g_trk_var.g_trk_settings.track_setting,6);
	U8 bit_send_altitude = bit_of_byte(g_trk_var.g_trk_settings.track_setting,5);
	U8 bit_send_battery = bit_of_byte(g_trk_var.g_trk_settings.track_setting,4);
	U8 bit_send_speed = bit_of_byte(g_trk_var.g_trk_settings.track_setting,3);
	U8 bit_sample_in_min = bit_of_byte(g_trk_var.g_trk_settings.track_setting,2);
	U8 Location_Accuracy =  g_trk_var.g_trk_settings.location_accuracy;
	U8 sample_times = g_trk_var.g_trk_settings.sample_time;
	U8 report_times = g_trk_var.g_trk_settings.report_time;
	maintask_printf("delta_creat_compressed_message_with_measurement compressed_number=%d,compressed_type=%d\r\n",compressed_number,compressed_type);
	if(compressed_number == 0)
	return;
	actual_n = compressed_number;	
	num_1 = get_number_of_records_in_backup_database();
	if(actual_n > num_1)
	actual_n = num_1;
	if(actual_n == 0)
	return;
	memset(compressed_message,0,COMPRESSED_MSG_LEN);//2048	
	memset(compressed_lat,0,10);
	memset(compressed_long,0,10);		
	memset(fixed_compression_buffer,0,20);
	compressed_message[0] = 0x60;//0110
	set_bit_of_byte(compressed_message,0,7-4,bit_send_deviceid);
	set_bit_of_byte(compressed_message,0,7-3,bit_data_compression);
	set_bit_of_byte(compressed_message,0,7-2,bit_send_speed);
	set_bit_of_byte(compressed_message,0,7-1,bit_send_altitude);
	set_bit_of_byte(compressed_message,0,7-0,bit_send_battery);
	compressed_message[1] = Location_Accuracy << 4;
	set_bit_of_byte(compressed_message,1,7-3,bit_sample_in_min);
	set_bit_of_byte(compressed_message,1,7-2,1);
	set_bit_of_byte(compressed_message,1,7-1,1);
	set_bit_of_byte(compressed_message,1,7-0,1);
	compressed_message[2] = sample_times;
	compressed_message[3] = report_times;
	current_byte = 4;	
	current_bit = 0;
	if(bit_send_deviceid == 1)
	{
		//此处是整数
		compressed_message[4] = g_trk_var.g_trk_device_status.deviceid_4G[0];
		compressed_message[5] = g_trk_var.g_trk_device_status.deviceid_4G[1];
		compressed_message[6] = g_trk_var.g_trk_device_status.deviceid_4G[2];
		compressed_message[7] = g_trk_var.g_trk_device_status.deviceid_4G[3];
		memset(log,0,200);
		sprintf(log,"add device id: %d,%d,%d,%d\r\n",compressed_message[4],compressed_message[5],compressed_message[6],compressed_message[7]);
		maintask_printf("%s",log);
		jump_bit(32);//跳4个byte 
	}	 
	print_log_for_gps_backup();
	maintask_printf("creat_compressed_message_with_measuremet actual_n=%d\r\n",actual_n);
	cordinate_base = -1;
	for(i=0;i<actual_n;i++)
	{
		if(gps_backup[i].type == GPS_NO_FIX)//这里不用加电池信息么
		{
			//先写后跳
			set_bit_of_byte(compressed_message,current_byte,current_bit,0);
			jump_bit(1);
			maintask_printf("GPS_NO_FIX: i=%d-- current_byte:%d,current_bit:%d\r\n",i,current_byte,current_bit);
		}
		else if(gps_backup[i].type == GPS_FIXED || gps_backup[i].type == GPS_BACKUP)
		{	
			if(cordinate_base == -1)//base
			{
			cordinate_base = i;
			buffer_lenth = 0;
			memset(compressed_lat,0,10);
			memset(compressed_long,0,10);		
			memset(fixed_compression_buffer,0,20);
			if(compressed_type == 0)//3
			{	 
				compression_method_2_lat(gps_backup[i].latitude, compressed_lat, 4);
				compression_method_2_long(gps_backup[i].longitude, compressed_long, 4);
				fixed_compression_buffer[buffer_lenth] = compressed_lat[0]|0x80;buffer_lenth++; //前面第一个bit必须是1，表示有效坐标
				fixed_compression_buffer[buffer_lenth] = compressed_lat[1];buffer_lenth++;
				fixed_compression_buffer[buffer_lenth] = compressed_lat[2];buffer_lenth++;
				fixed_compression_buffer[buffer_lenth] = compressed_long[0];buffer_lenth++;
				fixed_compression_buffer[buffer_lenth] = compressed_long[1];buffer_lenth++;
				fixed_compression_buffer[buffer_lenth] = compressed_long[2];buffer_lenth++;
			}
			else//4
			{
				compression_method_3_lat(gps_backup[i].latitude, compressed_lat, 4);
				compression_method_3_long(gps_backup[i].longitude, compressed_long, 4);
				fixed_compression_buffer[buffer_lenth] = compressed_lat[0]|0x80;buffer_lenth++; //前面第一个bit必须是1，表示有效坐标
				fixed_compression_buffer[buffer_lenth] = compressed_lat[1];buffer_lenth++;
				fixed_compression_buffer[buffer_lenth] = compressed_lat[2];buffer_lenth++;
				fixed_compression_buffer[buffer_lenth] = compressed_lat[3];buffer_lenth++;
				fixed_compression_buffer[buffer_lenth] = compressed_long[0];buffer_lenth++;
				fixed_compression_buffer[buffer_lenth] = compressed_long[1];buffer_lenth++;
				fixed_compression_buffer[buffer_lenth] = compressed_long[2];buffer_lenth++;
				fixed_compression_buffer[buffer_lenth] = compressed_long[3];buffer_lenth++;
			}      
			//速度现在是2个byte
			if(bit_send_speed == 1)
			{
				U16 speed_i = 0;
				U8 speed_i_high = 0;
				U8 speed_i_low = 0;
				//先换成uint16类型
				if(gps_backup[i].speed > 65535)
				{	
				speed_i = 65535;
				}
				else
				{
				speed_i = gps_backup[i].speed;
				}
				speed_i_high = speed_i/256;
				speed_i_low = speed_i - 	speed_i_high*256;  		  
				fixed_compression_buffer[buffer_lenth] = speed_i_high;buffer_lenth++;
				fixed_compression_buffer[buffer_lenth] = speed_i_low;buffer_lenth++;
			}
			//altitude 也是2bytes
			if(bit_send_altitude == 1)
			{
				U16 altitude_i = 0;
				U8 altitude_i_high = 0;
				U8 altitude_i_low = 0;
				//先换成uint16类型
				if(gps_backup[i].altitude > 65535)
				{	
				altitude_i = 65535;
				}
				else
				{
				altitude_i = gps_backup[i].altitude;
				}
				altitude_i_high = altitude_i/256;
				altitude_i_low = altitude_i -altitude_i_high*256;  		  
				fixed_compression_buffer[buffer_lenth] = altitude_i_high;buffer_lenth++;
				fixed_compression_buffer[buffer_lenth] = altitude_i_low;buffer_lenth++;
			}
			//battery 1byte     		   		 	 
			if(bit_send_battery == 1)
			{
				fixed_compression_buffer[buffer_lenth] = gps_backup[i].battery;buffer_lenth++;
			} 
			if(current_bit != 0)//整数8，什么都不做
			{	
				bit_move_right_of_n_byte(fixed_compression_buffer,current_bit,buffer_lenth);
				for(j=0;j<=buffer_lenth;j++)
				{
					if(j==0)
					{	
					//第一个字节，直接或，没问题,其他所有没填的bit都是0
					compressed_message[current_byte] = compressed_message[current_byte] | fixed_compression_buffer[0];
					}
					else
					{
					compressed_message[current_byte+j] = fixed_compression_buffer[j];
					}
				}
			} 
			else//整数8，直接copy
			{
				for(j=0;j<=(buffer_lenth-1);j++)
				{
					compressed_message[current_byte+j] = fixed_compression_buffer[j];
				}
			} 
			//jumb 7个字节
			jump_bit(buffer_lenth*8);
			}
			else//之前已经有base了
			{
			buffer_lenth = 0;
			memset(compressed_lat,0,10);
			memset(compressed_long,0,10);		
			memset(fixed_compression_buffer,0,20);
			lat_difference = gps_backup[i].latitude - gps_backup[cordinate_base].latitude;
			long_difference = gps_backup[i].longitude - gps_backup[cordinate_base].longitude;;
			memset(log,0,200);
			sprintf(log,"fix:lat=%f,long=%f,lat_difference=%f,long_difference=%f",gps_backup[i].latitude,gps_backup[i].longitude,lat_difference,long_difference);
			maintask_printf("%s",log);
			delta_compression_method(lat_difference, compressed_lat, 4);
			delta_compression_method(long_difference , compressed_long, 4);
			//不需要正负bit，直接压，压缩里面有正负
			fixed_compression_buffer[buffer_lenth] = compressed_lat[0];buffer_lenth++;
			fixed_compression_buffer[buffer_lenth] = compressed_lat[1];buffer_lenth++; 
			fixed_compression_buffer[buffer_lenth] = compressed_long[0];buffer_lenth++; 
			fixed_compression_buffer[buffer_lenth] = compressed_long[1];buffer_lenth++;
			//速度现在是2个byte
			if(bit_send_speed == 1)
			{
				U16 speed_i = 0;
				U8 speed_i_high = 0;
				U8 speed_i_low = 0;
				//先换成uint16类型
				if(gps_backup[i].speed > 65535)
				{	
				speed_i = 65535;
				}
				else
				{
				speed_i = gps_backup[i].speed;
				}
				speed_i_high = speed_i/256;
				speed_i_low = speed_i - 	speed_i_high*256;  		  
				fixed_compression_buffer[buffer_lenth] = speed_i_high;buffer_lenth++;
				fixed_compression_buffer[buffer_lenth] = speed_i_low;buffer_lenth++;
			}
			//altitude 也是2bytes
			if(bit_send_altitude == 1)
			{
				U16 altitude_i = 0;
				U8 altitude_i_high = 0;
				U8 altitude_i_low = 0;
				//先换成uint16类型
				if(gps_backup[i].altitude > 65535)
				{	
				altitude_i = 65535;
				}
				else
				{
				altitude_i = gps_backup[i].altitude;
				}
				altitude_i_high = altitude_i/256;
				altitude_i_low = altitude_i - 	altitude_i_high*256;  		  
				fixed_compression_buffer[buffer_lenth] = altitude_i_high;buffer_lenth++;
				fixed_compression_buffer[buffer_lenth] = altitude_i_low;buffer_lenth++;
			}
			//battery 1byte     		   		 	 
			if(bit_send_battery == 1)
			{
				fixed_compression_buffer[buffer_lenth] = gps_backup[i].battery;buffer_lenth++;
			} 
			//有效要写一个1
			set_bit_of_byte(compressed_message,current_byte,current_bit,1);
			jump_bit(1);
			if(current_bit != 0)//整数8，什么都不做
			{	
				bit_move_right_of_n_byte(fixed_compression_buffer,current_bit,buffer_lenth);
				for(j=0;j<=buffer_lenth;j++)
				{
					if(j==0)
					{	
					//第一个字节，直接或，没问题,其他所有没填的bit都是0
					compressed_message[current_byte] = compressed_message[current_byte] | fixed_compression_buffer[0];
					}
					else
					{
					compressed_message[current_byte+j] = fixed_compression_buffer[j];
					}
				}
			} 
			else//整数8，直接copy
			{
				for(j=0;j<=(buffer_lenth-1);j++)
				{
					compressed_message[current_byte+j] = fixed_compression_buffer[j];
				}
			} 
			//jumb 7个字节
			jump_bit(buffer_lenth*8);
			}		   
		}
	}
	//最后，如果跳到一个byte头上，退到上一个byte
	if(current_bit == 0 && current_byte>0)
	{
		current_byte--;
	}	
}


BOOL gps_xmode_send_with_measuremet(void) 	  	
{
	BOOL ret = FALSE;
	U8 tracking_sample_times=g_trk_var.g_trk_settings.sample_time;
	U8 tracking_report_times=g_trk_var.g_trk_settings.report_time;
	U8 tracking_channel = g_trk_var.g_trk_settings.track_channel; 
	U8 tracking_compression =  g_trk_var.g_trk_settings.location_accuracy;//压缩，0:23bit低压缩，1：32bit，2：debug
	U8 tracking_send_deviceid = bit_of_byte(g_trk_var.g_trk_settings.track_setting,7);//-	Transmit Device ID (0 – off/1 – on)
	U8 tracking_delta_compression = bit_of_byte(g_trk_var.g_trk_settings.track_setting,6);//-	Delta Compression (0 – off/1 – on)

	if(tracking_report_times==0)
	{
		maintask_printf("gps_xmode_send_with_measuremet error :tracking_report_times =  0\r\n");
		return FALSE;
	}
	xmode_send_with_measuremet_times++;
	maintask_printf("gps_xmode_send xmode_send_with_measuremet_times=%d,tracking_report_times=%d\r\n",xmode_send_with_measuremet_times,tracking_report_times); 
	if(xmode_send_with_measuremet_times >= tracking_report_times-tracking_sample_times)
	{	
		xmode_send_with_measuremet_times = 0;
		ret = TRUE;	
		maintask_printf("gps_xmode_send_with_measuremet data: delta:%d, add device id:%d, multi tracks:%d, compression:%d\r\n",tracking_delta_compression,tracking_send_deviceid,tracking_report_times,tracking_compression);
		if(FALSE == gps_history_all_no_fix(tracking_report_times)) 
		{	
			//-	Delta Compression (0 – off/1 – on) (1 byte) 
			if(tracking_delta_compression == 0)//no delta
			{	
				//此处用来发送单条信息单多条历史坐标
				creat_compressed_message_with_measuremet(tracking_report_times,tracking_compression); 
				data_send_any_string(compressed_message,(current_byte+1));
			} 
			else //delta enable
			{
				//此处用来发送单条信息单多条历史坐标
				delta_creat_compressed_message_with_measurement(tracking_report_times,tracking_compression); 
				data_send_any_string(compressed_message,(current_byte+1));
			}
		}       		       	   		   
		//check_no_gsm_history(g_trk_var.g_trk_settings.report_time);
	}
	if(ret == TRUE)
	{	
		maintask_printf("gps_xmode_send_with_measuremet TRUE\r\n");
	}
	else
	{
		maintask_printf("gps_xmode_send_with_measuremet FALSE\r\n");
	}	 	  
	return ret;
}    

void tracking_with_measurement_in_sec(void)
{
	U8 times_for_fix = 0;
	U8 times_for_no_fix = 0;
	maintask_printf("tracking_with_measurement_in_sec timer:%d, fix_sending_times:%d,no_fix_sending:%d\r\n",g_trk_var.g_trk_settings.sample_time,fix_sending_times,no_fix_sending);
	times_for_fix = 60/g_trk_var.g_trk_settings.sample_time;
	times_for_no_fix = 300/g_trk_var.g_trk_settings.sample_time;
	maintask_printf("times_for_fix=%d,times_for_no_fix=%d",times_for_fix,times_for_no_fix);
	if(fix_sending_times > times_for_fix || no_fix_sending > times_for_no_fix)
	{
		maintask_printf("stop tracking and return...");
		XMODE_COMMAND_TURN_OFF_GPS();//此处必须关gps，否则没地方关
		//trackin in sec 中断永远开 	
		#ifdef SUPPORT_MY_GSENSOR
		xmode_turnon_gsensor();
		CheckTurnOnOffGsensor();
		#endif
		stopMyTimer(MY_TIMER_61);
		startMyTimer(MY_TIMER_61, 60*1000, tracking_with_measurement_in_sec);
		return;
	} 
	//秒tracking不能每次都开，比如5s开一次，gps那边不停开关uart口，导致uart口没开就又重来了
	if(Is_xmode_gps_on() == FALSE)
	{	
		XMODE_COMMAND_TURN_ON_GPS();
	}   
	if(TRUE == gps_2d_3d_fix_with_hdop_and_fix_sec(10))//timer为1，发realtime的 //抑制飘移，只要3D不要2D
	{
		gps_back_up_push_fixed();//压栈，保存至backup，发的时候再从backup里面取
		if(TRUE == gps_xmode_send_with_measuremet())
		{
			fix_sending_times++;
			no_fix_sending=0;
		}
	}
	else
	{
		gps_back_up_push_no_fix();
		if(TRUE == gps_xmode_send_with_measuremet())
		{	
			no_fix_sending++;
			fix_sending_times = 0;
			//check_if_send_wifi();//要在外面那发的次数也太多了
		}
	}	
	//trackin in sec 中断永远开 	
	#ifdef SUPPORT_MY_GSENSOR
	xmode_turnon_gsensor();
	CheckTurnOnOffGsensor(); 	
	#endif
	stopMyTimer(MY_TIMER_61);
	startMyTimer(MY_TIMER_61, g_trk_var.g_trk_settings.sample_time*1000, tracking_with_measurement_in_sec);
}

void tracking_with_measurement_fix_wait(void)
{
	if(TRUE == gps_2d_3d_fix_with_hdop_and_fix_sec(10))
	{
		maintask_printf("tracking_with_measurement_fix_wait:FIX!\r\n");
		gps_back_up_push_fixed();//压栈，保存至backup，发的时候再从backup里面取
		//重新校准fence那边，防止两边撞一起，费电
		//if(TRUE == fence_or_speed_alarm_on())
		//{
		//	U8 fence_speed_check_sec = get_fence_speed_checking_timer_in_sec();
		//	printf("gps_fix_wait:check fence speed\r\n");
			//防止跟那边在同一秒撞上
			//check_speed_alarm(); //speed 可以
			//check_fence_alarm();
			//stopMyTimer(MINI_MY_TIMER_11);//这是fence speed 休眠5分钟的timer，这里要关
			//stopMyTimer(MY_TIMER_51);
			//startMyTimer(MY_TIMER_51, fence_speed_check_sec*1000, check_fence_or_speed_start);  
		//}
		if(TRUE == gps_xmode_send_with_measuremet())
		{
			tracking_with_measurement_sent = TRUE;
			if(g_trk_var.g_trk_content_info.gps_info.speed_km/1000>10)//有速度认为在动，不需要加速度传感器
			{
			//当成动了
			fix_sending_times=0;
			no_fix_sending=0;
			}
			else
			{
			fix_sending_times++;
			no_fix_sending=0;
			} 
		}
		else
		{
			tracking_with_measurement_sent = FALSE;
		}  
		//gps要保持一段时间
		if(Is_smart_gps() == TRUE)
		{	
			xmode_gps_off_after_fix_check();
		} 
		else//smart_gps = KAL_FALSE
		{
			if(tracking_high_speed_turnoff_gps == TRUE)	
			{
				U8 speed_off = get_speed_for_turnoff_gps_in_tracking();
				if(g_trk_var.g_trk_content_info.gps_info.speed_km > speed_off)
				{
					xmode_gps_off_after_fix_check();
				}
			}
		}	   
		tracking_with_measurement_fixed = TRUE;//不管有没有真发，这里都必须认为是真发了，否则那边会认为没法压一个no fix 进去 
		//开中断,此处必须开，否则fix_sending_times++了，之前gsensor被关了，没法跑中断，下次再检测tracking_with_measurement_in_min就会把gps关掉
		//xmode_turnon_gsensor();
		//CheckTurnOnOffGsensor(); 
	}
	else
	{
		//没fix，10s看一次
		maintask_printf("tracking_with_measurement_fix_wait:NO GPS FIX!\r\n");
		tracking_with_measurement_fixed = FALSE;
		stopMyTimer(MY_TIMER_62);
		startMyTimer(MY_TIMER_62, 2*1000, tracking_with_measurement_fix_wait);
	}
}

void tracking_with_measurement_no_fix_process(void)
{
	maintask_printf("tracking_with_measurement_no_fix_process");
	gps_back_up_push_no_fix();
	//check_if_send_wifi();
	if(TRUE == gps_xmode_send_with_measuremet())
	{
		tracking_with_measurement_sent = TRUE;
		fix_sending_times = 0;//本次没fix，置0
		no_fix_sending++;
	}
	else
	{
		tracking_with_measurement_sent = FALSE;
	}
	maintask_printf("after 1min,send 0 and stop tracking...");
	stopMyTimer(MY_TIMER_62);//此处应该关掉gps_fix_wait的timer，全停
	stopMyTimer(MINI_MY_TIMER_14);//check 1min的timer
	stopMyTimer(MINI_MY_TIMER_15);//定到位看什么时候关gps的timer
	XMODE_COMMAND_TURN_OFF_GPS();//此处必须关gps，否则没地方关
	//xmode_turnon_gsensor();
	//CheckTurnOnOffGsensor();
}	

void tracking_with_measurement_send_zero_every_1_min(void)
{
	send_zero_count++;
	maintask_printf("tracking_with_measurement_send_zero_every_1_min %d --- %d",send_zero_count,send_zero_total);
	gps_back_up_push_no_fix();
	//check_if_send_wifi();
	if(TRUE == gps_xmode_send_with_measuremet())
	{
		maintask_printf("push 0, tracking_with_measurement_sent = KAL_TRUE");
		tracking_with_measurement_sent = TRUE;
	}	
	else
	{
		maintask_printf("push 0, tracking_with_measurement_sent = KAL_FALSE");
		tracking_with_measurement_sent = FALSE;
	}
	if(send_zero_count >= send_zero_total)
	{
		stopMyTimer(MINI_MY_TIMER_10);
		tracking_with_measurement_start();
	}	 	
	else
	{	
		stopMyTimer(MINI_MY_TIMER_10);
		startMyTimer(MINI_MY_TIMER_10, 60*1000, tracking_with_measurement_send_zero_every_1_min);
	}  
}


void tracking_with_measurement_wait_for_minutes_before_restart(U8 number_min)
{
	maintask_printf("tracking_with_measurement_wait_for_minutes_before_restart %d",number_min);
	send_zero_total = number_min;
	send_zero_count = 0;
	stopMyTimer(MINI_MY_TIMER_10);
	startMyTimer(MINI_MY_TIMER_10, 60*1000, tracking_with_measurement_send_zero_every_1_min);
}		

void  tracking_with_measurement_in_min(void)
{
	maintask_printf("tracking_with_measurement_in_min timer:%d, fix_sending_times:%d,no_fix_sending:%d\r\n",g_trk_var.g_trk_settings.sample_time,fix_sending_times,no_fix_sending);
	if(fix_sending_times > 0 || no_fix_sending > 3)
	{
		if(tracking_with_measurement_sent == TRUE)//之前发过了
		{
			maintask_printf("tracking_with_measurement_sent == KAL_TRUE, stop tracking and return...\r\n");
			XMODE_COMMAND_TURN_OFF_GPS();//此处必须关gps，否则没地方关
			stopMyTimer(MY_TIMER_62); //fix的timer
			stopMyTimer(MINI_MY_TIMER_15);//关掉gps定位后多久关gps的timer
			//xmode_turnon_gsensor();
			//CheckTurnOnOffGsensor();
			stopMyTimer(MY_TIMER_61);
			startMyTimer(MY_TIMER_61, g_trk_var.g_trk_settings.sample_time*60*1000, tracking_with_measurement_in_min);
		} 
		else//之前没发过
		{
			maintask_printf("tracking_with_measurement_sent == KAL_FALSE, push 0 and wait...\r\n");
			tracking_with_measurement_no_fix_process();
			stopMyTimer(MY_TIMER_61);
			startMyTimer(MY_TIMER_61, g_trk_var.g_trk_settings.sample_time*60*1000, tracking_with_measurement_in_min);
		}
		return;
	} 
	//timer内没fix
	if(tracking_with_measurement_fixed == FALSE)
	{
		BOOL in_door = FALSE;
		U8 max_snr = g_trk_var.g_trk_content_info.gps_info.max_snr;
		U8 max_indoor_sat_number = get_indoor_max_sat_number();
		U8 max_indoor_sat_signal = get_indoor_max_sat_signal();
		if(gps_moving_in_last_min == TRUE)
		{	
		maintask_printf("GPSDW_sate_number= %d, g_trk_var.g_trk_content_info.gps_info.max_snr=%d, gps_moving_in_last_min=KAL_TRUE\r\n",g_trk_var.g_trk_content_info.gps_info.satellite_visable_num,max_snr);
		}
		else
		{
		maintask_printf("GPSDW_sate_number= %d, g_trk_var.g_trk_content_info.gps_info.max_snr=%d, gps_moving_in_last_min=KAL_FALSE\r\n",g_trk_var.g_trk_content_info.gps_info.satellite_visable_num,max_snr);
		}   	
		if(gps_driving_mode == TRUE)
		{	
		maintask_printf("gps_driving_mode=KAL_TRUE\r\n");
		}
		else
		{
		maintask_printf("gps_driving_mode=KAL_FALSE\r\n");
		}
		//如果室内在动，信号很差，开1分钟关2分钟
		if( (fix_sending_times == 0 && no_fix_sending == 0) && //之前动了
		(gps_moving_in_last_min == TRUE) &&
		(Is_xmode_gps_on() == TRUE && Is_gps_searching() == TRUE && g_trk_var.g_trk_content_info.gps_info.satellite_visable_num <= max_indoor_sat_number && max_snr < max_indoor_sat_signal)) //搜星状态，没fix
		{
		in_door = TRUE;
		maintask_printf("in_door = KAL_TRUE\r\n");
		}
		gps_back_up_push_no_fix();
		//check_if_send_wifi();
		if(TRUE == gps_xmode_send_with_measuremet())
		{
			maintask_printf("push 0, tracking_with_measurement_sent = KAL_TRUE\r\n");
			tracking_with_measurement_sent = TRUE;
			fix_sending_times = 0;//本次没fix，置0
			no_fix_sending++;
		}	
		else
		{
			maintask_printf("push 0, tracking_with_measurement_sent = KAL_FALSE\r\n");
			tracking_with_measurement_sent = FALSE;
		}
		//直接停
		if((no_fix_sending == 2 && Is_xmode_gps_on() == TRUE && Is_gps_searching() == TRUE && g_trk_var.g_trk_content_info.gps_info.satellite_visable_num == 0 && max_snr  == 0) //搜星状态，没fix,全0
		)
		{
			maintask_printf("make no_fix_sending = 4\r\n");
			no_fix_sending = 4;
			maintask_printf("stop tracking and return...\r\n");
			stopMyTimer(MY_TIMER_62);//此处应该关掉gps_fix_wait的timer，全停
			stopMyTimer(MINI_MY_TIMER_15);//关掉gps定位后多久关gps的timer
			XMODE_COMMAND_TURN_OFF_GPS();//此处必须关gps，否则没地方关
			//xmode_turnon_gsensor();
			//CheckTurnOnOffGsensor();
			stopMyTimer(MY_TIMER_61);
			startMyTimer(MY_TIMER_61, g_trk_var.g_trk_settings.sample_time*60*1000, tracking_with_measurement_in_min);
			return;
		}
		if(in_door == TRUE)
		{
			if(g_trk_var.g_trk_content_info.gps_info.satellite_visable_num == 0 && max_snr == 0)
			{
			maintask_printf("XMODE INDOOR MOVING, NOTHING SATELLITE !!! STOP 5 MIN\r\n");	 
			//关所有
			tracking_with_measurement_stop();
			if(gps_driving_mode == TRUE)
			{	
				tracking_with_measurement_wait_for_minutes_before_restart(2);//2分钟
			}
			else
			{		
			}  
			}
			else
			{
				maintask_printf("XMODE INDOOR MOVING, SOME LOW SATELLITE !!! STOP 2 MIN\r\n");	 
				//关所有
				tracking_with_measurement_stop();
				tracking_with_measurement_wait_for_minutes_before_restart(2);//5分钟
			}          		  
			return;
		}
		else
		{ 
			maintask_printf("XMODE OUTDOOR MOVING !!! DO NOTHING\r\n");	
		}	     
	}
	else//fix 发过了
	{
		//gps_fix_wait那边已经做完了
	}	  	
	tracking_with_measurement_fixed = FALSE;
	XMODE_COMMAND_TURN_ON_GPS();
	stopMyTimer(MINI_MY_TIMER_15);//关掉gps定位后多久关gps的timer
	//开中断，否则如果一直没定位动也没用了
	//xmode_turnon_gsensor();
	//CheckTurnOnOffGsensor();
	stopMyTimer(MY_TIMER_61);
	startMyTimer(MY_TIMER_61, 60*1000, tracking_with_measurement_in_min);
	stopMyTimer(MY_TIMER_62);
	startMyTimer(MY_TIMER_62, 2*1000, tracking_with_measurement_fix_wait);
}

/***************************************************************************************
	g_trk_var.g_trk_settings.track_channel  - "Tracking Channel" as in the old message.
	g_trk_var.g_trk_settings.sample_time- "Sample Time (mins)" as in the old message.
	g_trk_var.g_trk_settings.  - "Report Time (mins)" as in the old message.
	g_trk_var.g_trk_settings.location_accuracy  - "Location Accuracy" as in the old message.
	report_time
	g_trk_var.g_trk_settings.track_setting
	1 bit - "Send Device ID" NOT AS IN THE OLD MESSAGE.
	1 bit - "Delta Compression" NOT AS IN THE OLD MESSAGE.
  1 bit - Send Altitude measurement
  1 bit - Send Battery measurement
	1 bit - Send Speed measurement
	1 bit - Sample time in minutes? (if not then it is in seconds)
	2 bits - Padding

********************************************************************/
void tracking_with_measurement_start(void)   	
{
	maintask_printf("tracking_with_measurement_start channel=%d,sample=%d,report=%d,setting=%d\r\n",
	g_trk_var.g_trk_settings.track_channel,
	g_trk_var.g_trk_settings.sample_time,
	g_trk_var.g_trk_settings.report_time,
	g_trk_var.g_trk_settings.track_setting);
	if(g_trk_var.g_trk_settings.sample_time == 0)
	{
		tracking_with_measurement_stop();
		//关中断
		//xmode_turnoff_gsensor();
		//CheckTurnOnOffGsensor();
		return;
	}
	//先停
	tracking_with_measurement_stop();
	//tracking in seconds
	if(bit_of_byte(g_trk_var.g_trk_settings.track_setting,2) == 0)
	{
		maintask_printf("go tracking_with_measurement_in_sec\r\n");
		tracking_with_measurement_in_sec();
	}
	else //分钟
	{	
		if(g_trk_var.g_trk_settings.sample_time== 1)
		{	
		//这时候gsensor要开，要确保1分钟内在测震动
		//xmode_turnon_gsensor();
		//CheckTurnOnOffGsensor();
		XMODE_COMMAND_TURN_ON_GPS();
		stopMyTimer(MY_TIMER_62);
		startMyTimer(MY_TIMER_62, 2*1000, tracking_with_measurement_fix_wait);
		stopMyTimer(MY_TIMER_61);
		startMyTimer(MY_TIMER_61,60*1000, tracking_with_measurement_in_min);
		}
		else//大于1分钟
		{
		tracking_with_measurement_fixed = FALSE;
		tracking_with_measurement_sent = TRUE;//重头开始
		tracking_with_measurement_in_more_than_1_min();	   
		}   
	}
}  

void tracking_with_measurement_stop(void)
{
	no_fix_sending = 0;
	fix_sending_times = 0;
	tracking_with_measurement_fixed = FALSE;
       tracking_with_measurement_sent = FALSE;//重头开始
 	stopMyTimer(MY_TIMER_61);//tracking timer
 	stopMyTimer(MY_TIMER_62); //fix的timer
 	//stopMyTimer(MINI_MY_TIMER_13);//关掉gps定位后多久关gps的timer
 	stopMyTimer(MINI_MY_TIMER_14);//gps check 1分钟的timer
 	stopMyTimer(MINI_MY_TIMER_10); //关tracking 再开的timer
 	XMODE_COMMAND_TURN_OFF_GPS();
} 

void tracking_with_measurement_stop_new(void)
{
 	stopMyTimer(TRACKER_GPS_FIX_WITH_MEASUREMENT_WAIT);
 	//stopMyTimer(TRACKER_GPS_STOP_WITH_MEASUREMENT); //fix的timer
 	XMODE_COMMAND_TURN_OFF_GPS();
} 

void tracking_with_measurement_fix_wait_new(void)
{
	if(getViburatorWorkmode()||tracker_with_measuremet_steps == 999){
	tracker_with_measuremet_steps = 0;
	}
	maintask_printf("steps=%d vib=%d\r\n",tracker_with_measuremet_steps,getViburatorWorkmode());
	//如果现在有gps信号，现在发
	if(TRUE == gps_2d_3d_fix_with_hdop_and_fix_sec(30))
	{	
		maintask_printf("tracking_with_measurement_fix_wait_new  FIX!\r\n");
		gps_back_up_push_fixed();//压栈，保存至backup，发的时候再从backup里面取
		check_speed_alarm();
		check_fence_alarm();
		//get_gps_check_schedule_sleep();
		//GET_GPS_COMMAND_TURN_OFF_GPS();	  
		tracking_with_measurement_stop_new();
		g_trk_var.g_trk_content_info.gps_info.gps_state = 0;
		if(TRUE == gps_xmode_send_with_measuremet())
		{
			tracking_with_measurement_sent = TRUE;
		}
	}
	else //没fix,等
	{
		maintask_printf("tracking_with_measurement_fix_wait_new NO FIX!\r\n");
		//10S看一次
		stopMyTimer(TRACKER_GPS_FIX_WITH_MEASUREMENT_WAIT);
		startMyTimer(TRACKER_GPS_FIX_WITH_MEASUREMENT_WAIT, 5000, tracking_with_measurement_fix_wait_new);
	}
}

void  tracking_with_measurement_in_min_new(void)
{
	maintask_printf("tracking_with_measurement_in_min_newsteps=%d vib=%d\r\n",tracker_with_measuremet_steps,getViburatorWorkmode());
	tracker_with_measuremet_steps++;
	if(tracker_with_measuremet_steps == 1)
	{
		XMODE_COMMAND_TURN_ON_GPS();
	}else{
		if(!getViburatorWorkmode()){
		XMODE_COMMAND_TURN_OFF_GPS();
		}
	}
	tracking_with_measurement_fix_wait_new();
	//开中断，否则如果一直没定位动也没用了
	//xmode_turnon_gsensor();
	//CheckTurnOnOffGsensor();
	stopMyTimer(TRACKER_GPS_START_WITH_MEASUREMENT);
	startMyTimer(TRACKER_GPS_START_WITH_MEASUREMENT,g_trk_var.g_trk_settings.sample_time*60*1000, tracking_with_measurement_in_min_new);
	//5分钟后强制关
	//stopMyTimer(TRACKER_GPS_STOP_WITH_MEASUREMENT);
	//startMyTimer(TRACKER_GPS_STOP_WITH_MEASUREMENT, 5*60*1000, tracking_with_measurement_stop_new);
}

/***************************************************************************************
	g_trk_var.g_trk_settings.track_channel  - "Tracking Channel" as in the old message.
	g_trk_var.g_trk_settings.sample_time- "Sample Time (mins)" as in the old message.
	g_trk_var.g_trk_settings.  - "Report Time (mins)" as in the old message.
	g_trk_var.g_trk_settings.location_accuracy  - "Location Accuracy" as in the old message.
	report_time
	g_trk_var.g_trk_settings.track_setting
	1 bit - "Send Device ID" NOT AS IN THE OLD MESSAGE.
	1 bit - "Delta Compression" NOT AS IN THE OLD MESSAGE.
  	1 bit - Send Altitude measurement
  	1 bit - Send Battery measurement
	1 bit - Send Speed measurement
	1 bit - Sample time in minutes? (if not then it is in seconds)
	2 bits - Padding

********************************************************************/
void tracking_with_measurement_start_new(void)   	
{
	maintask_printf("tracking_with_measurement_start_new channel=%d,sample=%d,report=%d,setting=%d\r\n",
	g_trk_var.g_trk_settings.track_channel,
	g_trk_var.g_trk_settings.sample_time,
	g_trk_var.g_trk_settings.report_time,
	g_trk_var.g_trk_settings.track_setting);
	if(g_trk_var.g_trk_settings.sample_time == 0)
	{
		tracking_with_measurement_stop_new();
		return;
	}
	//先停
	tracking_with_measurement_stop_new();
	tracking_with_measurement_in_min_new();
}  

void xmode_start_sending_timer(U32 continuous_timer_min)
{
	timer_minutes = continuous_timer_min;
	maintask_printf("xmode_start_sending_timer: %d mins\r\n",timer_minutes);
	fix_sending_times = 0; 
	no_fix_sending = 0; 
	//先全关再开
	xmode_stop_all();	
	if(timer_minutes == 0)
	{
		 //关中断
		 #ifdef SUPPORT_MY_GSENSOR
		 gSensor_off();
		 CheckTurnOnOffGsensor();
		 #endif
	}
	else if(timer_minutes == 99)//连续发送，不关gps，每10s上传一次，99:gps一直开
	{
		
		//开gps，然后每10s发一次
		GPS_POWERON();//开gps
		stopMyTimer(MY_TIMER_34);
		startMyTimer(MY_TIMER_34, 20*1000, gps_always_on_keep_sending);
	}	
	else if(timer_minutes == 1) 	
	{	
	 //这时候gsensor要开，要确保1分钟内在测震动
	 #ifdef SUPPORT_MY_GSENSOR
	 gSensor_on();
	 CheckTurnOnOffGsensor();
	 #endif
	 GPS_POWERON();
	 stopMyTimer(MY_TIMER_33);
	 startMyTimer(MY_TIMER_33, 2*1000, gps_fix_wait);

	 stopMyTimer(MY_TIMER_31);
	 startMyTimer(MY_TIMER_31, timer_minutes*60*1000, xmode_start_gps_timer_check_in_minutes);
	}
	else //more than 1
	{	
	gps_xmode_fixed = FALSE;
	gps_xmode_sent = TRUE;//重头开始
	xmode_start_gps_timer_check_in_more_than_1_minutes();
	}		  	
}

void XmodeStart(void)
{
	xmode_start_sending_timer(g_trk_var.g_trk_settings.Xmode_smaple_time);
}   	

BOOL no_gsm_history = FALSE;
char  no_gsm_history_from_year = 0;
char  no_gsm_history_from_month = 0;
char  no_gsm_history_from_day = 0;
char  no_gsm_history_from_hour = 0;
char  no_gsm_history_from_min = 0;
char  no_gsm_history_from_sec = 0;

char  no_gsm_history_to_year = 0;
char  no_gsm_history_to_month = 0;
char  no_gsm_history_to_day = 0;
char  no_gsm_history_to_hour = 0;
char  no_gsm_history_to_min = 0;
char  no_gsm_history_to_sec = 0;

void no_gsm_history_send(void)
{
	 if(GET_HISTORY_PROCESSING == TRUE)
	 {		   		
			return;
	 }
	 //数据ok
	 get_history_time_from[0] = no_gsm_history_from_year; 
	 get_history_time_from[1] = no_gsm_history_from_month; 
	 get_history_time_from[2] = no_gsm_history_from_day; 
	 get_history_time_from[3] = no_gsm_history_from_hour; 
	 get_history_time_from[4] = no_gsm_history_from_min; 
	 get_history_time_from[5] = no_gsm_history_from_sec; 
	 
	 get_history_time_to[0] = no_gsm_history_to_year;
	 get_history_time_to[1] = no_gsm_history_to_month;
	 get_history_time_to[2] = no_gsm_history_to_day;
	 get_history_time_to[3] = no_gsm_history_to_hour;
	 get_history_time_to[4] = no_gsm_history_to_min;
	 get_history_time_to[5] = no_gsm_history_to_sec;
	 GET_HISTORY_PROCESSING = TRUE;
        tracker_protocol_send_deal(REPLY_HEADER_HISTORYREPORT,g_trk_var.g_user_settings.data_transfer_channel);
} 

void clear_no_gsm_history_data(void)
{
	no_gsm_history = FALSE;
	no_gsm_history_from_year = 0;
	no_gsm_history_from_month = 0;
	no_gsm_history_from_day = 0;
	no_gsm_history_from_hour = 0;
	no_gsm_history_from_min = 0;
	no_gsm_history_from_sec = 0;
	no_gsm_history_to_year = 0;
	no_gsm_history_to_month = 0;
	no_gsm_history_to_day = 0;
	no_gsm_history_to_hour = 0;
	no_gsm_history_to_min = 0;
	no_gsm_history_to_sec = 0;	
	maintask_printf("clear_no_gsm_history_data\r\n");	
}

//extern kal_uint8 test_v;
void check_no_gsm_history(U8 report_times)
{
	U16 actual_n = 0;
	U16 num_1 = 0;
	U16 i = 0;
	if(1)//(TRUE == gsm_is_in_full_service())
	//if(test_v == 0)
	{
		maintask_printf("check_no_gsm_history GSM full service\r\n");
		if(gsm_service_stable == TRUE)
		{	
			maintask_printf("gsm_service_stable == KAL_TRUE\r\n");
			//查询no_gsm_history，如果有，发出去
			if(no_gsm_history == TRUE)
			{
			maintask_printf("check_no_gsm_history GSM full service,go no_gsm_history_send\r\n");	
			no_gsm_history_send();
			clear_no_gsm_history_data();
			}
			else
			{
			maintask_printf("check_no_gsm_history GSM full service, do nothing\r\n");
			}
		}
		else
		{
			maintask_printf("gsm_service_stable == KAL_FALSE, WAIT...\r\n");
		}			 	
	}
	else
	{		 
		actual_n = report_times;	
		num_1 = get_number_of_records_in_backup_database();
		if(actual_n > num_1)
		actual_n = num_1;
		if(actual_n == 0)
		{
			maintask_printf("check_no_gsm_history actual_n == 0, do nothing\r\n"); 
			return;
		} 

		if(no_gsm_history == TRUE)//要刷新最新点
		{
			maintask_printf("check_no_gsm_history no_gsm_history == KAL_TRUE\r\n"); 
			for(i=0;i<actual_n;i++) //找最新的那个点
			{
				if(gps_backup[i].type == GPS_FIXED)
				{	
				no_gsm_history_to_year = gps_backup[i].year;
				no_gsm_history_to_month = gps_backup[i].month;
				no_gsm_history_to_day = gps_backup[i].day;
				no_gsm_history_to_hour = gps_backup[i].hour;
				no_gsm_history_to_min = gps_backup[i].minute;
				no_gsm_history_to_sec = gps_backup[i].second;       
				maintask_printf("to: %d,%d,%d,%d,%d,%d\r\n",no_gsm_history_to_year,no_gsm_history_to_month,no_gsm_history_to_day,no_gsm_history_to_hour,no_gsm_history_to_min,no_gsm_history_to_sec);
				break;
				}
			}
		}
		else   
		{
			for(i=actual_n;i>0;i--)//找最老的那个点
			{
				if(gps_backup[i-1].type == GPS_FIXED)
				{	
				no_gsm_history = TRUE;
				no_gsm_history_from_year = gps_backup[i-1].year;
				no_gsm_history_from_month = gps_backup[i-1].month;
				no_gsm_history_from_day = gps_backup[i-1].day;
				no_gsm_history_from_hour = gps_backup[i-1].hour;
				no_gsm_history_from_min = gps_backup[i-1].minute;
				no_gsm_history_from_sec = gps_backup[i-1].second;       
				maintask_printf("from: %d,%d,%d,%d,%d,%d\r\n",no_gsm_history_from_year,no_gsm_history_from_month,no_gsm_history_from_day,no_gsm_history_from_hour,no_gsm_history_from_min,no_gsm_history_from_sec);
				break;
				}
			} 
			for(i=0;i<actual_n;i++) //找最新的那个点
			{
				if(gps_backup[i].type == GPS_FIXED)
				{	
				no_gsm_history_to_year = gps_backup[i].year;
				no_gsm_history_to_month = gps_backup[i].month;
				no_gsm_history_to_day = gps_backup[i].day;
				no_gsm_history_to_hour = gps_backup[i].hour;
				no_gsm_history_to_min = gps_backup[i].minute;
				no_gsm_history_to_sec = gps_backup[i].second;       
				maintask_printf("to: %d,%d,%d,%d,%d,%d\r\n",no_gsm_history_to_year,no_gsm_history_to_month,no_gsm_history_to_day,no_gsm_history_to_hour,no_gsm_history_to_min,no_gsm_history_to_sec);
				break;
				}
			}
		}
		maintask_printf("%d,%d,%d,%d,%d,%d  -- %d,%d,%d,%d,%d,%d\r\n",no_gsm_history_from_year,no_gsm_history_from_month,no_gsm_history_from_day,no_gsm_history_from_hour,no_gsm_history_from_min,no_gsm_history_from_sec,
		no_gsm_history_to_year,no_gsm_history_to_month,no_gsm_history_to_day,no_gsm_history_to_hour,no_gsm_history_to_min,no_gsm_history_to_sec);    	      
	}   
}     

U16 get_number_of_records_in_backup_requested_history_buffer(void)
{
	U16 number = 0;
	for(number = 0;number < GPS_BACKUP_MAX_NUMBER;number++)
  	{
  		if(gps_backup_fixed_history_buffer[number].type == GPS_EMPTY)
  			break;	
	}
	return number;
}

void stop_new_tracking_because_of_old_tracking(void)
{
	  if(g_trk_var.g_trk_settings.sample_time> 0)
	  {
	  	 maintask_printf("stop_new_tracking_because_of_old_tracking: new tracking on, stop it");
                //sample time
                g_trk_var.g_trk_settings.sample_time= 0;
                //report time
               g_trk_var.g_trk_settings.report_time = 0;
                //Location Accuracy
                g_trk_var.g_trk_settings.location_accuracy = 0;
                //setting
                g_trk_var.g_trk_settings.track_setting = 0;
                //tracker_refresh_settings_record();
	  	   tracking_with_measurement_stop();
	  }	
	  else
	  {
	  	 maintask_printf("stop_new_tracking_because_of_old_tracking: new tracking off, do nothing");
	  }	 	 
}

void stop_old_tracking_because_of_new_tracking(void)
{	  
	  if(g_trk_var.g_trk_settings.Xmode_smaple_time> 0)
	  {
	      	 maintask_printf("stop_old_tracking_because_of_new_tracking: old tracking on, stop it");
              g_trk_var.g_trk_settings.Xmode_smaple_time = 0;
              g_trk_var.g_trk_settings.Xmode_report_time = 0;
              g_trk_var.g_trk_settings.Xmode_location_accuracy=0;
              g_trk_var.g_trk_settings.Xmode_transmit_deviceid=0;
              g_trk_var.g_trk_settings.Xmode_delta_compression=0;
             // tracker_refresh_settings_record();
	  	 xmode_stop_all();
	  }	
	  else
	  {
	  	 maintask_printf("stop_old_tracking_because_of_new_tracking: old tracking off, do nothing");
	  }	 	 
}


BOOL compare_time_earlier(U8* time_1,U8* time_2)
{
    if (time_1[0] == time_2[0])//year
    {
        if (time_1[1] == time_2[1])//month
        {
            if (time_1[2] == time_2[2])//day
            {
                if (time_1[3] == time_2[3])//hour
                {
                    if (time_1[4] == time_2[4])//min
                    {
                        if (time_1[5] == time_2[5])
                        {
                            return TRUE;//一样
                        }
                        else
                        {
                            if (time_1[5] < time_2[5])
                            {
                                return TRUE;
                            }
                            else
                            {
                                return FALSE;
                            }
                        }
                    }
                    else
                    {
                        if (time_1[4] < time_2[4])
                        {
                            return TRUE;
                        }
                        else
                        {
                            return FALSE;
                        }
                    }
                }
                else
                {
                    if (time_1[3] < time_2[3])
                    {
                        return TRUE;
                    }
                    else
                    {
                        return FALSE;
                    }
                }
            }
            else
            {
                if (time_1[2] < time_2[2])
                {
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
        }
        else
        {
            if (time_1[1] < time_2[1])
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }
    else
    {
        if (time_1[0] < time_2[0])
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

}


BOOL InRequestedHistoryTime(void)
{
	if(TRUE == compare_time_earlier(get_history_time_from,history_current_time) &&
		 TRUE == compare_time_earlier(history_current_time,get_history_time_to))
	{
		 return TRUE;
	}
	else
	{
		 return FALSE;
	}
}		 		 	 

void fixed_backup_history_buffer_creat(void)
{
	 U16 i = 0;//可以为负，循环用
	 U16 j = 0;
  
	  //从后往前排，最老的要在最前面
	  for(i = (GPS_BACKUP_MAX_NUMBER-1);i>=0;i--)		
	  {
	  	 if(gps_backup[i].type == GPS_FIXED)
	  	 {
	  	 	  history_current_time[0] = gps_backup[i].year;
	  	 	  history_current_time[1] = gps_backup[i].month;
	  	 	  history_current_time[2] = gps_backup[i].day;
	  	 	  history_current_time[3] = gps_backup[i].hour;
	  	 	  history_current_time[4] = gps_backup[i].minute;
	  	 	  history_current_time[5] = gps_backup[i].second;
	  	 	  
	  	 	  //check 是不是在实践区间内
	  	 	  if(TRUE == InRequestedHistoryTime())
	  	 	  {
	  	 	  	 memset(gps_backup,0,GPS_BACKUP_MAX_NUMBER*sizeof(trackergps_backup_data_struct)); 
	  	 	  	 memcpy(&(gps_backup_fixed_history_buffer[j]),&(gps_backup[i]),sizeof(trackergps_backup_data_struct));
	  	 	  	 j++;
	  	 	  }	 	
	  	 }
	  }
} 


void fixed_backup_history_buffer_init(void)
{
	memset(gps_backup_fixed_history_buffer,0,GPS_BACKUP_MAX_NUMBER*sizeof(trackergps_backup_data_struct)); 
}	

//motion_count记录在GPS_GENERAL_SETUP_READ[27]里面
U8 get_motion_para1(void)
{
	  U8 motion_count = 0;
	  
	  if(g_trk_var.g_user_settings.motion_para_1 != 0 && g_trk_var.g_user_settings.motion_para_1 != 255)
    {	
       motion_count = g_trk_var.g_user_settings.motion_para_1;
    }
    else
    {
    	 motion_count = MOTION_COUNT_DEFAULT;
    }
   
    return motion_count;
}


//motion_filter记录在GPS_GENERAL_SETUP_READ[28]里面
U8 get_motion_para2(void)
{
    return  g_trk_var.g_user_settings.motion_para_2;
}

//16位filter的高8位

U8 get_motion_para3(void)
{
    return   g_trk_var.g_user_settings.motion_para_3;
}

//q timer记录在GPS_GENERAL_SETUP_READ[26]里面
U8 get_ussd_q_timer(void)
{
     U8 q_timer = 0;

    if(g_trk_var.g_user_settings.ussd_q_timer >= 10 && g_trk_var.g_user_settings.ussd_q_timer <= 60)//15改成4
    {
    	 //kal_prompt_trace(MOD_WAP,"USSD_Q_sending timer:%d sec\r\n",g_trk_var.g_user_settings.ussd_q_timer);
    	 q_timer = g_trk_var.g_user_settings.ussd_q_timer;
    }
    else
    {
    	// kal_prompt_trace(MOD_WAP,"USSD_Q_sending timer:15 sec,use defualt");
    	 q_timer = 10;
    }
    return q_timer;
}    


BOOL check_ack_setting_bit(U8 check_bit)	
{
	if((g_trk_var.g_trk_settings.gps_ack_setting[0] & check_bit) > 0)
	{
		maintask_printf("check_ack_setting_bit true\r\n");
		return TRUE;
	}
	else
	{
		maintask_printf("check_ack_setting_bit false\r\n");
		return FALSE;
	}		 			
}

int get_uint_from_tranformed_int(U8 abc)
{
	int ret = 0;
	switch(abc)
	{
		case 0x30:
			ret = 0;
			break;
			
		case 0x31:
			ret = 1;
			break;
			
		case 0x32:
			ret = 2;
			break;
			
		case 0x33:
			ret = 3;
			break;
			
		case 0x34:
			ret = 4;
			break;
			
		case 0x35:
			ret = 5;
			break;
			
		case 0x36:
			ret = 6;
			break;
			
		case 0x37:
			ret = 7;
			break;
			
		case 0x38:
			ret = 8;
			break;
			
		case 0x39:
			ret = 9;
			break;
			
		case 0x61:
			ret = 10;
			break;
			
		case 0x62:
			ret = 11;
			break;
			
		case 0x63:
			ret = 12;
			break;
			
		case 0x64:
			ret = 13;
			break;
			
		case 0x65:
			ret = 14;
			break;
			
		case 0x66:
			ret = 15;
			break;

		defult:
			ret = 0;
			break;
	}
	return ret;
}	 	

/************************************************************************
Xmode
para1:多长时间读一次，kal_uint8格式，0xff=0
para2:震动后多长时间开启backup，kal_uint8格式，0xff=0
para3:一次发多少个坐标，kal_uint8格式，0xff=0
para4：channel，char格式，‘0’：ussd，‘1’：data，‘2’：sms
para5：compression type，char格式，‘0’：compression_no,'1':compression_1,'2':compression_2,'3':compression_3
*************************************************************************/

void bcd_transform_back(char* str_ori,char* str_back)
{
	char buffer[20];
	char integer[20];
	char dot[20];
	memset(buffer,0,20);
	memset(dot,0,20);
	memset(integer,0,20);
	//先把每位都取出来
	buffer[0] = (str_ori[0] >> 4) & 0xf;
	buffer[1] = str_ori[0] & 0xf;
	buffer[2] = (str_ori[1] >> 4) & 0xf;
	buffer[3] = str_ori[1] & 0xf;
	buffer[4] = (str_ori[2] >> 4) & 0xf;
	buffer[5] = str_ori[2] & 0xf;
	buffer[6] = (str_ori[3] >> 4) & 0xf;
	buffer[7] = str_ori[3] & 0xf;
	buffer[8] = (str_ori[4] >> 4) & 0xf;
	buffer[9] = str_ori[4] & 0xf;
	//小数点
	dot[0] = '.';
	dot[1] = buffer[4] + 48;
	dot[2] = buffer[5] + 48;
	dot[3] = buffer[6] + 48;
	dot[4] = buffer[7] + 48;
	dot[5] = buffer[8] + 48;
	dot[6] = buffer[9] + 48;
	
	if(buffer[0] == 0)//正数
	{
		if(buffer[1] == 0 && buffer[2] == 0 && buffer[3] == 0)//3个全0
		{
			integer[0] = '0';
		}
		else if(buffer[1] == 0 && buffer[2] == 0)//个位
		{
			integer[0] = buffer[3] + 48;
		}
		else if(buffer[1] == 0)//十位
		{
			integer[0] = buffer[2] + 48;
			integer[1] = buffer[3] + 48;
		}	
		else//百位
		{
			integer[0] = buffer[1] + 48;
			integer[1] = buffer[2] + 48; 	
			integer[2] = buffer[3] + 48; 
		}
	}
	else//负数
	{
		integer[0] = '-';
		
		if(buffer[1] == 0 && buffer[2] == 0 && buffer[3] == 0)//3个全0
		{
			integer[1] = '0';
		}
		else if(buffer[1] == 0 && buffer[2] == 0)//个位
		{
			integer[1] = buffer[3] + 48;
		}
		else if(buffer[1] == 0)//十位
		{
			integer[1] = buffer[2] + 48;
			integer[2] = buffer[3] + 48;
		}	
		else//百位
		{
			integer[1] = buffer[1] + 48;
			integer[2] = buffer[2] + 48; 	
			integer[3] = buffer[3] + 48; 
		}
	}
	strcpy(str_back,integer);
	strcat(str_back,dot);
	maintask_printf("bcd back:%s -> %s",str_ori,str_back); 			 				 			
}			


//读出编码后一个字节的数值
U8 get_one_byte_uint_from_hex_asc(U8 bit4_0, U8 bit4_1)
{
	int ret=0;
	int a,b;
	a=get_uint_from_tranformed_int(bit4_0);
	b=get_uint_from_tranformed_int(bit4_1);
	ret = a*16 + b;//把最高control位滤掉
	maintask_printf("get_one_byte_uint_from_hex_asc = %d",ret); 
	return (U8)ret;
}

		
U8 bit_of_byte(U8 byte_1, U8 number_th)
{
	  U8 bit_ret = 0;
	  if(number_th > 7) 
	  {
	  	maintask_printf("bit_of_byte number_th wrong");
	  	return 0;
	  }
	  
	  switch(number_th)
	  {
	  	case 0:
	  	   bit_ret = byte_1 & 0x01;
	  	   break;
	  	 
	  	case 1:
	  	   bit_ret = byte_1 & 0x02;
	  	   break; 
	  	   
	  	case 2:
	  	   bit_ret = byte_1 & 0x04;
	  	   break; 
	  	  
	  	case 3:
	  	   bit_ret = byte_1 & 0x08;
	  	   break; 
	  	
	  	case 4:
	  	   bit_ret = byte_1 & 0x10;
	  	   break; 
	  	   
	  	case 5:
	  	   bit_ret = byte_1 & 0x20;
	  	   break; 
	  	   
	  	case 6:
	  	   bit_ret = byte_1 & 0x40;
	  	   break;                  
	  	   
	  	case 7:
	  	   bit_ret = byte_1 & 0x80;
	  	   break; 
	  	   
	  	default:
	  		break;
	 }
	 
	 if(bit_ret > 0)
	 	 return 1;
	 else
	 	 return 0;
}	


void plus_bcd_process(char* str, U8 length)
{
	char str_2[20];
	char final_bcd[6];
	U8 leng_1 = 0;
	U8 i=0;
	
	memset(str_2,0,20);
	memset(final_bcd,0,6);
	
	leng_1 = strlen(str);
	
	//整数只有一位，1.123456，变成0001123456
	if(leng_1 == 8)
	{
		str_2[0] = '0';
		str_2[1] = '0';
		str_2[2] = '0';
		str_2[3] = str[0];//跳过小数点
		str_2[4] = str[2];//后面是6位小数
		str_2[5] = str[3];
		str_2[6] = str[4];
		str_2[7] = str[5];
		str_2[8] = str[6];
		str_2[9] = str[7];
	}
	else if(leng_1 == 9)	//整数有2位，12.123456，变成0012123456
	{
		str_2[0] = '0';
		str_2[1] = '0';
		str_2[2] = str[0];
		str_2[3] = str[1];//跳过小数点
		str_2[4] = str[3];//后面是6位小数
		str_2[5] = str[4];
		str_2[6] = str[5];
		str_2[7] = str[6];
		str_2[8] = str[7];
		str_2[9] = str[8];
	}
	else if(leng_1 == 10) //整数有3位，113.123456，变成0113123456
	{
		str_2[0] = '0';
		str_2[1] = str[0];
		str_2[2] = str[1];
		str_2[3] = str[2];//跳过小数点
		str_2[4] = str[4];//后面是6位小数
		str_2[5] = str[5];
		str_2[6] = str[6];
		str_2[7] = str[7];
		str_2[8] = str[8];
		str_2[9] = str[9];
	}
	else
	{
		return;
	}	
	
	for(i=0;i<10;i++)
	{
			str_2[i] = str_2[i] - 48;//其他全变成数字
	}						
	
	//bcd编码
	final_bcd[0] = str_2[0]<<4 | str_2[1];
	final_bcd[1] = str_2[2]<<4 | str_2[3];
	final_bcd[2] = str_2[4]<<4 | str_2[5];
	final_bcd[3] = str_2[6]<<4 | str_2[7];
	final_bcd[4] = str_2[8]<<4 | str_2[9];
	memset(str,0,length);
	//有0,一个一个copy
	str[0] = final_bcd[0];
	str[1] = final_bcd[1];
	str[2] = final_bcd[2];
	str[3] = final_bcd[3];
	str[4] = final_bcd[4];

}	

//把负数的坐标转换成bcd编码，正数前4个bit为1
void minus_bcd_process(char* str, U8 length)
{
	char str_2[20];
	char final_bcd[6];
	U8 leng_1 = 0;
	U8 i=0;
	
	memset(str_2,0,20);
	memset(final_bcd,0,6);
	
	leng_1 = strlen(str);
	
	//整数只有一位，-1.123456，变成1001123456
	if(leng_1 == 9)
	{
		str_2[0] = '1';
		str_2[1] = '0';
		str_2[2] = '0';
		str_2[3] = str[1];//跳过小数点
		str_2[4] = str[3];//后面是6位小数
		str_2[5] = str[4];
		str_2[6] = str[5];
		str_2[7] = str[6];
		str_2[8] = str[7];
		str_2[9] = str[8];
	}
	else if(leng_1 == 10)	//整数有2位，-12.123456，变成1012123456
	{
		str_2[0] = '1';
		str_2[1] = '0';
		str_2[2] = str[1];
		str_2[3] = str[2];//跳过小数点
		str_2[4] = str[4];//后面是6位小数
		str_2[5] = str[5];
		str_2[6] = str[6];
		str_2[7] = str[7];
		str_2[8] = str[8];
		str_2[9] = str[9];
	}
	else if(leng_1 == 11) //整数有3位，-113.123456，变成1113123456
	{
		str_2[0] = '1';
		str_2[1] = str[1];
		str_2[2] = str[2];
		str_2[3] = str[3];//跳过小数点
		str_2[4] = str[5];//后面是6位小数
		str_2[5] = str[6];
		str_2[6] = str[7];
		str_2[7] = str[8];
		str_2[8] = str[9];
		str_2[9] = str[10];
	}
	else
	{
		return;
	}	
	for(i=0;i<10;i++)
	{
			str_2[i] = str_2[i] - 48;//其他全变成数字
	}			
	//bcd编码
	final_bcd[0] = str_2[0]<<4 | str_2[1];
	final_bcd[1] = str_2[2]<<4 | str_2[3];
	final_bcd[2] = str_2[4]<<4 | str_2[5];
	final_bcd[3] = str_2[6]<<4 | str_2[7];
	final_bcd[4] = str_2[8]<<4 | str_2[9];
	memset(str,0,length);
	//有0,一个一个copy
	str[0] = final_bcd[0];
	str[1] = final_bcd[1];
	str[2] = final_bcd[2];
	str[3] = final_bcd[3];
	str[4] = final_bcd[4];
}	
void compression_method_2_lat(double latitude, char* lat_1,U8 length)	
{
	double val_lat = latitude;
	double max_lat_val = 90;
	double scaled_val = 0;

	char log[200];
	double double_1=0;
	double uint64_1=0;
	double transmitted_val = 0;

	U32 final = 0;

	scaled_val = val_lat + (val_lat<0? 2*max_lat_val:0);
		
	double_1 = scaled_val/(2*max_lat_val);

	uint64_1 = 8388607;//2的23次方减1

	transmitted_val = uint64_1*double_1;

	//四舍五入
	final = (U32)(transmitted_val+0.5);

	memset(log,0,200);
	sprintf(log,"compression_method_2_lat ori:%f final:%d, %x",latitude,final,final);

	memset(lat_1, 0, length);

	lat_1[0] = final/65536;
	lat_1[1] = (final - (lat_1[0])*65536)/256;
	lat_1[2] = final - lat_1[0]*65536 - lat_1[1]*256;
  
}  

//低精度，用24位
void compression_method_2_long(double longitude, char* long_1,U8 length)	
{
	double val_long = longitude;
	double max_long_val = 180;
  	double scaled_val = 0;
 
  	char log[200];
  	double double_1=0;
 	double uint64_1=0;
  	double transmitted_val = 0;
 	U32 final = 0;
  	scaled_val = val_long + (val_long<0? 2*max_long_val:0);	
  	double_1 = scaled_val/(2*max_long_val);
  	uint64_1 = 16777215;//2
  	transmitted_val = uint64_1*double_1;
  	//四舍五入
  	final = (U32)(transmitted_val+0.5);
  	memset(log,0,200);
  	sprintf(log,"compression_method_2_long ori:%f final:%d, %x",longitude,final,final);
  	memset(long_1, 0, length);
 	long_1[0] = final/65536;
  	long_1[1] = (final - (long_1[0])*65536)/256;
  	long_1[2] = final - long_1[0]*65536 - long_1[1]*256; 
}  

//高精度，只用31位
void compression_method_3_lat(double latitude, char* lat_1,U8 length)	
{
	double val_lat = latitude;
	double max_lat_val = 90;
  	double scaled_val = 0;
 
	  char log[200];
	  double double_1=0;
	  double uint64_1=0;
	  double transmitted_val = 0;

	  U32 final = 0;
	    
	  scaled_val = val_lat + (val_lat<0? 2*max_lat_val:0);
	  	
	  double_1 = scaled_val/(2*max_lat_val);
	  
	  //uint64_1 = (256*256*256*128)-1;//2的31次方减1
	  uint64_1 = 2147483647;//2的31次方减1,必须直接写!!
	 
	  transmitted_val = uint64_1*double_1;
	  
	  //四舍五入
	  final = (U32)(transmitted_val+0.5);
	  
	  memset(log,0,200);
	  sprintf(log,"compression_method_3_lat ori:%f final:%d, %x",latitude,final,final);
	  
	  memset(lat_1, 0, length);
	  
	  lat_1[0] = final/(256*256*256);
	  lat_1[1] = (final - lat_1[0] * (256*256*256))/(256*256);
	  lat_1[2] = (final - lat_1[0] * (256*256*256) - lat_1[1] *(256*256))/256;
	  lat_1[3] = final - lat_1[0] * (256*256*256) - lat_1[1] *(256*256) - lat_1[2]*256; 
  
}  

//高精度，用32位
void compression_method_3_long(double longitude, char* long_1,U8 length)	
{
	  double val_long = longitude;
	  double max_long_val = 180;
	  double scaled_val = 0;
	 
	  char log[200];
	  double double_1=0;
	  double uint64_1=0;
	  double transmitted_val = 0;

	  U32 final = 0;
	    
	  scaled_val = val_long + (val_long<0? 2*max_long_val:0);
	  	
	  double_1 = scaled_val/(2*max_long_val);
	  
	  uint64_1 = 4294967295;//2的24次方减1
	  
	  transmitted_val = uint64_1*double_1;
	  
	  //四舍五入
	  final = (U32)(transmitted_val+0.5);
	  
	  memset(log,0,200);
	  sprintf(log,"compression_method_3_long ori:%f final:%d, %x",longitude,final,final);
	  
	  memset(long_1, 0, length);
	  
	  long_1[0] = final/(256*256*256);
	  long_1[1] = (final - long_1[0] * (256*256*256))/(256*256);
	  long_1[2] = (final - long_1[0] * (256*256*256) - long_1[1] *(256*256))/256;
	  long_1[3] = final - long_1[0] * (256*256*256) - long_1[1] *(256*256) - long_1[2]*256; 
  
}

void do_cordinates_compression(double latitude,double longitude, double speed, char compression_type)
{
	 //先清空
	 memset(compression_buffer,0,100);
	 
	 //不压缩，明文发送
	 if(compression_type == COMPRESSION_NO)
	 {
	    sprintf((char*)compression_buffer,"%f,%f,%.0f",latitude,longitude,speed);
	    maintask_printf("sss111111:%s,%s\r\n", compression_buffer);
	 }
	 else if(compression_type == COMPRESSION_1) //bcd压缩
	 {
	    char lat_1[20];
	    char long_1[20];
	 
	    memset(lat_1,0,20);
	    memset(long_1,0,20);	
	    
	    
	    //小数点后6位
	   // sprintf(lat_1,"%.6f",latitude);
	   // sprintf(long_1,"%.6f",longitude);
	    sprintf(lat_1,"%d.%6d",(int)latitude, (int)((latitude - (int)latitude) * 1000000));
	    sprintf(long_1,"%d.%6d",(int)longitude, (int)((longitude - (int)longitude) * 1000000));
	    maintask_printf("sss222222:%s,%s\r\n", lat_1, long_1);
	    if(latitude >= 0)
	    {	
	       plus_bcd_process(lat_1,20);
	    }
	    else
	    {
	    	 minus_bcd_process(lat_1,20);
	    }
	    
	    
	    if(longitude >= 0)	 	   
	    {	
	       plus_bcd_process(long_1,20);
	    }
	    else
	    {
	    	 minus_bcd_process(long_1,20);
	    } 
	    

	    
	    if(speed > 255)
	    	speed = 255;	
	    
	    compression_buffer[0] = lat_1[0];
	    compression_buffer[1] = lat_1[1];
	    compression_buffer[2] = lat_1[2];
	    compression_buffer[3] = lat_1[3];
	    compression_buffer[4] = lat_1[4];
	    
	    compression_buffer[5] = long_1[0];
	    compression_buffer[6] = long_1[1];
	    compression_buffer[7] = long_1[2];
	    compression_buffer[8] = long_1[3];
	    compression_buffer[9] = long_1[4];
	    
	    compression_buffer[10] = (U8) speed;
	    //maintask_printf("compression_buffer:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",compression_buffer[0],compression_buffer[1],compression_buffer[2],compression_buffer[3],compression_buffer[4],compression_buffer[5],compression_buffer[6],compression_buffer[7],compression_buffer[8],compression_buffer[9],compression_buffer[10],compression_buffer[11]);   
	 }
   	else if(compression_type == COMPRESSION_2) //特殊压缩,共7个字节，3个lat，3个long，1个speed  
        {
     	    char lat_1[20];
     	    char long_1[20];
     	    U8 u8_speed = 0;
     	    memset(lat_1,0,20);
     	    memset(long_1,0,20);	
     	    compression_method_2_lat(latitude, lat_1, 20);
     	    compression_method_2_long(longitude, long_1, 20);
     	    //速度最大255
     	    if(speed>255)
     	    	speed = 255;
     	    	
     	    u8_speed = speed;
     	
     	    //此处不能用string的函数，因为里面可能会有0
     	    compression_buffer[0] = lat_1[0]; 
     	    compression_buffer[1] = lat_1[1];
     	    compression_buffer[2] = lat_1[2];
     	    compression_buffer[3] = long_1[0];
     	    compression_buffer[4] = long_1[1];
     	    compression_buffer[5] = long_1[2];
     	    compression_buffer[6] = u8_speed;	
     	 }
	 else if(compression_type == COMPRESSION_3) //特殊压缩,共9个字节，4个lat，4个long，1个speed  
   	{
	    char lat_1[20];
	    char long_1[20];
	    
	    U8 u8_speed = 0;
	 
	    memset(lat_1,0,20);
	    memset(long_1,0,20);	
	    
	    
	    compression_method_3_lat(latitude, lat_1, 20);
	    compression_method_3_long(longitude, long_1, 20);
	    
	    //速度最大255
	    if(speed>255)
	    	speed = 255;
	    	
	    u8_speed = speed;
	    
	    //此处不能用string的函数，因为里面可能会有0
	    compression_buffer[0] = lat_1[0]; 
	    compression_buffer[1] = lat_1[1];
	    compression_buffer[2] = lat_1[2];
	    compression_buffer[3] = lat_1[3];
	    compression_buffer[4] = long_1[0];
	    compression_buffer[5] = long_1[1];
	    compression_buffer[6] = long_1[2];
	    compression_buffer[7] = long_1[3];
	    compression_buffer[8] = u8_speed;	    
	 }
}   	
void tracker_send_deivce_status(void)
{
      maintask_printf("tracker_send_deivce_status\r\n");
      tracker_protocol_send_deal(REPLY_HEADER_STATUS_REPORT_1,g_trk_var.g_user_settings.data_transfer_channel);
      tracker_protocol_send_deal(REPLY_HEADER_STATUS_REPORT_2,g_trk_var.g_user_settings.data_transfer_channel);      
      tracker_protocol_send_deal(REPLY_HEADER_STATUS_REPORT_3,g_trk_var.g_user_settings.data_transfer_channel);
      tracker_protocol_send_deal(REPLY_HEADER_STATUS_REPORT_4,g_trk_var.g_user_settings.data_transfer_channel);
      tracker_protocol_send_deal(REPLY_DYNAMIC_STATUS_REPORT,g_trk_var.g_user_settings.data_transfer_channel);
}
void single_gsm_gps_location_report(void)
{
	maintask_printf("single_gsm_gps_location_report\r\n");
	server_msg_analyze_get_gps_no_ack();
}

void tracker_sos_send(void)
{	
	key_longpress_event_send_to_server(0);
	//device id 0, 发status report1，里面有imei，让服务器发general setup下来
	if( g_trk_var.g_user_settings.deviceid[0]==0&&
	g_trk_var.g_user_settings.deviceid[1]==0&&
	g_trk_var.g_user_settings.deviceid[2]==0&&
	g_trk_var.g_user_settings.deviceid[3]==0)
	{
	maintask_printf("device id empty, send status report");
	tracker_protocol_send_deal(REPLY_HEADER_STATUS_REPORT_1,g_trk_var.g_user_settings.data_transfer_channel);
	}		 
}

static BOOL trackimo_init_receive_map(pfDefaultFunction * pDestArray, structReceiveMapFun * pSrcArray, int iSrcCount){
	int i;
	
	if((!pDestArray) || (!pSrcArray) || (iSrcCount <= 0))return FALSE;
	for(i = 0; i < iSrcCount; i++, pSrcArray++){
		pDestArray[pSrcArray->id] = pSrcArray->fun;
	}
	return TRUE;
}
static BOOL trackimo_init_send_map(pfTrackimoSendFunction * pDestArray, structSendMapFun * pSrcArray, int iSrcCount){
	int i;
	
	if((!pDestArray) || (!pSrcArray) || (iSrcCount <= 0))return FALSE;
	for(i = 0; i < iSrcCount; i++, pSrcArray++){
		pDestArray[pSrcArray->id] = pSrcArray->fun;
	}
	return TRUE;
}

command_type tracker_parsing_command_id(U8 bit4_0, U8 bit4_1)
{
	int ret=0;
	int a,b;


	a=get_uint_from_tranformed_int(bit4_0);
	b=get_uint_from_tranformed_int(bit4_1);


	ret = (a*16 + b)&0x7f;//把最高control位滤掉

	maintask_printf("tracker_parsing_command_id = %d\r\n",ret); 

	return (command_type)ret;
}

static BOOL tracker_check_msg_valid(tracker_struct_msg *pMsg)
{
	if(!pMsg)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
}

void trackimo_receive_deal(tracker_struct_msg *message)
{
	tracker_struct_msg *inmsg;
	command_type instruction_id = 0;
	unsigned int i,instruction_num;
	instruction_num = sizeof(static_receives)/sizeof(structReceiveMapFun);
	maintask_printf("mesage01=%d,mesage02=%d\r\n",message->body_msg[0],message->body_msg[1]);
	instruction_id=tracker_parsing_command_id(message->body_msg[0],message->body_msg[1]);
	inmsg = (tracker_struct_msg*)myMalloc(sizeof(tracker_struct_msg));
	memset(inmsg,0,sizeof(tracker_struct_msg));
	for (i=0;i<instruction_num;++i)
	{
	   if (instruction_id == static_receives[i].id)
	   {
	       maintask_printf("trackimo_receive_deal id %d\r\n",instruction_id);   
	       memcpy(inmsg,message,sizeof(tracker_struct_msg));
	       static_receives[i].fun(inmsg);
	       break;
	   }
	}
	//来任何指令都重置这个timer
	//heart_pulse_start();	
	myFree(inmsg);
}

//编码发送
void tracker_protocol_send_deal(int encode_id,int data_channel)
{
	unsigned int instruction_id = 0;
	unsigned char i = 0;
	unsigned char instruction_num;
    
	instruction_num = sizeof(static_sends)/sizeof(structSendMapFun);
    for (i=0;i<instruction_num;++i)
	{
		if (encode_id == static_sends[i].id)
		{
			maintask_printf("encode_id=%d data_channel=%d\r\n",encode_id,data_channel);
		    static_sends[i].fun(data_channel);   //编码
            break;
		}
	}
}

