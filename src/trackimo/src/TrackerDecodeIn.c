#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "MyCommon.h"
#include "TrackerCfg.h"
#include "My4G.h"
#include "TrackimoFrame.h"
#include "TrackerQueue.h"
#include "MyMemory.h"
#include "TrackerMain.h"
#include "MyTimer.h"
#include "MyBuzzer.h"
#include "MyLog.h"
#include "MyFile.h"
U8 rpy_set_active_carrier_channel = 0;
reply_channel rpy_get_carriers_channel=0xff;
#define MSG_START_TRACKING_MODE_LEN 28
#define MSG_STOP_TRACKING_MODE_LEN 4
#define MSG_TEST_MODE_LEN 6
#define MSG_DELETE_FILE_LEN 4
#define MSG_BEEP_LEN 8
#define MSG_BACKUP_ENABLE_LEN 6
#define MSG_CLEAR_ALL_ALARM_LEN 4
#define MSG_GET_HISTORY_LEN 30
#define MSG_VOICE_CALL_LEN 26
#define MSG_GET_CARRIERS_LEN 4
#define MSG_GET_GPS_LEN 8
#define MSG_FENCE_ALARM_LEN 58
#define MSG_FENCE_ALARM_ONOFF_LEN 8
#define MSG_SPEED_ALARM_LEN  20
#define MSG_MOVING_ALARM_LEN 18
#define MSG_BATTERY_ALARM_LEN 18
#define MSG_SET_SESSION_MAX_BYTES_LEN 6
#define MSG_MOTION_COUNT_LEN 8
#define MSG_USSD_Q_TIMER_LEN 6
#define MSG_AUTO_CHANGE_CARRIER_LEN 6
extern int auto_reboot_timer_in_hour;
extern reply_channel get_history_channel;
extern U8 get_history_time_from[6];
extern U8 get_history_time_to[6];
extern BOOL GET_HISTORY_PROCESSING;
/***************************************************************************************
	8 bits - Identification.
  8 bits - Ack Channel (0="USSD",1="SMS",2="Data")
	8 bits - "Tracking Channel" as in the old message.
	8 bits - "Sample Time (mins)" as in the old message.
	8 bits - "Report Time (mins)" as in the old message.
	8 bits - "Location Accuracy" as in the old message.
	1 bit - "Send Device ID" NOT AS IN THE OLD MESSAGE.
	1 bit - "Delta Compression" NOT AS IN THE OLD MESSAGE.
  1 bit - Send Altitude measurement
  1 bit - Send Battery measurement
	1 bit - Send Speed measurement
	1 bit - Sample time in minutes? (if not then it is in seconds)
	2 bits - Padding

********************************************************************/

//太长了，分成2条
void server_msg_analyze_status(tracker_struct_msg *message)
{
	//return; //****************测试
	if(check_ack_setting_bit(ACK_BIT_Status_Request) == TRUE)
	{
           g_trk_var.g_trk_content_info.command=COMMAND_STATUS;        
           tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
	}
       tracker_send_deivce_status();
}

//按button，服务器须返回ack
void server_msg_analyze_ack(tracker_struct_msg *message)
{
	resend_button_stop();
	#ifdef __SIM_DEFAULT_CHANNEL_DATA__
	main_printf(" ---- ACK ----");
	// check_ack_timer_restart();
	// data_no_ack_session_off_timer_stop(); 
	#endif
}	

void server_msg_analyze_tracking_mode_start(tracker_struct_msg *message)
{
       U8 ipadress[4]={0};
	U8 low_port=0;
	U8 high_port=0;	
	U16  ip_port=0;
     	g_trk_var.g_trk_content_info.command=COMMAND_TRACKING_MODE_START;
  	if(message->msgLen!= MSG_START_TRACKING_MODE_LEN)
  	{
              tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
  	       return;
  	}	
   	g_trk_var.g_trk_settings.Xmode_channel= get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]); 	
	#ifdef __SIM_DEFAULT_CHANNEL_DATA__
	//0-ussd,1-sms,2-data, tele2只能2
	g_trk_var.g_trk_settings.Xmode_channel= g_trk_var.g_user_settings.data_transfer_channel;	
	#endif 
  	if( g_trk_var.g_trk_settings.Xmode_channel>REPLY_CHANNEL_MAX)
	  {
	      g_trk_var.g_trk_settings.Xmode_channel = REPLY_CHANNEL_USSD;
	  }		
  
  	//ip
       ipadress[0] = get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]); 	
	ipadress[1]  = get_one_byte_uint_from_hex_asc(message->body_msg[8],message->body_msg[9]);
	ipadress[2]  = get_one_byte_uint_from_hex_asc(message->body_msg[10],message->body_msg[11]); 
	ipadress[3]  = get_one_byte_uint_from_hex_asc(message->body_msg[12],message->body_msg[13]);
	
	//port 
	high_port = get_one_byte_uint_from_hex_asc(message->body_msg[14],message->body_msg[15]);
	low_port = get_one_byte_uint_from_hex_asc(message->body_msg[16],message->body_msg[17]);
	
    	ip_port=high_port*256+low_port;
    
	if(ipadress[0]==0&&ipadress[1]==0&&ipadress[2]==0&&ipadress[3]==0) 
		{
		 	 ;//dont update
		}
	else
		{	
			 if(g_trk_var.g_user_settings.platform_url.host.ip[0]== ipadress[0] &&
	   	 	     g_trk_var.g_user_settings.platform_url.host.ip[1] == ipadress[1] &&
	   	 	     g_trk_var.g_user_settings.platform_url.host.ip[2] == ipadress[2] &&
	   	 	     g_trk_var.g_user_settings.platform_url.host.ip[3] == ipadress[3] &&
			     g_trk_var.g_user_settings.platform_url.port==ip_port)
			 {
			 	  ;//dont update
			 }
			 else
			 {		  	  
	                g_trk_var.g_user_settings.platform_url.host.ip[0]= ipadress[0];
	                g_trk_var.g_user_settings.platform_url.host.ip[1]=ipadress[1];
	                g_trk_var.g_user_settings.platform_url.host.ip[2] =ipadress[2];
	                g_trk_var.g_user_settings.platform_url.host.ip[3] =ipadress[3];
	                g_trk_var.g_user_settings.platform_url.port=ip_port;
	   	         //data_session_off();//更新了ip/port，要重连
	   	        }  
	   	 main_printf("ip:%d,%d,%d,%d:%d\r\n",g_trk_var.g_user_settings.platform_url.host.ip[0],
	                                                                                               g_trk_var.g_user_settings.platform_url.host.ip[1],
	                                                                                               g_trk_var.g_user_settings.platform_url.host.ip[2],
	                                                                                               g_trk_var.g_user_settings.platform_url.host.ip[3],
	                                                                                                g_trk_var.g_user_settings.platform_url.port);
	  	}		
  
 	 //多长时间读一次

   	g_trk_var.g_trk_settings.Xmode_smaple_time= get_one_byte_uint_from_hex_asc(message->body_msg[18],message->body_msg[19]);
  
  	//一次发几个坐标
   	g_trk_var.g_trk_settings.Xmode_report_time = get_one_byte_uint_from_hex_asc(message->body_msg[20],message->body_msg[21]);
  
  	//压缩，0:23bit低压缩，1：32bit，2：debug
   	g_trk_var.g_trk_settings.Xmode_location_accuracy= get_one_byte_uint_from_hex_asc(message->body_msg[22],message->body_msg[23]);
  	
  	//-	Transmit Device ID (0 – off/1 – on) (1 byte)
  	g_trk_var.g_trk_settings.Xmode_transmit_deviceid= get_one_byte_uint_from_hex_asc(message->body_msg[24],message->body_msg[25]);
  
 	 //-	Delta Compression (0 – off/1 – on) (1 byte)
  	g_trk_var.g_trk_settings.Xmode_delta_compression= get_one_byte_uint_from_hex_asc(message->body_msg[26],message->body_msg[27]);
  
 	 main_printf("server_msg_analyze_xmode %d,%d,%d,%d,%d\r\n",
                                                 g_trk_var.g_trk_settings.Xmode_smaple_time,
                                                 g_trk_var.g_trk_settings.Xmode_report_time ,
                                                 g_trk_var.g_trk_settings.Xmode_location_accuracy,
                                                 g_trk_var.g_trk_settings.Xmode_transmit_deviceid,
                                                 g_trk_var.g_trk_settings.Xmode_delta_compression);
  
 	 if(g_trk_var.g_trk_settings.Xmode_smaple_time>0)
  	{	
    	 stop_new_tracking_because_of_old_tracking();
  	}   
 	 //tracker_refresh_settings_record();
  	//5S后开启xmode
  	//stopMyTimer(MY_TIMER_50); 
  	//startMyTimer(MY_TIMER_50, 5*1000, XmodeStart);
  	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);  
}  

void server_msg_analyze_tracking_mode_stop(tracker_struct_msg *message)
{
    	g_trk_var.g_trk_content_info.command=COMMAND_TRACKING_MODE_STOP;
	if(message->msgLen!= MSG_STOP_TRACKING_MODE_LEN)
	{
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}	
   	 //多长时间读一次
       g_trk_var.g_trk_settings.Xmode_smaple_time = 0;
       g_trk_var.g_trk_settings.Xmode_report_time = 0;
       g_trk_var.g_trk_settings.Xmode_location_accuracy=0;
       g_trk_var.g_trk_settings.Xmode_transmit_deviceid=0;
       g_trk_var.g_trk_settings.Xmode_delta_compression=0;
  	//5S后开启xmode,这里会关闭gsensor
	//stopMyTimer(MY_TIMER_50);
	//startMyTimer(MY_TIMER_50, 2*1000, XmodeStart);

        g_trk_var.g_trk_settings.track_channel = g_trk_var.g_user_settings.data_transfer_channel;
        g_trk_var.g_trk_settings.sample_time = 0;
        g_trk_var.g_trk_settings.report_time=0;
        g_trk_var.g_trk_settings.location_accuracy=0;
        g_trk_var.g_trk_settings.track_setting=0;
	
       // tracker_refresh_settings_record();
	 tracking_with_measurement_stop();
        tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}  

