#include "MyBaseDefine.h"
#include "TrackerCfg.h"
#include "MyDefine.h"
#include "MyGPS.h"
#include "MyWifi.h"

#define DEVICEID_HEADER_4G 999000000
#define DEVICEID_4G_NUMBER_0 '9'
#define DEVICEID_4G_NUMBER_1 '9'
#define DEVICEID_4G_NUMBER_2 '9'
#define TRACKIMO_WORKSTATUS_REPORT_TIME   30000  // 10��
#define TRACKIMO_GPS_WORK_TIMEOUT	120000 //30��
#define TRACKER_SENDWFIF_START 1
#define TRACKER_WORK_START_TIME 2

typedef enum
{
    REPLY_CHANNEL_USSD = 0,    //USSD
    REPLY_CHANNEL_SMS = 1,     //SMS
    REPLY_CHANNEL_DATA_UDP = 2,     //DATA
    REPLY_CHANNEL_DATA_MODULE=3, // 4g cat-m1 or nB-iot 
    REPLY_CHANNEL_BOTH_UDP_AND_MODULE=4, // gsm ������ ͨѶһ������
    REPLY_CHANNEL_MAX
} reply_channel;

//call answer or not,0:answer,1:not answer,2:answer and send caller id,3:not implement
typedef enum
{
    ANSWER_CALL= 0,   
    NO_ANSWER_CALL = 1,     
    ANSER_AND_SEND_CALLER_ID = 2,     
    NOT_IMPLEMENT=3, 
} caller_answer_mode_type;

/*
* Mode enum
*/
typedef enum
{
	TRACKER_MODE_SWITCH_ALL_OFF= 0x00, 					 /* All modem are turn off */
	TRACKER_MODE_SWITCH_2G= 0x01,						/* 2g modem are turn on */
	TRACKER_MODE_SWITCH_4G = 0x02, 						/* 4g modem are turn on  */
	TRACKER_MODE_SWITCH_SIM_ALL=TRACKER_MODE_SWITCH_2G|TRACKER_MODE_SWITCH_4G,
	TRACKER_MODE_MODE_NUM
} tracker_work_mode_type_enum;


 /** ��������һ��IP��ַ **/
typedef struct{
	unsigned int ip[4];
} trk_struct_ip_address;

 /** ��������IP��ַ�Ͷ˿� **/
typedef struct{
	trk_struct_ip_address host;     /**< IP��ַ **/
	signed int          port;   /**< �˿� **/
	char apn[100];
} trk_struct_platform_url;


/** ��ʾһ��֪ͨʱ�䣬����ʱ�� **/
typedef struct{
	unsigned int   hour;
	unsigned int   minute;
} trk_struct_report_time_type;


typedef struct{
	unsigned short int battery_level;       /**< ��ǰ���� **/
	unsigned char               login_fail_count;    /**< ��ǰ��¼ʧ�ܴ��� **/
	tracker_work_mode_type_enum 		work_mode;     	/**< ����ģʽ **/
	unsigned char                     shutdown_reason;   /**�ػ�ԭ��**/
	unsigned short int                wake_total_min;  /**����ʱ��**/
	unsigned char                      alarm_type;  /**��������**/
}trk_global_comm_status;
	   
typedef struct{
	trk_comm_gps_repoter_info gps_info;
	//trk_comm_cell_repoter_info cell_info[MAX_CELL_NUM];
	trk_comm_wifi_repoter_info wifi_info;
	char version[TRACKER_COMM_CFG_VERSION_LENGTH];
	tracker_work_mode_type_enum work_status;          //����״̬
	command_type command; //����ָ��
	unsigned char sequence_1;
    unsigned char sequence_2;
}trk_global_comm_content_info;
	
typedef struct 
{
	unsigned char network_preference;    /**  Network preference (0=2G,1=3G,2=CAT-M1/NB-IOT,3=Both)**/// GPS_GENERAL_SETUP_READ[32] 
	reply_channel                         data_transfer_channel;     /**���ݴ��䷽ʽ **/  //GPS_GENERAL_SETUP_READ[0]
	trk_struct_platform_url              platform_url;               /**ƽ̨�����ַ **/ 
	trk_struct_platform_url              preserve_platform_url;               /**����ƽ̨�����ַ **/ 
	unsigned char    deviceid[TRK_CFG_DEVICE_ID_LENGTH]; /**< ���豸device id **/  //GPS_GENERAL_SETUP_READ[1]-GPS_GENERAL_SETUP_READ[4]
	unsigned char    tracker_phone_number[10];   /**�����绰�������� **/  //GPS_GENERAL_SETUP_READ[5]-GPS_GENERAL_SETUP_READ[14]
	caller_answer_mode_type       answer_mode;               /**�������ģʽ**/  //GPS_GENERAL_SETUP_READ[15]
	trk_struct_report_time_type location_report_time[TRK_LOCATION_REPORT_TIME_MAX];//��λ����ʱ��
	unsigned char                     charge_powon_gps;     /**����GPS **/  //GPS_GENERAL_SETUP_READ[16]
	unsigned char                     ussd_heart_beat_in_hour; /**USSD������**/ //GPS_GENERAL_SETUP_READ[17]
	unsigned char                     data_heart_beat_in_min;  /**UDP������**/ //GPS_GENERAL_SETUP_READ[18]	
	unsigned char                     beeper_enable;  /**����������**/ //GPS_GENERAL_SETUP_READ[19]
	unsigned char                     session_enable;  /**������������**/ //GPS_GENERAL_SETUP_READ[20]
	unsigned char                     reboot_timer;   /**������ʱ��**///GPS_GENERAL_SETUP_READ[21]
	unsigned char                     tracking_alarm_in_10s; /**10S���񶯴�������**/ //GPS_GENERAL_SETUP_READ[22]
	unsigned char                     tracking_heart_pulse; /**������**///GPS_GENERAL_SETUP_READ[23]
	unsigned char                     no_heart_pulse_reboot_in_min;/**�����޻ظ�������������**/ //GPS_GENERAL_SETUP_READ[24]
	unsigned char                     send_ussd_fail_num;/**����USSD���ݳ�����������**/ //GPS_GENERAL_SETUP_READ[25]
	unsigned char                     ussd_q_timer;/**����USSD���ݼ��**/ //GPS_GENERAL_SETUP_READ[26]
	unsigned char                     motion_para_1;/**g-sensor����һ**/ //GPS_GENERAL_SETUP_READ[27]
	unsigned char                     motion_para_2;/**g-sensor������**/ //GPS_GENERAL_SETUP_READ[28]
	unsigned char                     motion_para_3;/**g-sensor������**/ //GPS_GENERAL_SETUP_READ[29]
	unsigned char                     press_ack_time_out_in_sec;/**Button Press ACK Timeout**/ //GPS_GENERAL_SETUP_READ[30]
	unsigned char                     bt_enable; /**����״̬ **/   //GPS_GENERAL_SETUP_READ[31]
	unsigned char                     agps_setting[2];/**agps ����**/ //GPS_GENERAL_SETUP_READ[33] GPS_GENERAL_SETUP_READ[34]
	unsigned char                     smart_save_power_enable; /**����ʡ�� **/   //GPS_GENERAL_SETUP_READ[35]
	unsigned char                     power_off_message_enable; /**�ػ���Ϣ **/   //GPS_GENERAL_SETUP_READ[36]
	unsigned char                     watchdog_enable; /**���Ź���Ϣ **/   //GPS_GENERAL_SETUP_READ[37]
	unsigned char                     agps1_enable;/**agps ģʽ1**/ //GPS_GENERAL_SETUP_READ[38] 
	unsigned char                     agps2_enable;/**agps ģʽ2**/ //GPS_GENERAL_SETUP_READ[39]
	unsigned char                     smart_wifi_enable[4];/**����WIFI**/ //GPS_GENERAL_SETUP_READ[40]-GPS_GENERAL_SETUP_READ[43]
	unsigned char                     service_control_deviceid_enable;    /**�������޸�DEVICE_ID����**/ //GPS_GENERAL_SETUP_READ[44]
	unsigned char                     check_ack_timer_in_min;  /**���ACK TIMERʱ������**/ //GPS_GENERAL_SETUP_READ[45]
	unsigned char                     check_ack_timer_enable;  /**���ACK TIMERʱ�����ÿ���**/ //GPS_GENERAL_SETUP_READ[46]
	unsigned char                     ussd_lock_timer;  /**USSDʱ�䷢�ͳ���**/ //GPS_MSG_WAIT_DUR_READ[0]
	unsigned char                     gps_backup_enable;  /**����GPS���ݹ���**/ //GPS_BACKUP_ENABLE_READ[0]
	//kal_uint16                    ussd_q_timer; //GPS_UMBRAL_READ[2]  //�������û��ʵ��Ӧ��
	unsigned char                     current_session_max_bytes;//GPS_GENERAL_SETUP_READ[70]  ��Ҫ����256 �����Խ��
	unsigned char                     reset_sig_flag;//GPS_GENERAL_SETUP_READ[91]
	unsigned char                     auto_change_carrier;//GPS_GENERAL_SETUP_READ[92]  
	/****************************
	gps on when charging �ĵڶ���bit�������Ƴ�����10���Ӻ�غ�ƣ�ʡ�ýӳ�籦��ƺĵ�
	1: turn off red led after fully charged for 10 min.
	0: red led always on after fully charged.
	******************************/
	unsigned char                     full_battery_led_off;     /**����ص� **/  
} trk_global_usr_settings;