void server_msg_analyze_general_setup(tracker_struct_msg *message)
{
        U8 ipadress[4]={0};
        U8 low_port=0;
        U8 high_port=0;  
        U16  ip_port=0; 
	 U8 wait_dur = 0;
	 U8 device_id0,device_id1,device_id2,device_id3;
	 U8 phone_number0,phone_number1,phone_number2,phone_number3,phone_number4,phone_number5,phone_number6,phone_number7,phone_number8,phone_number9;
	 U8 temp = 0;
	 
	 //0 - USSD/ 1 - SMS/ 2 – Data (1 byte)
	g_trk_var.g_user_settings.data_transfer_channel = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
	 
	 //UDP control IP (4 bytes)(a.b.c.d)
   	//UDP control port (2 bytes)(max = 65536)
        ipadress[0] = get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]); 	
	 ipadress[1]  = get_one_byte_uint_from_hex_asc(message->body_msg[8],message->body_msg[9]);
	 ipadress[2]  = get_one_byte_uint_from_hex_asc(message->body_msg[10],message->body_msg[11]); 
	 ipadress[3]  = get_one_byte_uint_from_hex_asc(message->body_msg[12],message->body_msg[13]);
	 high_port = get_one_byte_uint_from_hex_asc(message->body_msg[14],message->body_msg[15]);
	 low_port = get_one_byte_uint_from_hex_asc(message->body_msg[16],message->body_msg[17]);
    	 ip_port=high_port*256+low_port;
        if(ipadress[0]==0&&ipadress[1]==0&&ipadress[2]==0&&ipadress[3]==0) 
	{
	 	 ;//dont update
	}
	 else
	{	
		 if(g_trk_var.g_user_settings.platform_url.host.ip[0]== ipadress[0] &&
   	 	     g_trk_var.g_user_settings.platform_url.host.ip[1] == ipadress[1] &&
   	 	     g_trk_var.g_user_settings.platform_url.host.ip[2] == ipadress[2] &&
   	 	     g_trk_var.g_user_settings.platform_url.host.ip[3] == ipadress[3] &&
		     g_trk_var.g_user_settings.platform_url.port==ip_port)
		 {
		 	  ;//dont update
		 }
		 else
		 {		  	  
                     g_trk_var.g_user_settings.platform_url.host.ip[0]=ipadress[0];
                     g_trk_var.g_user_settings.platform_url.host.ip[1]=ipadress[1];
                     g_trk_var.g_user_settings.platform_url.host.ip[2]=ipadress[2];
                     g_trk_var.g_user_settings.platform_url.host.ip[3]=ipadress[3];
                     g_trk_var.g_user_settings.platform_url.port=ip_port;
        	       //data_session_off();//更新了ip/port，要重连
        	      // tracker_refresh_settings_record();
   	        }  
         	 main_printf("ip:%d,%d,%d,%d:%d\r\n",g_trk_var.g_user_settings.platform_url.host.ip[0],
                g_trk_var.g_user_settings.platform_url.host.ip[1],
                g_trk_var.g_user_settings.platform_url.host.ip[2],
                g_trk_var.g_user_settings.platform_url.host.ip[3],
                g_trk_var.g_user_settings.platform_url.port);
	 }   
		 //Device ID (4 bytes)
	 device_id0 = get_one_byte_uint_from_hex_asc(message->body_msg[18],message->body_msg[19]);
	 device_id1 = get_one_byte_uint_from_hex_asc(message->body_msg[20],message->body_msg[21]);
	 device_id2 = get_one_byte_uint_from_hex_asc(message->body_msg[22],message->body_msg[23]);
	 device_id3 = get_one_byte_uint_from_hex_asc(message->body_msg[24],message->body_msg[25]);
	 if(device_id0 == 0 && device_id1 == 0 && device_id2 == 0 && device_id3 == 0)
	 {
	 	  ;//do nothing
	 }
 	else
	{	
	#ifdef __TRACKER_LED_SUPPORT__
	if( g_trk_var.g_user_settings.deviceid[0] == 0 &&
	g_trk_var.g_user_settings.deviceid[1] == 0 &&
	g_trk_var.g_user_settings.deviceid[2] == 0 &&
	g_trk_var.g_user_settings.deviceid[3] == 0)
	{
	//write_device_id_from_empty_led();
	}		 
	#endif	  	 
	g_trk_var.g_user_settings.deviceid[0]= device_id0;
	g_trk_var.g_user_settings.deviceid[1]= device_id1;
	g_trk_var.g_user_settings.deviceid[2]= device_id2;
	g_trk_var.g_user_settings.deviceid[3]= device_id3;
	#ifdef __TRACKIMO_BLE__
	//Tracker_BLE_broadcast_name_change();
	#endif
	}   	 
	 //Phone number for Voice (20 digits - BCD)(10 bytes)
	 phone_number0 = get_one_byte_uint_from_hex_asc(message->body_msg[26],message->body_msg[27]);
	 phone_number1 = get_one_byte_uint_from_hex_asc(message->body_msg[28],message->body_msg[29]);
	 phone_number2 = get_one_byte_uint_from_hex_asc(message->body_msg[30],message->body_msg[31]);
	 phone_number3 = get_one_byte_uint_from_hex_asc(message->body_msg[32],message->body_msg[33]);
	 phone_number4 = get_one_byte_uint_from_hex_asc(message->body_msg[34],message->body_msg[35]);
	 phone_number5 = get_one_byte_uint_from_hex_asc(message->body_msg[36],message->body_msg[37]);
	 phone_number6 = get_one_byte_uint_from_hex_asc(message->body_msg[38],message->body_msg[39]);
	 phone_number7 = get_one_byte_uint_from_hex_asc(message->body_msg[40],message->body_msg[41]);
	 phone_number8 = get_one_byte_uint_from_hex_asc(message->body_msg[42],message->body_msg[43]);
	 phone_number9 = get_one_byte_uint_from_hex_asc(message->body_msg[44],message->body_msg[45]);
 
 	if(phone_number0==0 && phone_number1==0 && phone_number2==0 && phone_number3==0 && phone_number4==0 && phone_number5==0 && phone_number6==0 && phone_number7==0 && phone_number8==0 && phone_number9==0)
	 {	
	 	 ;//do nothing
	 }
 	else
	 {		 
	   	  g_trk_var.g_user_settings.tracker_phone_number[0] = phone_number0;
	   	  g_trk_var.g_user_settings.tracker_phone_number[1] = phone_number1;
	   	  g_trk_var.g_user_settings.tracker_phone_number[2] = phone_number2;
	   	  g_trk_var.g_user_settings.tracker_phone_number[3]= phone_number3;
	   	  g_trk_var.g_user_settings.tracker_phone_number[4] = phone_number4;
	   	  g_trk_var.g_user_settings.tracker_phone_number[5]= phone_number5;
	   	  g_trk_var.g_user_settings.tracker_phone_number[6]= phone_number6;
	   	  g_trk_var.g_user_settings.tracker_phone_number[7] = phone_number7;
	   	  g_trk_var.g_user_settings.tracker_phone_number[8] = phone_number8;
	   	  g_trk_var.g_user_settings.tracker_phone_number[9] = phone_number9;
  	 }	 	 
	 //注意：GPS_GENERAL_SETUP_READ[15]去用来做来电是否接听
 	wait_dur = get_one_byte_uint_from_hex_asc(message->body_msg[46],message->body_msg[47]); 
	 if(wait_dur > 0)
	 {	
	    g_trk_var.g_user_settings.ussd_lock_timer= wait_dur;
	 } 
	 //防止服务器犯错，GPS_MSG_WAIT_DUR_READ不能为0
	 if(g_trk_var.g_user_settings.ussd_lock_timer== 0)
	 	g_trk_var.g_user_settings.ussd_lock_timer=10;
 		 	
	 //call answer or not,0:answer,1:not answer,2:answer and send caller id,3:not implement
	 temp = get_one_byte_uint_from_hex_asc(message->body_msg[48],message->body_msg[49]); 
	 
	 if(temp >=0 && temp<= 3)
	 {
	 	 g_trk_var.g_user_settings.answer_mode = temp;
	 } 
	 //添加GPS on When Charging (1 byte) 0 - Off/ 1 – on
	 
	 /****************************
	 second bit
	 1: turn off red led after fully charged for 10 min.
        0: red led always on after fully charged.
   ******************************/
 	temp = get_one_byte_uint_from_hex_asc(message->body_msg[50],message->body_msg[51]);   //原来是GPS_GENERAL_SETUP_READ[16],
 	g_trk_var.g_user_settings.charge_powon_gps=bit_of_byte(temp,0);
 	g_trk_var.g_user_settings.full_battery_led_off=bit_of_byte(temp,1);
 	//gsm keep alive, in hour
	temp = get_one_byte_uint_from_hex_asc(message->body_msg[52],message->body_msg[53]);
 	g_trk_var.g_user_settings.ussd_heart_beat_in_hour = temp;
	 //data keep alive, in min
 	temp = get_one_byte_uint_from_hex_asc(message->body_msg[54],message->body_msg[55]); 
  	g_trk_var.g_user_settings.data_heart_beat_in_min = temp;
	 //beep enable
	 temp = get_one_byte_uint_from_hex_asc(message->body_msg[56],message->body_msg[57]); 
	 if(temp == 0 || temp == 1)
 	g_trk_var.g_user_settings.beeper_enable= temp;	
	//session start
	g_trk_var.g_user_settings.session_enable= get_one_byte_uint_from_hex_asc(message->body_msg[58],message->body_msg[59]); 
	//reboot timer
	g_trk_var.g_user_settings.reboot_timer = get_one_byte_uint_from_hex_asc(message->body_msg[60],message->body_msg[61]);
	auto_reboot_timer_in_hour=0;//重置reboot
	//10s内多少个中断认为是震动，可以重新开始tracking, alarm...
	g_trk_var.g_user_settings.tracking_alarm_in_10s= get_one_byte_uint_from_hex_asc(message->body_msg[62],message->body_msg[63]);
	//防止服务器犯错
	if( g_trk_var.g_user_settings.tracking_alarm_in_10s > 20)
	g_trk_var.g_user_settings.tracking_alarm_in_10s = 20;
	//heart pulse, 多长时间发一次心跳包，单位小时
	g_trk_var.g_user_settings.tracking_heart_pulse= get_one_byte_uint_from_hex_asc(message->body_msg[64],message->body_msg[65]);
	heart_pulse_start();	
	//如果心跳包没回复，关机几分钟开机，单位分钟
	g_trk_var.g_user_settings.no_heart_pulse_reboot_in_min = get_one_byte_uint_from_hex_asc(message->body_msg[66],message->body_msg[67]);
	//如果连续n个ussd send fail，reboot
	g_trk_var.g_user_settings.send_ussd_fail_num= get_one_byte_uint_from_hex_asc(message->body_msg[68],message->body_msg[69]);
	//ussd q timer
	g_trk_var.g_user_settings.ussd_q_timer = get_one_byte_uint_from_hex_asc(message->body_msg[70],message->body_msg[71]);
	//motion para 1
	g_trk_var.g_user_settings.motion_para_1 = get_one_byte_uint_from_hex_asc(message->body_msg[72],message->body_msg[73]);
	//motion para 2
	g_trk_var.g_user_settings.motion_para_2= get_one_byte_uint_from_hex_asc(message->body_msg[74],message->body_msg[75]);
	//motion para 3, 16位filter，高8位
	g_trk_var.g_user_settings.motion_para_3= get_one_byte_uint_from_hex_asc(message->body_msg[76],message->body_msg[77]);
	//Button Press ACK Timeout (seconds)(1 byte)
	g_trk_var.g_user_settings.press_ack_time_out_in_sec= get_one_byte_uint_from_hex_asc(message->body_msg[78],message->body_msg[79]);
	//tracker_refresh_comm_user_settings_record();
	g_trk_var.g_trk_content_info.command=COMMAND_GENERAL_SETUP;
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}

void server_msg_analyze_testmode(tracker_struct_msg *message)
{
        g_trk_var.g_trk_content_info.command=COMMAND_TESTMODE;
	 if(message->msgLen != MSG_TEST_MODE_LEN)
	 {
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	 }	
	 
	 if(message->body_msg[5] == '1')
	 {
		main_printf("server_msg_analyze_testmode on");
		//BB05_TEST_MODE = KAL_TRUE;
		#ifdef SUPPORT_MY_BUZZER 
		//	 BeepAlert(30);//短音
		#else
		//srv_prof_play_tone(SUCCESS_TONE, NULL);
		#endif
	 }
	 else if(message->body_msg[5] == '0')
	 {
		main_printf("server_msg_analyze_testmode off");
		//do_testmode_off();
	 }
	 else //格式不对
	 {
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;	
	 }	  

	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}

void server_msg_analyze_delete_file(tracker_struct_msg *message)
{
	g_trk_var.g_trk_content_info.command=COMMAND_DELETE_FILE;
	if(message->msgLen != MSG_DELETE_FILE_LEN)
	{            
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}	
	//Factory_Delete_gps_file();
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);

}

void server_msg_analyze_beep(tracker_struct_msg *message)
{
	 U8 beep1 = 0;
	 if(message->msgLen != MSG_BEEP_LEN)
	 {
		g_trk_var.g_trk_content_info.command=COMMAND_BEEP;
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	 }	
	 beep1 = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]); 
	 if(message->body_msg[7] == '1')
	 {
		#ifdef SUPPORT_MY_BUZZER 
		main_printf("beep!beep!beep1!!!!!");
		buzzer_power_on();
		#else
		//srv_prof_play_tone(SUCCESS_TONE, NULL);
		//speaker_play_beep(beep1,1);
		#endif
	 }
	 else if(message->body_msg[7] == '0')
	 {
	 	main_printf("beep!beep!beep0!!!!!");
		#ifdef SUPPORT_MY_BUZZER 
		buzzer_power_on();
		#else
		//srv_prof_play_tone(ERROR_TONE, NULL);
		//speaker_play_beep(beep1,0);
		#endif
	 }
	 else //格式不对
	 {
		g_trk_var.g_trk_content_info.command=COMMAND_BEEP;
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	 }	  
    g_trk_var.g_trk_content_info.command=COMMAND_BEEP;
    tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}