/***************************************************************************************
	g_trk_var.g_trk_settings.track_channel - "Tracking Channel" as in the old message.
	g_trk_var.g_trk_settings.sample_time - "Sample Time (mins)" as in the old message.
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
typedef struct 
{
	BOOL bInit;
	trk_struct_report_time_type location_report_time[TRK_LOCATION_REPORT_TIME_MAX];//��λ����ʱ��
	unsigned int location_report_interval;     /**<��λ���������룩 **/
	reply_channel  track_channel;  //GPS_TRACKING_WITH_MEASUREMENT_READ[0]
	unsigned char  sample_time;  //GPS_TRACKING_WITH_MEASUREMENT_READ[1]
	unsigned char  report_time;  //GPS_TRACKING_WITH_MEASUREMENT_READ[2] 
	unsigned char  location_accuracy; //	GPS_TRACKING_WITH_MEASUREMENT_READ[3] 
	unsigned char  track_setting; //	GPS_TRACKING_WITH_MEASUREMENT_READ[4] 
	unsigned char  Xmode_smaple_time;  //GPS_XMODE_PARAMETERS_READ[0]
	unsigned char  Xmode_report_time;  //GPS_XMODE_PARAMETERS_READ[2]
	reply_channel  Xmode_channel;  //GPS_XMODE_PARAMETERS_READ[3]
	unsigned char  Xmode_location_accuracy;  //GPS_XMODE_PARAMETERS_READ[4]
	unsigned char  Xmode_transmit_deviceid;  //GPS_XMODE_PARAMETERS_READ[5] Transmit Device ID
	unsigned char  Xmode_delta_compression;  //GPS_XMODE_PARAMETERS_READ[6] Delta Compression
	unsigned char  gps_search_control[8]; //GPS_SEARCH_CONTROL_READ
	unsigned char  gps_data_timer[2];  //GPS_DATA_TIMER_READ[2]
	unsigned char  gps_moving_alarm_setting[7]; //GPS_MOVING_ALARM_SETTING_READ[7];
	unsigned char  gps_stop_moving_alarm_setting[7]; //GPS_STOP_MOVING_ALARM_SETTING_READ[6];
	unsigned char  gps_schedule_sleep[10];//GPS_SCHEDULE_SLEEP_READ[20]
	unsigned char  gps_ack_setting[10]; //GPS_ACK_SETTING_READ[10]
	unsigned char  gps_ussd_prefix_read[50];//GPS_USSD_PREFIX_READ[50]
	unsigned char  gps_apn_string[50]; //GPS_APN_STRING_READ[50]
	unsigned char  gps_apn_user[50]; //GPS_APN_USER_READ[50]
	unsigned char  gps_apn_pw[50];//GPS_APN_PW_READ[50]
	unsigned char  gps_fance_cordinates[20][20];//GPS_FENCE_CORDINATES_READ
	unsigned char  gps_fance_alarm_setting[5][10];//GPS_FENCE_ALARM_SETTING_READ
	unsigned char  gps_speed_alarm_setting[5][10];//GPS_SPEED_ALARM_SETTING_READ
	unsigned char  gps_battery_alarm_setting[5][10];//GPS_BATTERY_ALARM_SETTING_READ
}trk_global_settings;

/** �����豸��ص����ã���Щ������Ҫд��NVRAM **/
typedef struct{
	unsigned char deviceid_2G[TRK_CFG_DEVICE_ID_LENGTH]; /**< ���豸device id **/  //GPS_GENERAL_SETUP_READ[1]-GPS_GENERAL_SETUP_READ[4]
	char imsi_2G[TRK_CFG_IMSI_LENGTH]; /**< �󶨵�imsi���� **/
	char imei_2G[TRK_CFG_IMEI_LENGTH]; /**< �󶨵�imei **/
	char iccid_2G[TRK_CFG_ICCID_LENGTH]; /**< �󶨵�iccid **/ 
	unsigned char deviceid_4G[TRK_CFG_DEVICE_ID_LENGTH]; /**< ���豸device id **/
	char imsi_4G[TRK_CFG_IMSI_LENGTH]; /**< �󶨵�imsi���� **/
	char imei_4G[TRK_CFG_IMEI_LENGTH]; /**< �󶨵�imei **/
	char iccid_4G[TRK_CFG_ICCID_LENGTH];/**< �󶨵�iccid **/ 
	unsigned char power_on_status; /**����״̬ **/
	unsigned char botton_id;  /**���� GPS botton_id**/
} trk_global_device_status;
	
typedef struct _trk_variable
{
	trk_global_device_status g_trk_device_status; /**< �����豸������� **/
	trk_global_settings g_trk_settings;            /**< ����ƽ̨ȫ������ **/
	trk_global_usr_settings g_user_settings;            /**< �����û�ȫ������ **/
	trk_global_comm_content_info g_trk_content_info;  //��ȡ�ı���������Ϣ
	trk_global_comm_status g_trk_status;  //tracker�����״̬
}trk_variable;

void Tracker_init(void);
void lte_network_status_callback_func(void);
void tracker_start_work(void);
void Tracker_network_start(void);
void tracker_work_status(void);
void mmi_tracker_set_deviceid_by_imei(char* imei,tracker_work_mode_type_enum work_mode);
void idle_lte_set_imei_callback_func(char* imei,int len);
void idle_lte_sim_status_callback_func(char* imsi,int len);
void agps_setting_init(void);
void powerup_tracking_start(void);
extern trk_variable g_trk_var;