void server_msg_analyze_reboot(tracker_struct_msg *message)
{
	U8 reboot_para = 0;
	g_trk_var.g_trk_content_info.command=COMMAND_REBOOT;
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
	//timer min
	reboot_para = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
	main_printf("server_msg_analyze_reboot %d",reboot_para);
	//tracker_write_power_up_sig(2);	
	if(reboot_para == 0)//0min，直接reset
	{ 
		  g_trk_var.g_trk_status.shutdown_reason=2;
		  g_trk_var.g_trk_status.wake_total_min=0;
		  tracker_protocol_send_deal(REPLY_HEADER_SHUTDOWN,g_trk_var.g_user_settings.data_transfer_channel);
		  tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
		 // ResetOrderDo();
	}
	else
	{
		//至少2分钟
		if(reboot_para == 1)
		reboot_para = 2;
		g_trk_var.g_trk_status.shutdown_reason=2;
		g_trk_var.g_trk_status.wake_total_min=reboot_para;
		tracker_protocol_send_deal(REPLY_HEADER_SHUTDOWN,g_trk_var.g_user_settings.data_transfer_channel);
		tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
		//set_auto_pwron_time(reboot_para);
		//do_power_off();
	} 
}

void server_msg_analyze_backup_enable(tracker_struct_msg *message)
{
     g_trk_var.g_trk_content_info.command=COMMAND_BACKUP_ENABLE;
     if(message->msgLen != MSG_BACKUP_ENABLE_LEN)
	 {
              tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	       return;
	 }	
	g_trk_var.g_user_settings.gps_backup_enable = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
      //tracker_refresh_comm_user_settings_record();
      main_printf("server_msg_analyze_backup_enable %d",g_trk_var.g_user_settings.gps_backup_enable);              
      tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}
		   
/**********************************************
总共10个byte
Timer1 (1 byte),
Timer2 (1 byte),

前2个byte是以前2g用的

fence_speed_timer (1 byte in sec)
gps_on_cpu_sleep_timer (1 byte in sec/10, 55 is 5.5sec)
gps_on_cpu_wake_timer (1 byte in sec/10, 15 is 1.5sec)
indoor_checking_condition_sat_number (1 byte)
indoor_checking_condition_sat_max_signal (1 byte)

***********************************************/
//#define MSG_GPS_SEARCHMODE_LEN 8
void server_msg_analyze_gps_search_mode(tracker_struct_msg *message)
{
	 //老的不动
       g_trk_var.g_trk_settings.gps_search_control[0] = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);	 
	g_trk_var.g_trk_settings.gps_search_control[1]  = get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]); 
	 //后加
	 //fence_speed_timer (1 byte in sec) 多少秒看一次fence speed
	 g_trk_var.g_trk_settings.gps_search_control[2]  = get_one_byte_uint_from_hex_asc(message->body_msg[8],message->body_msg[9]);
	 //gps_on_cpu_sleep_timer (1 byte in sec/10, 55 is 5.5sec)，关gsm时间
	 g_trk_var.g_trk_settings.gps_search_control[3] = get_one_byte_uint_from_hex_asc(message->body_msg[10],message->body_msg[11]);
	 
	 //gps_on_cpu_wake_timer,开gsm时间
	 g_trk_var.g_trk_settings.gps_search_control[4]  = get_one_byte_uint_from_hex_asc(message->body_msg[12],message->body_msg[13]);
	 
	 //indoor_checking_condition_sat_number indoor最大卫星数
	 g_trk_var.g_trk_settings.gps_search_control[5]  = get_one_byte_uint_from_hex_asc(message->body_msg[14],message->body_msg[15]);
	 
	 //indoor_checking_condition_sat_max_signal (1 byte) indoor最大信号强度
	 g_trk_var.g_trk_settings.gps_search_control[6]  = get_one_byte_uint_from_hex_asc(message->body_msg[16],message->body_msg[17]); 
	 //tracking speed超过多少可以关gps
	 g_trk_var.g_trk_settings.gps_search_control[7]  = get_one_byte_uint_from_hex_asc(message->body_msg[18],message->body_msg[19]); 	 	 
	 main_printf("server_msg_analyze_gps_search_mode %d,%d,%d,%d,%d,%d,%d,%d\n",
                                                        g_trk_var.g_trk_settings.gps_search_control[0],
                                                        g_trk_var.g_trk_settings.gps_search_control[1],
                                                        g_trk_var.g_trk_settings.gps_search_control[2],
                                                        g_trk_var.g_trk_settings.gps_search_control[3],
                                                        g_trk_var.g_trk_settings.gps_search_control[4],
                                                        g_trk_var.g_trk_settings.gps_search_control[5],
                                                        g_trk_var.g_trk_settings.gps_search_control[6],
                                                        g_trk_var.g_trk_settings.gps_search_control[7]);
	 
   	//tracker_refresh_settings_record();
   	g_trk_var.g_trk_content_info.command=COMMAND_GPS_SEARCHMODE;
  	 tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}	 

void server_msg_analyze_clear_all_alarm(tracker_struct_msg *message)
{
	int i=0;
	g_trk_var.g_trk_content_info.command=COMMAND_CLEAR_ALL_ALARMS;
	if(message->msgLen != MSG_CLEAR_ALL_ALARM_LEN)
	{            
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}
	memset(g_trk_var.g_trk_settings.gps_fance_cordinates,0,20*20);
	memset(g_trk_var.g_trk_settings.gps_fance_alarm_setting,0,5*10);
	memset(g_trk_var.g_trk_settings.gps_speed_alarm_setting,0,5*10);
	memset(g_trk_var.g_trk_settings.gps_battery_alarm_setting,0,5*10);
	for(i=0;i<7;i++)
	{
	   g_trk_var.g_trk_settings.gps_moving_alarm_setting[i]=0;
	}
	for(i=0;i<7;i++)
	{
	   g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[i]=0;
	}
	smart_moving_alarm_start();
	speed_para_reset();
	fence_para_reset();
	bat_alarm_sent_sig_reset();
	fence_speed_operation_reset();
	check_fence_or_speed_start(); 
	//tracker_refresh_settings_record();
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);

}

void server_msg_analyze_get_history(tracker_struct_msg *message)
{
	U8 get_history_time_from_tmp[6] = {0,0,0,0,0,0};
	U8 get_history_time_to_tmp[6] = {0,0,0,0,0,0};
	g_trk_var.g_trk_content_info.command=COMMAND_GET_HISTORY;
	if(message->msgLen != MSG_GET_HISTORY_LEN)
	{
		main_printf("server_msg_analyze_get_history len wrong!");            
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}
	//from year
	get_history_time_from_tmp[0] = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);

	//from	month
	get_history_time_from_tmp[1] = get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]);

	//from day
	get_history_time_from_tmp[2] = get_one_byte_uint_from_hex_asc(message->body_msg[8],message->body_msg[9]); 

	//from hour
	get_history_time_from_tmp[3] = get_one_byte_uint_from_hex_asc(message->body_msg[10],message->body_msg[11]);

	//from min
	get_history_time_from_tmp[4] = get_one_byte_uint_from_hex_asc(message->body_msg[12],message->body_msg[13]);

	//from sec
	get_history_time_from_tmp[5] = get_one_byte_uint_from_hex_asc(message->body_msg[14],message->body_msg[15]); 


	//to year
	get_history_time_to_tmp[0] = get_one_byte_uint_from_hex_asc(message->body_msg[16],message->body_msg[17]);

	//to month
	get_history_time_to_tmp[1] = get_one_byte_uint_from_hex_asc(message->body_msg[18],message->body_msg[19]); 

	//to day
	get_history_time_to_tmp[2] = get_one_byte_uint_from_hex_asc(message->body_msg[20],message->body_msg[21]); 

	//to hour
	get_history_time_to_tmp[3] = get_one_byte_uint_from_hex_asc(message->body_msg[22],message->body_msg[23]); 

	//to min
	get_history_time_to_tmp[4] = get_one_byte_uint_from_hex_asc(message->body_msg[24],message->body_msg[25]); 

	//to sec
	get_history_time_to_tmp[5] = get_one_byte_uint_from_hex_asc(message->body_msg[26],message->body_msg[27]); 

	//channel
	get_history_channel = get_one_byte_uint_from_hex_asc(message->body_msg[28],message->body_msg[29]); 

	//year
	if(get_history_time_from_tmp[0] < 13 || get_history_time_to_tmp[0] < 13)
	{
	main_printf("server_msg_analyze_get_history year wrong!");

	g_trk_var.g_trk_content_info.command=COMMAND_GET_HISTORY;
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}

	//month
	if((get_history_time_from_tmp[1] < 1 || get_history_time_from_tmp[1] > 12)  || 
	(get_history_time_to_tmp[1] < 1 || get_history_time_to_tmp[1] > 12) )
	{
	main_printf("server_msg_analyze_get_history month wrong!");	   		
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}	 

	//day
	if((get_history_time_from_tmp[2] < 1 || get_history_time_from_tmp[2] > 31)  || 
	(get_history_time_to_tmp[2] < 1 || get_history_time_to_tmp[2] > 31) )
	{	
	main_printf("server_msg_analyze_get_history day wrong!");	   		
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}	

	//hour
	if( get_history_time_from_tmp[3] > 24  ||  get_history_time_to_tmp[3] > 24)
	{	 
	main_printf("server_msg_analyze_get_history hour wrong!");	  		
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}	

	//min
	if( get_history_time_from_tmp[4] > 60  ||  get_history_time_to_tmp[4] > 60)
	{	
	main_printf("server_msg_analyze_get_history min wrong!");	   		
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}	  

	//sec
	if( get_history_time_from_tmp[5] > 60  ||  get_history_time_to_tmp[5] > 60)
	{
	main_printf("server_msg_analyze_get_history sec wrong!");		   		
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}	

	if(GET_HISTORY_PROCESSING == TRUE)
	{
	main_printf("server_msg_analyze_get_history GET_HISTORY_PROCESSING KAL_TRUE,return error");		   		
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}		 	

	//数据ok
	get_history_time_from[0] = get_history_time_from_tmp[0]; 
	get_history_time_from[1] = get_history_time_from_tmp[1]; 
	get_history_time_from[2] = get_history_time_from_tmp[2]; 
	get_history_time_from[3] = get_history_time_from_tmp[3]; 
	get_history_time_from[4] = get_history_time_from_tmp[4]; 
	get_history_time_from[5] = get_history_time_from_tmp[5]; 

	get_history_time_to[0] = get_history_time_to_tmp[0];
	get_history_time_to[1] = get_history_time_to_tmp[1];
	get_history_time_to[2] = get_history_time_to_tmp[2];
	get_history_time_to[3] = get_history_time_to_tmp[3];
	get_history_time_to[4] = get_history_time_to_tmp[4];
	get_history_time_to[5] = get_history_time_to_tmp[5];

	main_printf("get_history from %d:%d:%d:%d:%d:%d to %d:%d:%d:%d:%d:%d,channel:%d",get_history_time_from[0],get_history_time_from[1],get_history_time_from[2],get_history_time_from[3],get_history_time_from[4],
	get_history_time_from[5],get_history_time_to[0],get_history_time_to[1],get_history_time_to[2],get_history_time_to[3],get_history_time_to[4],get_history_time_to[5],get_history_channel);

	GET_HISTORY_PROCESSING = TRUE;

	if(get_history_channel==0XFF)
	get_history_channel=g_trk_var.g_user_settings.data_transfer_channel;

	tracker_protocol_send_deal(REPLY_HEADER_HISTORYREPORT,get_history_channel);

	if(check_ack_setting_bit(ACK_BIT_Get_History) == TRUE)
	{
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
	}
} 

void server_msg_analyze_voice_call(tracker_struct_msg *message)
{
	g_trk_var.g_trk_content_info.command=COMMAND_VOICE_CALL;
	if(message->msgLen != MSG_VOICE_CALL_LEN)
	{
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}

	//注意：GPS_GENERAL_SETUP_READ[15]去用来做来电是否接听
	g_trk_var.g_user_settings.answer_mode  = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);

	//Phone number for Voice (20 digits - BCD)(10 bytes)
	g_trk_var.g_user_settings.tracker_phone_number[0] = get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]);
	g_trk_var.g_user_settings.tracker_phone_number[1]  = get_one_byte_uint_from_hex_asc(message->body_msg[8],message->body_msg[9]);
	g_trk_var.g_user_settings.tracker_phone_number[2] = get_one_byte_uint_from_hex_asc(message->body_msg[10],message->body_msg[11]);
	g_trk_var.g_user_settings.tracker_phone_number[3] = get_one_byte_uint_from_hex_asc(message->body_msg[12],message->body_msg[13]);
	g_trk_var.g_user_settings.tracker_phone_number[4]  = get_one_byte_uint_from_hex_asc(message->body_msg[14],message->body_msg[15]);
	g_trk_var.g_user_settings.tracker_phone_number[5]  = get_one_byte_uint_from_hex_asc(message->body_msg[16],message->body_msg[17]);
	g_trk_var.g_user_settings.tracker_phone_number[6]  = get_one_byte_uint_from_hex_asc(message->body_msg[18],message->body_msg[19]);
	g_trk_var.g_user_settings.tracker_phone_number[7] = get_one_byte_uint_from_hex_asc(message->body_msg[20],message->body_msg[21]);
	g_trk_var.g_user_settings.tracker_phone_number[8] = get_one_byte_uint_from_hex_asc(message->body_msg[22],message->body_msg[23]);
	g_trk_var.g_user_settings.tracker_phone_number[9]  = get_one_byte_uint_from_hex_asc(message->body_msg[24],message->body_msg[25]);
	//tracker_refresh_comm_user_settings_record();
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}


void server_msg_analyze_get_carriers(tracker_struct_msg *message)
{
	g_trk_var.g_trk_content_info.command=COMMAND_GET_CARRIERS;
	if(message->msgLen != MSG_GET_CARRIERS_LEN)
	{            
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}
	rpy_get_carriers_channel = get_one_byte_uint_from_hex_asc(message->body_msg[2],message->body_msg[3]);
	main_printf("rpy_get_carriers_channel = %d",rpy_get_carriers_channel);
	//check_operator_in_view();
	if(check_ack_setting_bit(ACK_BIT_Get_Carriers) == TRUE)
	{
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
	}
}

void server_msg_analyze_get_gps(tracker_struct_msg *message)//只支持bcd!
{
	U8 force_gps_read = 0;
	U8 gsm_location = 0;
	g_trk_var.g_trk_content_info.command=COMMAND_GET_GPS;
	if(message->msgLen != MSG_GET_GPS_LEN)
	{
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}
	//Force GPS read: 0 – Off/1- On
	force_gps_read = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
	//Send GSM before Lock (1 byte) 0 – Off/ 1 - On
	gsm_location = get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]);
	//如果正在fix，直接用
	if(TRUE == gps_2d_3d_fix_with_hdop_and_fix_sec(10))
	{ 	
	main_printf("GPS ON AND FIXED!");
	tracker_protocol_send_deal(REPLY_HEADER_GET_GPS,g_trk_var.g_user_settings.data_transfer_channel);
	}
	else //没fix,开gps
	{
	if(gsm_location == 1)//发gsm先
	{
	//get_gps_send_gsm_1();
	#ifdef SUPPORT_MY_WIFI
	tracker_read_wifi_and_send(WIFI_READ_FROM_GET_GPS);
	#endif
	}
	BACKEND_GPS_POWERON_SPECIAL();
	}
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}  

void server_msg_analyze_get_gps_no_ack(void)//只支持bcd!
{ 
	 //清空buffer
 	 //data要加device id 
	 //如果正在fix，直接用
  	if(TRUE == gps_2d_3d_fix_with_hdop_and_fix_sec(10))
	{ 	
		main_printf("GPS ON AND FIXED\r\n");
		tracker_protocol_send_deal(REPLY_HEADER_GET_GPS,g_trk_var.g_user_settings.data_transfer_channel);
	}
   	else //没fix,开gps
	{
		#ifdef SUPPORT_MY_WIFI
		tracker_read_wifi_and_send(WIFI_READ_FROM_GET_GPS);
		#endif
		//防止读wifi失败导致什么都没发
		//StopTimer(TRACKER_WIFI_TIMER_2);
		//StartTimer(TRACKER_WIFI_TIMER_2, 5000, get_gps_send_gsm);
		BACKEND_GPS_POWERON_SPECIAL();
	}
} 

void server_msg_analyze_fence_alarm(tracker_struct_msg *message)
{
	U8 i=0;
	U8 fence_number = 0;
	U8 para0,para1,para2,para3,para4,para5,para6;
	char lat1[20];
	char lon1[20];
	char lat2[20];
	char lon2[20];
	//转换回来
	char lat1_back[20];
	char lon1_back[20];
	char lat2_back[20];
	char lon2_back[20];
	memset(lat1_back,0,20);
	memset(lon1_back,0,20);
	memset(lat2_back,0,20);
	memset(lon2_back,0,20);
	memset(lat1,0,20);
	memset(lon1,0,20);
	memset(lat2,0,20);
	memset(lon2,0,20);
	g_trk_var.g_trk_content_info.command=COMMAND_FENCE_ALARM;
	if(message->msgLen != MSG_FENCE_ALARM_LEN)
	{
	main_printf("server_msg_analyze_fence_alarm length wrong:%d",message->msgLen);            
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}
	//先要读出第几个fence
	fence_number = get_one_byte_uint_from_hex_asc(message->body_msg[14],message->body_msg[15]);
	if(fence_number<1 || fence_number>5)
	{
	main_printf("server_msg_analyze_fence_alarm fence_number wrong:%d",fence_number);
	g_trk_var.g_trk_content_info.command=COMMAND_FENCE_ALARM;
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}			
	//cooldown(min)
	para0 = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
	//Sample Rate (mins) (1 byte)//add
	para1 = get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]);
	//alarm id
	para2 = get_one_byte_uint_from_hex_asc(message->body_msg[8],message->body_msg[9]); 	
	//alarm channel
	para3 = get_one_byte_uint_from_hex_asc(message->body_msg[10],message->body_msg[11]);
	//alarm on/off
	para4 = get_one_byte_uint_from_hex_asc(message->body_msg[12],message->body_msg[13]);
	//alarm number,1-5
	para5 = get_one_byte_uint_from_hex_asc(message->body_msg[14],message->body_msg[15]);
	//Lat1(5bytes)
	lat1[0] = get_one_byte_uint_from_hex_asc(message->body_msg[16],message->body_msg[17]);
	lat1[1] = get_one_byte_uint_from_hex_asc(message->body_msg[18],message->body_msg[19]);
	lat1[2] = get_one_byte_uint_from_hex_asc(message->body_msg[20],message->body_msg[21]);
	lat1[3] = get_one_byte_uint_from_hex_asc(message->body_msg[22],message->body_msg[23]);
	lat1[4] = get_one_byte_uint_from_hex_asc(message->body_msg[24],message->body_msg[25]);
	//Lon1(5bytes)
	lon1[0] = get_one_byte_uint_from_hex_asc(message->body_msg[26],message->body_msg[27]);
	lon1[1] = get_one_byte_uint_from_hex_asc(message->body_msg[28],message->body_msg[29]);
	lon1[2] = get_one_byte_uint_from_hex_asc(message->body_msg[30],message->body_msg[31]);
	lon1[3] = get_one_byte_uint_from_hex_asc(message->body_msg[32],message->body_msg[33]);
	lon1[4] = get_one_byte_uint_from_hex_asc(message->body_msg[34],message->body_msg[35]);
	//Lat2(5bytes)
	lat2[0] = get_one_byte_uint_from_hex_asc(message->body_msg[36],message->body_msg[37]);
	lat2[1] = get_one_byte_uint_from_hex_asc(message->body_msg[38],message->body_msg[39]);
	lat2[2] = get_one_byte_uint_from_hex_asc(message->body_msg[40],message->body_msg[41]);
	lat2[3] = get_one_byte_uint_from_hex_asc(message->body_msg[42],message->body_msg[43]);
	lat2[4] = get_one_byte_uint_from_hex_asc(message->body_msg[44],message->body_msg[45]);
	//Lon2(5bytes)
	lon2[0] = get_one_byte_uint_from_hex_asc(message->body_msg[46],message->body_msg[47]);
	lon2[1] = get_one_byte_uint_from_hex_asc(message->body_msg[48],message->body_msg[49]);
	lon2[2] = get_one_byte_uint_from_hex_asc(message->body_msg[50],message->body_msg[51]);
	lon2[3] = get_one_byte_uint_from_hex_asc(message->body_msg[52],message->body_msg[53]);
	lon2[4] = get_one_byte_uint_from_hex_asc(message->body_msg[54],message->body_msg[55]);	

	//把bcd的str转成原型，str
	bcd_transform_back(lat1,lat1_back);
	bcd_transform_back(lon1,lon1_back);
	bcd_transform_back(lat2,lat2_back);
	bcd_transform_back(lon2,lon2_back);
	//Trigger direction,跳20个byte，乘2
	para6 = get_one_byte_uint_from_hex_asc(message->body_msg[56],message->body_msg[57]);
	//容错
	if(para3 != 0xff && para3 != 0x0 && para3 != 0x1 && para3 != 0x2)
	{
	main_printf("server_msg_analyze_fence_alarm alarm channel wrong:%d",para3);
	g_trk_var.g_trk_content_info.command=COMMAND_FENCE_ALARM;
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}
	if(para4 > 1)
	{ 
	main_printf("server_msg_analyze_fence_alarm onoff wrong:%d",para4);
	g_trk_var.g_trk_content_info.command=COMMAND_FENCE_ALARM;
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}	 
	if(para6 > 2)
	{ 
	main_printf("server_msg_analyze_fence_alarm triger wrong:%d",para6);
	g_trk_var.g_trk_content_info.command=COMMAND_FENCE_ALARM;
	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	return;
	}
	//数据OK
	g_trk_var.g_trk_settings.gps_fance_alarm_setting[fence_number-1][0] = para0;
	g_trk_var.g_trk_settings.gps_fance_alarm_setting[fence_number-1][1] = para1;
	g_trk_var.g_trk_settings.gps_fance_alarm_setting[fence_number-1][2] = para2;
	g_trk_var.g_trk_settings.gps_fance_alarm_setting[fence_number-1][3] = para3;
	g_trk_var.g_trk_settings.gps_fance_alarm_setting[fence_number-1][4] = para4;
	g_trk_var.g_trk_settings.gps_fance_alarm_setting[fence_number-1][5] = para5;
	g_trk_var.g_trk_settings.gps_fance_alarm_setting[fence_number-1][6] = para6;
	for(i=0;i<20;i++)
	{
	main_printf("old fence: %d, %s",i,&(g_trk_var.g_trk_settings.gps_fance_cordinates[i][0]));
	}	
	for(i=0;i<5;i++)
	{
	main_printf("old setting: %d, %d,%d,%d,%d,%d,%d,%d\n",i,
	g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][0],
	g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][1],
	g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][2],
	g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][3],
	g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][4],
	g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][5],
	g_trk_var.g_trk_settings.gps_fance_alarm_setting[i][6]);
	}
	strcpy(&(g_trk_var.g_trk_settings.gps_fance_cordinates[4*(fence_number-1)][0]),lat1_back);
	strcpy(&(g_trk_var.g_trk_settings.gps_fance_cordinates[4*(fence_number-1)+1][0]),lon1_back);
	strcpy(&(g_trk_var.g_trk_settings.gps_fance_cordinates[4*(fence_number-1)+2][0]),lat2_back);
	strcpy(&(g_trk_var.g_trk_settings.gps_fance_cordinates[4*(fence_number-1)+3][0]),lon2_back);
	main_printf("%d:%x,%x,%x,%x,%x - %x,%x,%x,%x,%x - %x,%x,%x,%x,%x - %x,%x,%x,%x,%x\r\n",fence_number,lat1[0],lat1[1],lat1[2],lat1[3],lat1[4],lon1[0],lon1[1],lon1[2],lon1[3],lon1[4],
	lat2[0],lat2[1],lat2[2],lat2[3],lat2[4],lon2[0],lon2[1],lon2[2],lon2[3],lon2[4]);
	main_printf("GPS_FENCE_ALARM_SETTING_READ:%d,%d,%d,%d,%d,%d,%d\r\n",g_trk_var.g_trk_settings.gps_fance_alarm_setting[fence_number-1][0],
	         g_trk_var.g_trk_settings.gps_fance_alarm_setting[fence_number-1][1],
	         g_trk_var.g_trk_settings.gps_fance_alarm_setting[fence_number-1][2],
	         g_trk_var.g_trk_settings.gps_fance_alarm_setting[fence_number-1][3],
	         g_trk_var.g_trk_settings.gps_fance_alarm_setting[fence_number-1][4],
	         g_trk_var.g_trk_settings.gps_fance_alarm_setting[fence_number-1][5],
	         g_trk_var.g_trk_settings.gps_fance_alarm_setting[fence_number-1][6]);
	fence_para_reset_sig(fence_number-1);
	//fence_speed_operation_reset();
	//check_fence_or_speed_start();
	//tracker_refresh_settings_record(); 
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);	
}

void server_msg_analyze_fence_alarm_onoff(tracker_struct_msg *message)
{
	 U8 fence_number = 0;
	 U8 fence_onoff = 0;
        g_trk_var.g_trk_content_info.command=COMMAND_FENCE_ALARM_ONOFF;
	 if(message->msgLen != MSG_FENCE_ALARM_ONOFF_LEN)
	 {            
            tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	      return;
	 } 
	 fence_number = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
	 fence_onoff =  get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]);	 
	 main_printf("server_msg_analyze_fence_alarm_onoff %d,%d",fence_number,fence_onoff);
	 if(fence_number<1 || fence_number>5 || fence_onoff > 1)
	 {
             tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	 }	
	 g_trk_var.g_trk_settings.gps_fance_alarm_setting[fence_number-1][4] = fence_onoff;
       check_fence_or_speed_start();   
       //tracker_refresh_settings_record();
       tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}

void server_msg_analyze_speed_alarm(tracker_struct_msg *message)
{
	 U8 speed_number=0;
	 U8 para0,para1,para2,para3,para4,para5,para6,para7;
        g_trk_var.g_trk_content_info.command=COMMAND_SPEED_ALARM;
	 if(message->msgLen != MSG_SPEED_ALARM_LEN)
	 {
	 	  main_printf("server_msg_analyze_speed_alarm lenth wrong:%d!",message->msgLen);            
               tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		  return;
	 }	
	 //先取得speed编号
	 speed_number = get_one_byte_uint_from_hex_asc(message->body_msg[14],message->body_msg[15]);
	 if(speed_number<1 || speed_number>5)
	 {
		main_printf("server_msg_analyze_speed_alarm speed_number wrong:%d!",speed_number);
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	 }
	 //Cooldown(mins) (1byte)
	 para0 = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
	 //Sample Rate (mins) (1 byte)//add
	 para1 = get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]);
	 //Alarm number (1byte,1 - 255)
	 para2 = get_one_byte_uint_from_hex_asc(message->body_msg[8],message->body_msg[9]);
	 //Alarm channel
	 para3 = get_one_byte_uint_from_hex_asc(message->body_msg[10],message->body_msg[11]);
	 //0-0ff.1-On(1 byte)
	 para4 = get_one_byte_uint_from_hex_asc(message->body_msg[12],message->body_msg[13]);
	 //alarm number(1byte,1-5)
	 para5 = get_one_byte_uint_from_hex_asc(message->body_msg[14],message->body_msg[15]);
	 //Speed threshold (mph) (1 byte)
	 para6 = get_one_byte_uint_from_hex_asc(message->body_msg[16],message->body_msg[17]);
	 //0 - Under/1 - Over trigger (1 byte)
	 para7 = get_one_byte_uint_from_hex_asc(message->body_msg[18],message->body_msg[19]);
	 //容错!
	 if(para3 != 0xff && para3 != 0x0 && para3 != 0x1 && para3 != 0x2)
	 {
		main_printf("server_msg_analyze_speed_alarm Alarm channel wrong:%d!",para3);
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	 }
	 if(para4 > 1)
	 {
		main_printf("server_msg_analyze_speed_alarm onoff wrong:%d!",para4);
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	 }
	 if(para5<1 || para5>5)
	 {
		main_printf("server_msg_analyze_speed_alarm alarm number wrong:%d!",para5);
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	 }
	 
	 if(para6 == 0)
	 {
		main_printf("server_msg_analyze_speed_alarm Speed threshold wrong:0!");
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	 }
	 
	 if(para7 > 1)
	 {
		main_printf("server_msg_analyze_speed_alarm triger wrong:%d!",para7);
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	 }	 		 	  		 	 				
	//数据OK
	g_trk_var.g_trk_settings.gps_speed_alarm_setting[speed_number-1][0] = para0;
	g_trk_var.g_trk_settings.gps_speed_alarm_setting[speed_number-1][1] = para1;//sample rate 没用
	g_trk_var.g_trk_settings.gps_speed_alarm_setting[speed_number-1][2] = para2;
	g_trk_var.g_trk_settings.gps_speed_alarm_setting[speed_number-1][3] = para3;
	g_trk_var.g_trk_settings.gps_speed_alarm_setting[speed_number-1][4] = para4;
	g_trk_var.g_trk_settings.gps_speed_alarm_setting[speed_number-1][5] = para5;
	g_trk_var.g_trk_settings.gps_speed_alarm_setting[speed_number-1][6] = para6;
	g_trk_var.g_trk_settings.gps_speed_alarm_setting[speed_number-1][7] = para7;
	main_printf("server_msg_analyze_speed_alarm ok:%d,%d,%d,%d,%d,%d,%d,%d\n",para0,para1,para2,para3,para4,para5,para6,para7);
	speed_para_reset();
	fence_speed_operation_reset();
	check_fence_or_speed_start();   	           
	//tracker_refresh_settings_record();
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
   }

//只支持2个，一个是moving，一个是stop moving
void server_msg_analyze_moving_alarm(tracker_struct_msg *message)
{
	U8 para0,para1,para2,para3,para4,para5,para6;
	g_trk_var.g_trk_content_info.command=COMMAND_MOVING_ALARM;
	if(message->msgLen != MSG_MOVING_ALARM_LEN)
	{
		main_printf("server_msg_analyze_moving_alarm length wrong:%d",message->msgLen);
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}
	//Cooldown(mins) (1byte)
	para0 = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
	//Sample Rate (mins) (1 byte)//add
	para1 = get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]);
	//Alarm number (1byte,1 - 255)
	para2 = get_one_byte_uint_from_hex_asc(message->body_msg[8],message->body_msg[9]);
	//Alarm channel
	para3 = get_one_byte_uint_from_hex_asc(message->body_msg[10],message->body_msg[11]);
	//0-0ff.1-On(1 byte)		
	para4 = get_one_byte_uint_from_hex_asc(message->body_msg[12],message->body_msg[13]);
	//Alarm number (1byte, 1-5)
	para5 = get_one_byte_uint_from_hex_asc(message->body_msg[14],message->body_msg[15]);
	//0 - Move/1 - Stop trigger (1 byte)
	para6 = get_one_byte_uint_from_hex_asc(message->body_msg[16],message->body_msg[17]);
	if(para0 == 0)
	{
		main_printf("server_msg_analyze_moving_alarm coolddown wrong:%d",para0);
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}			
	if(para3 !=0xff && para3 !=0x0 && para3 !=0x1 && para3 !=0x2)
	{
		main_printf("server_msg_analyze_moving_alarm Alarm channel wrong:%d",para3);
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}
	if(para4 > 1)
	{
		main_printf("server_msg_analyze_moving_alarm onoff wrong:%d",para4);
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}
	if(para6 > 1)
	{
		main_printf("server_msg_analyze_moving_alarm triger wrong:%d",para5);
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}
	//0 - Move/1 - Stop trigger (1 byte)
	if(para6 == 0)//moving
	{	
		//数据OK
		g_trk_var.g_trk_settings.gps_moving_alarm_setting[0] = para0;
		g_trk_var.g_trk_settings.gps_moving_alarm_setting[1] = para1;
		g_trk_var.g_trk_settings.gps_moving_alarm_setting[2] = para2;
		g_trk_var.g_trk_settings.gps_moving_alarm_setting[3] = para3;
		g_trk_var.g_trk_settings.gps_moving_alarm_setting[4] = para4;
		g_trk_var.g_trk_settings.gps_moving_alarm_setting[5] = para5;
		g_trk_var.g_trk_settings.gps_moving_alarm_setting[6] = para6;
		smart_moving_alarm_start();
		main_printf("server_msg_analyze_moving_alarm ok,moving:%d,%d,%d,%d,%d,%d,%d\n",para0,para1,para2,para3,para4,para5,para6);
	}
	else //stop moving
	{
		//数据OK
		g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[0] = para0;
		g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[1] = para1;
		g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[2] = para2;
		g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[3] = para3;
		g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[4] = para4;
		g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[5] = para5;
		g_trk_var.g_trk_settings.gps_stop_moving_alarm_setting[6] = para6;
		smart_moving_alarm_start();
		main_printf("server_msg_analyze_moving_alarm ok,stop moving:%d,%d,%d,%d,%d,%d,%d",para0,para1,para2,para3,para4,para5,para6);
	} 		 	
	//tracker_refresh_settings_record();
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}

void server_msg_analyze_battery_alarm(tracker_struct_msg *message)	
{
	U8 i=0;
	U8 j=0;
	U8 para0,para1,para2,para3,para4,para5,para6;
	U8 battery_number = 0;
       g_trk_var.g_trk_content_info.command=COMMAND_BATTERY_ALARM;
	if(message->msgLen != MSG_BATTERY_ALARM_LEN)
	{
		main_printf("server_msg_analyze_battery_alarm length wrong:%d",message->msgLen);
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}
	battery_number = get_one_byte_uint_from_hex_asc(message->body_msg[14],message->body_msg[15]);
	if(battery_number<1 || battery_number>5)
	{
		main_printf("server_msg_analyze_battery_alarm battery_number wrong:%d",battery_number);
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;		 	
	}
	//Cooldown(mins) (1byte)
	para0 = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
	//Sample Rate (mins) (1 byte)//add
	para1 = get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]);
	//Alarm number (1byte,1 - 255)
	para2 = get_one_byte_uint_from_hex_asc(message->body_msg[8],message->body_msg[9]);
	//Alarm channel
	para3 = get_one_byte_uint_from_hex_asc(message->body_msg[10],message->body_msg[11]);
	//0-0ff.1-On(1 byte)		
	para4 = get_one_byte_uint_from_hex_asc(message->body_msg[12],message->body_msg[13]);
	//Alarm number (1byte, 1-5)
	para5 = get_one_byte_uint_from_hex_asc(message->body_msg[14],message->body_msg[15]);
	//Battery percent threshold (1 byte,”100,65,35,10,5”)
	para6 = get_one_byte_uint_from_hex_asc(message->body_msg[16],message->body_msg[17]);
	if(para3 !=0xff && para3 !=0x0 && para3 !=0x1 && para3 !=0x2)
	{
		main_printf("server_msg_analyze_battery_alarm Alarm channel wrong:%d",para3);
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}
	
	if(para4 > 1)
	{
		main_printf("server_msg_analyze_battery_alarm onoff wrong:%d",para4);
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}
	
	if(para6 !=100 && para6 !=65 && para6 !=35 && para6 !=10 && para6 !=5)
	{
		main_printf("server_msg_analyze_battery_alarm Battery percent threshold wrong:%d",para6);
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}
	//数据OK
	g_trk_var.g_trk_settings.gps_battery_alarm_setting[battery_number-1][0] = para0;
	g_trk_var.g_trk_settings.gps_battery_alarm_setting[battery_number-1][1] = para1;
	g_trk_var.g_trk_settings.gps_battery_alarm_setting[battery_number-1][2] = para2;
	g_trk_var.g_trk_settings.gps_battery_alarm_setting[battery_number-1][3] = para3;
	g_trk_var.g_trk_settings.gps_battery_alarm_setting[battery_number-1][4] = para4;
	g_trk_var.g_trk_settings.gps_battery_alarm_setting[battery_number-1][5] = para5;
	g_trk_var.g_trk_settings.gps_battery_alarm_setting[battery_number-1][6] = para6;
	reset_bat_alarm_sig(battery_number-1);//置false
	main_printf("server_msg_analyze_battery_alarm ok:%d,%d,%d,%d,%d,%d,%d\n",para0,para1,para2,para3,para4,para5,para6);
       //tracker_refresh_settings_record();
       tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}	

void server_msg_analyze_set_apn(tracker_struct_msg *message) 
{
	 U8 count = 0;
        char set_apn[200];
        BOOL ret = TRUE;
        U8 comma_total = 0;
        g_trk_var.g_trk_content_info.command=COMMAND_SET_APN;
	 if(0 == get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]))
	 {
	    	 main_printf("server_msg_analyze_set_apn fail 1"); 
	 	 ret = FALSE;
	 }
	 else
	 {		
		memset(set_apn,0,200); 	
		while(0!=get_one_byte_uint_from_hex_asc(message->body_msg[4+count*2],message->body_msg[4+count*2+1]) && count < 180)
		{
		set_apn[count] = get_one_byte_uint_from_hex_asc(message->body_msg[4+count*2],message->body_msg[4+count*2+1]);
		if(set_apn[count] == ',')
		{
			comma_total++;
		}	
			count++;
		}
		if(comma_total < 3)
		{
			ret = FALSE;
		}
	  }
	  if(ret == FALSE)
	  {
        	tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	  }
	  else
	  {
           	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
	    	main_printf("server_msg_analyze_set_apn %s",set_apn);
		#if 0//def __ADD_APN_BY_SMS__
		tracker_cmd_process_handle(set_apn);
		printf("xy_add_apn_test apn_str=%s\r\n",apn_str);
		printf("xy_add_apn_test account_str=%s\r\n",account_str);
		printf("xy_add_apn_test homepage_str=%s\r\n",homepage_str);
		printf("xy_add_apn_test px_port=%d\r\n",px_port);
		printf("xy_add_apn_test px_addr=%s\r\n",px_addr);
		printf("xy_add_apn_test px_name=%s\r\n",px_name);
		printf("xy_add_apn_test px_password=%s\r\n",px_password);
		tracker_add_dtcnt();		
		#endif
	  }    
}	

void server_msg_analyze_set_active_carrier(tracker_struct_msg *message)
{
	 char carrier_new[100];
	 U8 len = 0;
	 U8 i=0;
	 memset(carrier_new,0,100);
	 len = message->msgLen;
	 if(len == 4)//空参数，不改
	 {
		g_trk_var.g_trk_content_info.command=COMMAND_SET_ACTIVE_CARRIER;
		tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
		return;	
 	 }
 	 //把channel记下来
 	 rpy_set_active_carrier_channel = get_one_byte_uint_from_hex_asc(message->body_msg[2],message->body_msg[3]);
 	 len = len - 4;//去掉前2个byte 
 	 //把后面的carrier字符串读出来
 	 for(i=0;i<(len/2);i++)
 	 {
 	 	 carrier_new[i] = get_one_byte_uint_from_hex_asc(message->body_msg[2*i+4],message->body_msg[2*i+5]);
 	 }
 	 main_printf("set_active_carrier:%s,channel:%d",carrier_new,rpy_set_active_carrier_channel);	
 	// set_active_carrier(carrier_new);
}

void server_msg_analyze_set_session_max_bytes(tracker_struct_msg *message)
{
	g_trk_var.g_trk_content_info.command=COMMAND_SESSION_MAX_BYTES;
	if(message->msgLen != MSG_SET_SESSION_MAX_BYTES_LEN)
	{
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}
	g_trk_var.g_user_settings.current_session_max_bytes= get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
	main_printf("server_msg_analyze_set_session_max_bytes %d\n",g_trk_var.g_user_settings.current_session_max_bytes);
	//tracker_refresh_comm_user_settings_record();
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
} 

void server_msg_analyze_ota_start(tracker_struct_msg *message)
{
        g_trk_var.g_trk_content_info.command=COMMAND_OTA_START;
        tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
} 	 		

/***************************************************
	8 bits - Identification.
  8 bits - Ack Channel.
	1 bit - Send Ack for "Status Request"
	1 bit - Send Ack for "Get History"
	1 bit - Send Ack for "Get Imsis"
	1 bit - Send Ack for "Get Carriers"
	1 bit - Send Ack for "Get Single Measurement"
	1 bit - Send Ack for "Status 2 Request"
	1 bit - Send Ack for "Confirm CallerID"
  1 bit - Send Ack for "Get Bluetooth MAC Report"   		// Unimplemented yet

	This is a total of 24 bits that will be accommodated in 3 bytes. 
	We will expect an ACK message as a response.

***************************************************/
void server_msg_analyze_ack_setting(tracker_struct_msg *message)
{
	//为将来扩展，不check长度
	g_trk_var.g_trk_content_info.command=COMMAND_ACK_SETTING;
	g_trk_var.g_trk_settings.gps_ack_setting[0] = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
	//tracker_refresh_settings_record();	 
	main_printf("server_msg_analyze_ack_setting %x\n",g_trk_var.g_trk_settings.gps_ack_setting[0]);
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}

void server_msg_analyze_start_tracking_mode_with_measurement(tracker_struct_msg *message)
{
	g_trk_var.g_trk_content_info.command=COMMAND_START_TRACKING_MODE_WITH_MEASUREMENT; //tracking channe
	g_trk_var.g_trk_settings.track_channel = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
	g_trk_var.g_trk_settings.track_channel = g_trk_var.g_user_settings.data_transfer_channel;
	//sample time
	g_trk_var.g_trk_settings.sample_time= get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]);
	//report time
	g_trk_var.g_trk_settings.report_time = get_one_byte_uint_from_hex_asc(message->body_msg[8],message->body_msg[9]);
	//Location Accuracy
	g_trk_var.g_trk_settings.location_accuracy = get_one_byte_uint_from_hex_asc(message->body_msg[10],message->body_msg[11]);
	//setting
	g_trk_var.g_trk_settings.track_setting= get_one_byte_uint_from_hex_asc(message->body_msg[12],message->body_msg[13]);
	main_printf("server_msg_analyze_start_tracking_mode_with_measurement channel=%x,sample=%x,report=%x,setting=%x\r\n",
	                                      g_trk_var.g_trk_settings.track_channel,
	                                      g_trk_var.g_trk_settings.sample_time,
	                                      g_trk_var.g_trk_settings.report_time, 
	                                      g_trk_var.g_trk_settings.track_setting);
	tracker_refresh_settings_record(NVDM_SETTING_SAMPLE_TIME);
	tracker_refresh_settings_record(NVDM_SETTING_REPORT_TIME);
	if(g_trk_var.g_trk_settings.sample_time> 0)
	{	
		stop_old_tracking_because_of_new_tracking();
	}   
	//tracking_with_measurement_start();
	tracking_with_measurement_start_new();
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
} 

/************************************************
	8 bits   -  Identification.
       8 bits   -  Ack Channel (0="USSD",1="SMS",2="Data")
	16 bits -  StartDiffMinutes   (Important only once, and in Minutes e.g. 37 minutes)
	16 bits -  SleepPeriodMinutes  (e.g. 25 Min)
	16 bits -  AwakePeriodMinutes (e.g. 7 Minutes awake)
       8 Bits   -  SendGPSAndSleep (0=off,1=on)
	8 bits   -  Repeat (0=off,1=on)
	We will expect an ACK message as a response.  

***************************************************/	
void server_msg_analyze_schedule_sleep(tracker_struct_msg *message)
{
         g_trk_var.g_trk_content_info.command=COMMAND_SCHEDULE_SLEEP;
	  //单独添加，发这个指令就是on
	  g_trk_var.g_trk_settings.gps_schedule_sleep[0] = 1;
	  //16 bits -  StartDiffMinutes   (Important only once, and in Minutes e.g. 37 minutes)
	  g_trk_var.g_trk_settings.gps_schedule_sleep[1] = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
	  g_trk_var.g_trk_settings.gps_schedule_sleep[2] = get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]);
	  //16 bits -  SleepPeriodMinutes  (e.g. 25 Min)
	  g_trk_var.g_trk_settings.gps_schedule_sleep[3] = get_one_byte_uint_from_hex_asc(message->body_msg[8],message->body_msg[9]);
	  g_trk_var.g_trk_settings.gps_schedule_sleep[4] = get_one_byte_uint_from_hex_asc(message->body_msg[10],message->body_msg[11]);
	  //16 bits -  AwakePeriodMinutes (e.g. 7 Minutes awake)
	  g_trk_var.g_trk_settings.gps_schedule_sleep[5] = get_one_byte_uint_from_hex_asc(message->body_msg[12],message->body_msg[13]);
	  g_trk_var.g_trk_settings.gps_schedule_sleep[6] = get_one_byte_uint_from_hex_asc(message->body_msg[14],message->body_msg[15]);
	  //8 Bits   -  SendGPSAndSleep (0=off,1=on)
	  g_trk_var.g_trk_settings.gps_schedule_sleep[7] = get_one_byte_uint_from_hex_asc(message->body_msg[16],message->body_msg[17]);
	  //8 bits   -  Repeat (0=off,1=on)
	  g_trk_var.g_trk_settings.gps_schedule_sleep[8] = get_one_byte_uint_from_hex_asc(message->body_msg[18],message->body_msg[19]);
         //tracker_refresh_settings_record();	  
	  schedule_sleep_start();
         tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}	

void server_msg_analyze_schedule_sleep_off(tracker_struct_msg *message)
{
	g_trk_var.g_trk_content_info.command=COMMAND_SCHEDULE_SLEEP_OFF;
	//单独添加，发这个指令就是off
	g_trk_var.g_trk_settings.gps_schedule_sleep[0] = 0;
	main_printf("server_msg_analyze_schedule_sleep_off %x,%x,%x,%x,%x,%x,%x,%x,%x\n",
	g_trk_var.g_trk_settings.gps_schedule_sleep[0],g_trk_var.g_trk_settings.gps_schedule_sleep[1],g_trk_var.g_trk_settings.gps_schedule_sleep[2],
	g_trk_var.g_trk_settings.gps_schedule_sleep[3],g_trk_var.g_trk_settings.gps_schedule_sleep[4],g_trk_var.g_trk_settings.gps_schedule_sleep[5],
	g_trk_var.g_trk_settings.gps_schedule_sleep[6],g_trk_var.g_trk_settings.gps_schedule_sleep[7],g_trk_var.g_trk_settings.gps_schedule_sleep[8]);
	//tracker_refresh_settings_record();      
	schedule_sleep_stop();
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}

/******************************************************************
 A new “General Setup with Net Identifiers” will be sent as a response to the “Handshake request”, or without any relation to it. The message will look like this:
8 bits - Identification.
8 bits - Ack Channel (0="USSD",1="SMS",2="Data")
8 bits - Channel(0="USSD",1="SMS",2="Data")
32 bits - UDP Ip (sent as Ip)
16 bits - UDP Port
32 bits - Device ID
80 bits - Phone Number
8 bits - Response Delay Time
8 bits - Calls To Device (0=Answer,1=Ignore,2=Send CallerID and Answer,3=Send    CallerID and Wait for Confirmation)
8bits - Gps On When Charging (0=off,1=on)
8 bits - GSM Keep Alive
8 bits - Data Keep Alive
8 bits - Beep(0=off,1=on)
8 bits - Send Session Start(0=off,1=on)
8 bits - Reboot Timer Active
8 bits - moving interrupt count for gps
8 bits - heart pulse msg in hour
8 bits - turn off_on in min
8 bits - number of failure ussd to reboot
8 bits - USSD Q Timer
8 bits - MOTION ALARM PARA 1
8 bits - MOTION ALARM PARA 2
8 bits - MOTION ALARM PARA 3
8 bits - button press ack timeout
String - APN
String - APN User Name
String - APN Password
String - USSD Prefix
8 bits - BT on/off(0=off,1=on)
8 bits - Network preference (0=2G,1=3G,2=LTE).
16 bits - Config Version
8 bits - Smart power saving on/off(0=off,1=on)
8 bits - Shutdown message on/off(0=off,1=on)
**********************************************************/
void server_msg_analyze_general_setup_with_net_identifiers(tracker_struct_msg *message)
{
        U8 ipadress[4]={0};
        U8 low_port=0;
        U8 high_port=0;  
        U16 ip_port=0;
	 U8 wait_dur = 0;
	 U8 device_id0,device_id1,device_id2,device_id3;
	 U8 phone_number0,phone_number1,phone_number2,phone_number3,phone_number4,phone_number5,phone_number6,phone_number7,phone_number8,phone_number9;
	 U8 temp = 0;
	 U8 count = 0;
	 //0 - USSD/ 1 - SMS/ 2 – Data (1 byte)
	 g_trk_var.g_user_settings.data_transfer_channel = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);	 
	 main_printf("1:%d\n",g_trk_var.g_user_settings.data_transfer_channel); 
	 //UDP control IP (4 bytes)(a.b.c.d)
   	//UDP control port (2 bytes)(max = 65536)
        ipadress[0] = get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]); 	
	 ipadress[1] = get_one_byte_uint_from_hex_asc(message->body_msg[8],message->body_msg[9]);
	 ipadress[2] = get_one_byte_uint_from_hex_asc(message->body_msg[10],message->body_msg[11]); 
	 ipadress[3] = get_one_byte_uint_from_hex_asc(message->body_msg[12],message->body_msg[13]);
	 high_port = get_one_byte_uint_from_hex_asc(message->body_msg[14],message->body_msg[15]);
	 low_port = get_one_byte_uint_from_hex_asc(message->body_msg[16],message->body_msg[17]);
        ip_port=high_port*256+low_port;
     	 if(ipadress[0]==0&&ipadress[1]==0&&ipadress[2]==0&&ipadress[3]==0) 
		{
		  ;//dont update
		}
		 else
		{   
			if(g_trk_var.g_user_settings.platform_url.host.ip[0]== ipadress[0] &&
			g_trk_var.g_user_settings.platform_url.host.ip[1] == ipadress[1] &&
			g_trk_var.g_user_settings.platform_url.host.ip[2] == ipadress[2] &&
			g_trk_var.g_user_settings.platform_url.host.ip[3] == ipadress[3] &&
			g_trk_var.g_user_settings.platform_url.port==ip_port)
				{
				;//dont update
				}
			else
				{            
					g_trk_var.g_user_settings.platform_url.host.ip[0]=ipadress[0];
					g_trk_var.g_user_settings.platform_url.host.ip[1]=ipadress[1];
					g_trk_var.g_user_settings.platform_url.host.ip[2]=ipadress[2];
					g_trk_var.g_user_settings.platform_url.host.ip[3]=ipadress[3];
					g_trk_var.g_user_settings.platform_url.port=ip_port;
					//data_session_off();//更新了ip/port，要重连
				}  
			main_printf("ip:%d,%d,%d,%d:%d\r\n",g_trk_var.g_user_settings.platform_url.host.ip[0],
			g_trk_var.g_user_settings.platform_url.host.ip[1],
			g_trk_var.g_user_settings.platform_url.host.ip[2],
			g_trk_var.g_user_settings.platform_url.host.ip[3],
			g_trk_var.g_user_settings.platform_url.port);
		}   
	 //Device ID (4 bytes)
	 device_id0 = get_one_byte_uint_from_hex_asc(message->body_msg[18],message->body_msg[19]);
	 device_id1 = get_one_byte_uint_from_hex_asc(message->body_msg[20],message->body_msg[21]);
	 device_id2 = get_one_byte_uint_from_hex_asc(message->body_msg[22],message->body_msg[23]);
	 device_id3 = get_one_byte_uint_from_hex_asc(message->body_msg[24],message->body_msg[25]);
	 main_printf("3:%d,%d,%d,%d\r\n",device_id0,device_id1,device_id2,device_id3); 
	 if(device_id0 == 0 && device_id1 == 0 && device_id2 == 0 && device_id3 == 0)
	 {
	 	  ;//do nothing
	 }
	 else
	 {	
		#ifdef __TRACKER_LED_SUPPORT__	 
		if( g_trk_var.g_user_settings.deviceid[0] == 0 &&
		g_trk_var.g_user_settings.deviceid[1] == 0 &&
		g_trk_var.g_user_settings.deviceid[2] == 0 &&
		g_trk_var.g_user_settings.deviceid[3] == 0)
		{
		//write_device_id_from_empty_led();
		}	
		#endif	 	  
	    	g_trk_var.g_user_settings.deviceid[0]= device_id0;
	    	g_trk_var.g_user_settings.deviceid[1]= device_id1;
	    	g_trk_var.g_user_settings.deviceid[2]= device_id2;
	    	g_trk_var.g_user_settings.deviceid[3]= device_id3;
		#ifdef __TRACKIMO_BLE__	    
	     	//Tracker_BLE_broadcast_name_change();
		#endif
	 }   	 
	 //Phone number for Voice (20 digits - BCD)(10 bytes)
	 phone_number0 = get_one_byte_uint_from_hex_asc(message->body_msg[26],message->body_msg[27]);
	 phone_number1 = get_one_byte_uint_from_hex_asc(message->body_msg[28],message->body_msg[29]);
	 phone_number2 = get_one_byte_uint_from_hex_asc(message->body_msg[30],message->body_msg[31]);
	 phone_number3 = get_one_byte_uint_from_hex_asc(message->body_msg[32],message->body_msg[33]);
	 phone_number4 = get_one_byte_uint_from_hex_asc(message->body_msg[34],message->body_msg[35]);
	 phone_number5 = get_one_byte_uint_from_hex_asc(message->body_msg[36],message->body_msg[37]);
	 phone_number6 = get_one_byte_uint_from_hex_asc(message->body_msg[38],message->body_msg[39]);
	 phone_number7 = get_one_byte_uint_from_hex_asc(message->body_msg[40],message->body_msg[41]);
	 phone_number8 = get_one_byte_uint_from_hex_asc(message->body_msg[42],message->body_msg[43]);
	 phone_number9 = get_one_byte_uint_from_hex_asc(message->body_msg[44],message->body_msg[45]);
	 main_printf("4:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",phone_number0,phone_number1,phone_number2,phone_number3,phone_number4,phone_number5,phone_number6,phone_number7,phone_number8,phone_number9); 
	 if(phone_number0==0 && phone_number1==0 && phone_number2==0 && phone_number3==0 && phone_number4==0 && phone_number5==0 && phone_number6==0 && phone_number7==0 && phone_number8==0 && phone_number9==0)
	 {	
	 	 ;//do nothing
	 }
	 else
	 {		      
             g_trk_var.g_user_settings.tracker_phone_number[0] = phone_number0;
             g_trk_var.g_user_settings.tracker_phone_number[1] = phone_number1;
             g_trk_var.g_user_settings.tracker_phone_number[2] = phone_number2;
             g_trk_var.g_user_settings.tracker_phone_number[3]= phone_number3;
             g_trk_var.g_user_settings.tracker_phone_number[4] = phone_number4;
             g_trk_var.g_user_settings.tracker_phone_number[5]= phone_number5;
             g_trk_var.g_user_settings.tracker_phone_number[6]= phone_number6;
             g_trk_var.g_user_settings.tracker_phone_number[7] = phone_number7;
             g_trk_var.g_user_settings.tracker_phone_number[8] = phone_number8;
             g_trk_var.g_user_settings.tracker_phone_number[9] = phone_number9;
      }	 	 
	 //注意：GPS_GENERAL_SETUP_READ[15]去用来做来电是否接听
	 wait_dur = get_one_byte_uint_from_hex_asc(message->body_msg[46],message->body_msg[47]);
	 main_printf("5:%d\r\n",wait_dur);
	 if(wait_dur > 0)
	 {	
	    g_trk_var.g_user_settings.ussd_lock_timer = wait_dur;
	 } 
	 //防止服务器犯错，GPS_MSG_WAIT_DUR_READ不能为0
	 if(g_trk_var.g_user_settings.ussd_lock_timer== 0)
	 	g_trk_var.g_user_settings.ussd_lock_timer = 10;
	 		 	
	 //call answer or not,0:answer,1:not answer,2:answer and send caller id,3:not implement
	 temp = get_one_byte_uint_from_hex_asc(message->body_msg[48],message->body_msg[49]); 
	 
	 //tracker_prompt_trace("6:%d\r\n",temp);
	 if(temp >=0 && temp<= 3)
	 {
	 	 g_trk_var.g_user_settings.answer_mode	= temp;
	 } 	 
	 g_trk_var.g_user_settings.charge_powon_gps = get_one_byte_uint_from_hex_asc(message->body_msg[50],message->body_msg[51]);
	 main_printf("7:%d\r\n",temp);
	 
	 //gsm keep alive, in hour
	 temp = get_one_byte_uint_from_hex_asc(message->body_msg[52],message->body_msg[53]); 
	 main_printf("8:%d\r\n",temp);
	 g_trk_var.g_user_settings.ussd_heart_beat_in_hour = temp;
	 
	 //data keep alive, in min
	 temp = get_one_byte_uint_from_hex_asc(message->body_msg[54],message->body_msg[55]); 
	 main_printf("9:%d\r\n",temp);
	   g_trk_var.g_user_settings.data_heart_beat_in_min= temp;
	 	
	 //beep enable
	 temp = get_one_byte_uint_from_hex_asc(message->body_msg[56],message->body_msg[57]); 
	 main_printf("10:%d\r\n",temp);
	 if(temp == 0 || temp == 1)
	 	g_trk_var.g_user_settings.beeper_enable= temp;	
	 	
	 //session start
	g_trk_var.g_user_settings.session_enable= get_one_byte_uint_from_hex_asc(message->body_msg[58],message->body_msg[59]); 
 	 main_printf("11:%d\r\n",g_trk_var.g_user_settings.session_enable);
 	 //reboot timer
 	g_trk_var.g_user_settings.reboot_timer = get_one_byte_uint_from_hex_asc(message->body_msg[60],message->body_msg[61]);
 	 main_printf("12:%d\r\n",g_trk_var.g_user_settings.reboot_timer);
     
         auto_reboot_timer_in_hour=0;//重置reboot
 	 
 	 //10s内多少个中断认为是震动，可以重新开始tracking, alarm...
 	  g_trk_var.g_user_settings.tracking_alarm_in_10s= get_one_byte_uint_from_hex_asc(message->body_msg[62],message->body_msg[63]);
 	 main_printf("13:%d\r\n", g_trk_var.g_user_settings.tracking_alarm_in_10s);
 	 //防止服务器犯错
 	 if( g_trk_var.g_user_settings.tracking_alarm_in_10s> 20)
 	 	 g_trk_var.g_user_settings.tracking_alarm_in_10s = 20;
 	 	
 	 //heart pulse, 多长时间发一次心跳包，单位小时
 	 g_trk_var.g_user_settings.tracking_heart_pulse = get_one_byte_uint_from_hex_asc(message->body_msg[64],message->body_msg[65]);
 	 main_printf("14:%d\r\n",g_trk_var.g_user_settings.tracking_heart_pulse);
 	 heart_pulse_start();	
 	 
 	 //如果心跳包没回复，关机几分钟开机，单位分钟
 	 g_trk_var.g_user_settings.no_heart_pulse_reboot_in_min = get_one_byte_uint_from_hex_asc(message->body_msg[66],message->body_msg[67]);
 	 main_printf("15:%d\r\n",g_trk_var.g_user_settings.no_heart_pulse_reboot_in_min);
 	 //如果连续n个ussd send fail，reboot
 	 g_trk_var.g_user_settings.send_ussd_fail_num = get_one_byte_uint_from_hex_asc(message->body_msg[68],message->body_msg[69]);
 	 main_printf("16:%d\r\n",g_trk_var.g_user_settings.send_ussd_fail_num);
 	 //ussd q timer
 	 g_trk_var.g_user_settings.ussd_q_timer = get_one_byte_uint_from_hex_asc(message->body_msg[70],message->body_msg[71]);
 	 main_printf("17:%d\r\n",g_trk_var.g_user_settings.ussd_q_timer);
 	 //motion para 1
 	 g_trk_var.g_user_settings.motion_para_1 = get_one_byte_uint_from_hex_asc(message->body_msg[72],message->body_msg[73]);
 	 main_printf("18:%d\r\n",g_trk_var.g_user_settings.motion_para_1);
 	 //motion para 2
 	  g_trk_var.g_user_settings.motion_para_2= get_one_byte_uint_from_hex_asc(message->body_msg[74],message->body_msg[75]);
 	 main_printf("19:%d\r\n", g_trk_var.g_user_settings.motion_para_2);
 	 //motion para 3, 16位filter，高8位
 	   g_trk_var.g_user_settings.motion_para_3 = get_one_byte_uint_from_hex_asc(message->body_msg[76],message->body_msg[77]);
 	 main_printf("20:%d\r\n",  g_trk_var.g_user_settings.motion_para_3);
 	 //Button Press ACK Timeout (seconds)(1 byte)
	 g_trk_var.g_user_settings.press_ack_time_out_in_sec= get_one_byte_uint_from_hex_asc(message->body_msg[78],message->body_msg[79]);
	 main_printf("21:%d\r\n",g_trk_var.g_user_settings.press_ack_time_out_in_sec);
	 //添加
	 //String - APN
	 count = 0;
	 temp = count;
	 if(0 == get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]))
	 {
	 
               memset(g_trk_var.g_trk_settings.gps_apn_string,0,50);
	 	 count++;
	 }
	 else
	 {		
            memset(g_trk_var.g_trk_settings.gps_apn_string,0,50);
	    while(0!=get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]))
	    {
	    	g_trk_var.g_trk_settings.gps_apn_string[count-temp] = get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]);
	    	count++;
	    }
	    count++;//string结尾是0
	 }
	 main_printf("GPS_APN_STRING_READ:%s\r\n",g_trk_var.g_trk_settings.gps_apn_string);  
	 temp = count;
	 //apn user	
	 if(0 == get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]))
	 {
	 	 memset(g_trk_var.g_trk_settings.gps_apn_user,0,50);
	 	 count++;
	 }
	 else
	 {		
	 	memset(g_trk_var.g_trk_settings.gps_apn_user,0,50);
	    while(0!=get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]))
	    {
	    	g_trk_var.g_trk_settings.gps_apn_user[count-temp] = get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]);
	    	count++;
	    }
	    count++;//string结尾是0 
	 }
	 main_printf("GPS_APN_USER_READ:%s\r\n",g_trk_var.g_trk_settings.gps_apn_user);
	 //apn pw	
	 temp = count;
	 if(0 == get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]))
	 {
		memset(g_trk_var.g_trk_settings.gps_apn_pw,0,50);
		count++;
	 }
	 else
	 {		
		memset(g_trk_var.g_trk_settings.gps_apn_pw,0,50);
		while(0!=get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]))
		{
		g_trk_var.g_trk_settings.gps_apn_pw[count-temp] = get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]);
		count++;
		}
		count++;//string结尾是0 
	 }
	 main_printf("GPS_APN_PW_READ:%s\r\n",g_trk_var.g_trk_settings.gps_apn_pw);
	 //ussd prefix	 暂时不用
	 temp = count;
	 if(0 == get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]))
	 {
	 	 memset(g_trk_var.g_trk_settings.gps_ussd_prefix_read,0,50);
	 	 count++;
	 }
	 else
	 {		
	 	 memset(g_trk_var.g_trk_settings.gps_ussd_prefix_read,0,50);
	    while(0!=get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]))
	    {
	    	g_trk_var.g_trk_settings.gps_ussd_prefix_read[count-temp] = get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]);
	    	count++;
	    }
	    count++;//string结尾是0 
	 }
	 main_printf("g_trk_var.g_trk_settings.gps_ussd_prefix_read:%s\r\n",g_trk_var.g_trk_settings.gps_ussd_prefix_read); 	 	 
	 //8 bits - BT on/off(0=off,1=on)	 
	 g_trk_var.g_user_settings.bt_enable = get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]);count++;
	 main_printf("bt_enable:%d\r\n",g_trk_var.g_user_settings.bt_enable);
	 //去掉它，bt不能关 bt_start_from_server();
	 //8 bits - Network preference (0=2G,1=3G,2=LTE).
         g_trk_var.g_user_settings.network_preference= get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]);count++;       	 
	 main_printf("g_trk_var.g_user_settings.network_preference:%d\r\n",g_trk_var.g_user_settings.network_preference);
	 //16 bits - Config Version 
	 g_trk_var.g_user_settings.agps_setting[0] = get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]);count++;
	 g_trk_var.g_user_settings.agps_setting[1] = get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]);count++;
	 main_printf("agps_setting:%d,%d\r\n",g_trk_var.g_user_settings.agps_setting[0],g_trk_var.g_user_settings.agps_setting[1]);
	 //8 bits - Smart power saving on/off(0=off,1=on)
	 g_trk_var.g_user_settings.smart_save_power_enable= get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]);count++;
	 check_smart_power_saving_setting();
	 main_printf("save_power_enable:%d\r\n",g_trk_var.g_user_settings.smart_save_power_enable);	
	 //关了要把sensor打开
	 if(g_trk_var.g_user_settings.smart_save_power_enable == 0)
	 {
	 	#ifdef SUPPORT_MY_GSENSOR
	 	gSensor_on();
	 //	gsensor_eint_enable();
	 	#endif
	 }
	 //8 bits - Shutdown message on/off(0=off,1=on)
	 g_trk_var.g_user_settings.power_off_message_enable= get_one_byte_uint_from_hex_asc(message->body_msg[80+count*2],message->body_msg[80+count*2+1]);count++;
	 main_printf("power_off_message_enable:%d\r\n",g_trk_var.g_user_settings.power_off_message_enable);
        //tracker_refresh_comm_user_settings_record();	 
        g_trk_var.g_trk_content_info.command=COMMAND_GENERAL_SETUP_WITH_NET_IDENTIFIERS;
        tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}


/*********************************************************************
Control
----------
AGpsBootstrapLocation
Message id (int , 1 byte)
Message Ack Channel (int, 1 byte)
Message seq num (int, 2 bytes)
Latitude (location, 5 bytes)
Longitude (location, 5 bytes)
Current UTC time (String)

b5ff000000225290510114040500323031363038303331323034333600

**********************************************************************/
//此处只回复ack
void server_msg_analyze_agps_bootstrap_location(tracker_struct_msg *message)
{
        g_trk_var.g_trk_content_info.command=COMMAND_AGPS_BOOTSTRAP_LOCATION;   
        g_trk_var.g_trk_content_info.sequence_1=get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
        g_trk_var.g_trk_content_info.sequence_2=get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]);
        tracker_protocol_send_deal(REPLY_ACK_SEQUENTIAL,g_trk_var.g_user_settings.data_transfer_channel);
}    

/***************************************************************
In return, the server will send a "Set components" message (in case of discrepancy, of course) which looks like this:

b6ff0000007500007601007703

Message id is 54 (since this is a control message we add 128, resulting in 182 - b6 in hex)
"ff" is the ack channel (in this case - the default channel)
"0000" is the sequential number 
the rest of the message was encoded like described in the previous message.
****************************************************************/	
void server_msg_analyze_set_components_dynamic(tracker_struct_msg *message)
{   
	U8 i=0;
	U8 len = message->msgLen;
	U8 dynamic_item;
	U8 id1,id2,dynamic_value;
       g_trk_var.g_trk_content_info.command=COMMAND_SET_COMPONENTS_DYNAMIC;
	if(len<=8)
	{
		main_printf("server_msg_analyze_set_components_dynamic len=%d wrong",len);	
              tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}	
       g_trk_var.g_trk_content_info.sequence_1=get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
       g_trk_var.g_trk_content_info.sequence_2=get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]);
	dynamic_item = (len-8)/2/3;
	main_printf("server_msg_analyze_set_components_dynamic dynamic_item=%d",dynamic_item);
	for(i=0;i<dynamic_item;i++)
	{
		id1 = get_one_byte_uint_from_hex_asc(message->body_msg[8+i*6],message->body_msg[9+i*6]);	
		id2 = get_one_byte_uint_from_hex_asc(message->body_msg[10+i*6],message->body_msg[11+i*6]);
		dynamic_value = get_one_byte_uint_from_hex_asc(message->body_msg[12+i*6],message->body_msg[13+i*6]);
	        tracker_dynamic_analyze(id1,id2,dynamic_value);
	}
	//tracker_refresh_comm_user_settings_record();
      tracker_protocol_send_deal(REPLY_ACK_SEQUENTIAL,g_trk_var.g_user_settings.data_transfer_channel);
}	

void server_msg_analyze_reset_session(tracker_struct_msg *message)
{
	 main_printf("server_msg_analyze_reset_session");
	// data_session_off();
     	g_trk_var.g_trk_content_info.command=COMMAND_RESET_SESSION;     
    	 tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
} 

void server_msg_analyze_ussd_q_timer(tracker_struct_msg *message)
{
	U8 ussd_q_timer = 0;
    	g_trk_var.g_trk_content_info.command=COMMAND_MY_COMMAND_USSD_Q_TIMER;
  	 if(message->msgLen != MSG_USSD_Q_TIMER_LEN)
	    {            
	          tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
	          return;
	     }
     // GPS_UMBRAL_READ[0]= get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);   //这个参数没实际应用
     ussd_q_timer=get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]); 
     main_printf("server_msg_analyze_ussd_q_timer %d",ussd_q_timer);
     tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}

void server_msg_analyze_motion_count(tracker_struct_msg *message)
{	 
	g_trk_var.g_trk_content_info.command=COMMAND_MY_COMMAND_MOTION_COUNT;
	if(message->msgLen != MSG_MOTION_COUNT_LEN)
	{            
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}
	//count
	g_trk_var.g_trk_settings.gps_data_timer[0]= get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
	//filter
	g_trk_var.g_trk_settings.gps_data_timer[1]= get_one_byte_uint_from_hex_asc(message->body_msg[6],message->body_msg[7]);
	//tracker_refresh_settings_record();	 
	main_printf("server_msg_analyze_motion_count %d,filter:%d\r\n",g_trk_var.g_trk_settings.gps_data_timer[0],g_trk_var.g_trk_settings.gps_data_timer[1]);
	tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}

void server_msg_analyze_auto_change_carrier(tracker_struct_msg *message)
{
	U8 on_off = 0;
       g_trk_var.g_trk_content_info.command=COMMAND_AUTO_CHANGE_CARRIER;
       if(message->msgLen != MSG_AUTO_CHANGE_CARRIER_LEN)
	{
		tracker_protocol_send_deal(REPLY_HEADER_ERROR,g_trk_var.g_user_settings.data_transfer_channel);
		return;
	}
	g_trk_var.g_user_settings.auto_change_carrier = get_one_byte_uint_from_hex_asc(message->body_msg[4],message->body_msg[5]);
	main_printf("server_msg_analyze_auto_change_carrier %d",g_trk_var.g_user_settings.auto_change_carrier );
	//tracker_refresh_comm_user_settings_record();
       tracker_protocol_send_deal(REPLY_HEADER_SUCCESS,g_trk_var.g_user_settings.data_transfer_channel);
}


