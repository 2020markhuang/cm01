#ifndef MY_GPS_FHQ_20190722_H
#define MY_GPS_FHQ_20190722_H
#include "MyDefine.h"
#include "WatchDateTime.h"
#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
#define MAX_GPS_SATELLITES 12
#define GPS_LAST_NUMBER_SEC 20
#define M_PRN_NUM  32

typedef struct
{
    U8 num;		//���Ǳ��
    U8 eledeg;	//��������
    U16 azideg;	//���Ƿ�λ��
    U8 sn;		//�����		  
} trk_struct_prn;

typedef enum
{
    GPS_ON_FROM_COMMAND_GET_GPS = 0,   
    GPS_ON_FROM_COMMAND_XMODE,   
    GPS_ON_FROM_COMMAND_TESTMODE, 
    GPS_ON_FROM_COMMAND_FENCE_OR_SPEED,
    GPS_ON_FROM_CHARGER, 
    GPS_ON_FROM_MAX   
} gps_on_from_enum;

typedef enum {
	GPS_EVENT_POWER_ON,
	GPS_EVENT_POWER_OFF,
	GPS_EVENT_LOCATED,
	GPS_EVENT_NEW_DATA,
	GPS_EVENT_TIME_CALIBRATE,
	GPS_EVENT_MAX
} enumGPSEvent;

typedef enum
{
    GPS_EMPTY = 0,   
    GPS_FIXED,  
    GPS_BACKUP,       
    GPS_NO_FIX,    
} gps_save_status_enum;

typedef enum
{
    GPS_STATUS_OFF_NOTHING = 0,    //gps�ر�״̬  
    GPS_STATUS_ON_SEARCHING,     //gps��������������
    GPS_STATUS_ON_2D_FIX,      //gps������2d fix
    GPS_STATUS_ON_3D_FIX   //gps������3d fix
} trk_enum_gps_state;

typedef struct{
	unsigned second:6;
	unsigned minute:6;
	unsigned hour:5;
	unsigned day:5;
	unsigned month:4;
	unsigned year:6;
}structUTC;

/** ��ʾһ������ʱ�䣬����������ʱ���� **/
typedef struct{
	U8  year;
	U8   month;
	U8   day;
	U8   hour;
	U8   minute;
	U8   second;
} trk_comm_struct_time_type;

typedef struct{
	trk_comm_struct_time_type date_time;
	int time;
	int longitude;	 //γ�� ������1000000��,ʵ��Ҫ����1000000
	int latitude;	//γ�� ������1000000��,ʵ��Ҫ����1000000
	int altitude;    //���θ߶�,�Ŵ���10��,ʵ�ʳ���10.��λ:0.1m
	trk_enum_gps_state gps_state;              //GPS״̬
	U8 satellite_using_num;                   //����ʹ�õ����ǵ�����
	U8 satellite_visable_num;                   //�������ǵ���
	U16 hdop;                    //ˮƽ�������� 0~500,��Ӧʵ��ֵ0~50.0
	U16 vdop;                //��ֱ�������� 0~500,��Ӧʵ��ֵ0~50.0 
	U16 pdop;                //λ�þ������� 0~500,��Ӧʵ��ֵ0~50.0
	int differ_time;                //���ʱ��
	U16 differ_id;                  //���վID��0000 - 1023
	U8 pos_mode;    //��λģʽ 0�Զ���1���ֶ�
	U8 pos_type;    //��λ���� 0:δ��λ 1��2D��λ 2��3D��λ
	trk_struct_prn prn_info[M_PRN_NUM]; //ÿ��PRN���Ӧ����Ϣ	
    int max_snr;     //�ź�ǿ��
	int T_angle;   //���汱Ϊ�ο���׼���˶��Ƕ�    /*Track mode degrees,north is 0*/
	U16 speed_km;    //��������,�Ŵ���1000��,ʵ�ʳ���10.��λ:0.001����/Сʱ
	U8 magnetic_declination;    //��ƫ��
}trk_comm_gps_repoter_info;

typedef struct
{
	//type,1�ֽ�
	gps_save_status_enum      type;
	//ʱ�䣬6�ֽ�
	U8      year;
	U8      month;
	U8      day;
	U8      hour;
	U8      minute;
	U8      second;
	//double 8�ֽ�
	double     longitude;
	//double 8�ֽ�
	double     latitude;
	//double 8�ֽ�
	double     altitude;    
	//double 8�ֽ�
	double     speed;
	U8  battery;
	#ifdef __USE_NEW_AGPS_FUNCTION__	
	//trk_comm_cell_repoter_info    cell_info[6];             /*С������Ϣ*/
	#endif	
}trackergps_backup_data_struct;

typedef enum{
	PERODIC_TYPE_NORMAL = 0,
	PERODIC_TYPE_PERODIC_BACKUP = 1,
	PERODIC_TYPE_PERODIC_STANDBY = 2,
	PERODIC_TYPE_PERPETUAL_BACKUP = 4,
	PERODIC_TYPE_ALWAYS_STANDBY = 8,
	PERODIC_TYPE_ALWAYS_BACKUP = 9,	
}enumGPSPerodicType;

typedef enum{
	GPS_LOCATED_NOT_YET = 1,
	GPS_LOCATED_2D,
	GPS_LOCATED_3D,
	GPS_LOCATED_MAX
}enumGPSLocateStatus;
typedef enum{
	PMTK_TEST = 0,
	PMTK_ACK = 1,
	PMTK_SYS_MSG = 10,
	PMTK_TXT_MSG = 11,
	PMTK_CMD_HOT_START = 101,
	PMTK_CMD_WARM_START = 102,
	PMTK_CMD_COLD_START = 103,
	PMTK_CMD_FULL_COLD_START = 104,
	PMTK_CMD_CLEAR_EPO = 127,
	PMTK_CMD_STANDBY_MODE = 161,
	PMTK_LOCUS_QUERY_STATUS = 183,
	PMTK_LOCUS_ERASE_FLASH = 184,
	PMTK_LOCUS_STOP_LOGGER = 185,
	PMTK_LOCUS_LOG_NOW = 186,
	PMTK_LOCUS_CONFIG = 187,
	PMTK_SET_POS_FIX = 220,
	PMTK_SET_AL_DEE_CFG = 223,
	PMTK_SET_PERIODIC_MODE = 225,
	PMTK_SET_DATA_PORT = 250,
	PMTK_SET_NMEA_BAUDRATE = 251,
	PMTK_SET_SYNC_PPS_NMEA = 255,
	PMTK_SET_GLP_MODE = 262,
	PMTK_SET_NMEA_REPORT_INTERVAL = 263,
	PMTK_SET_NMEA_REPORT_CONDITION = 264,
	PMTK_SET_PPS_CONFIG_CMD = 285,
	PMTK_SET_AIC_CMD = 286,
	PMTK_SET_OUTPUT_DEBUG = 299,
	PMTK_API_SET_FIX_CTL = 300,
	PMTK_API_SET_SBAS_ENABLED = 313,
	PMTK_API_SET_NMEA_OUTPUT = 314,
	PMTK_API_SET_PPS = 326,
	PMTK_API_SET_GNSS_SEARCH_MODE = 353,
	PMTK_API_QUERY_GNSS_SEARCH_MODE = 355,
	PMTK_API_SET_HDOP_THRESHOLD = 356,
	PMTK_API_GET_HDOP_THRESHOLD = 357,
	PMTK_API_SET_STATIC_NAV = 386,
	PMTK_API_Q_FIX_CTL = 400,
	PMTK_API_Q_SBAS_ENABLED = 413,
	PMTK_API_Q_NMEA_OUTPUT = 414,
	PMTK_DT_FIX_CTL = 500,
	PMTK_DT_SBAS_ENABLED = 513,
	PMTK_DT_NMEA_OUTPUT = 514,
	PMTK_Q_RELEASE = 605,
	PMTK_Q_EPO_INFO = 607,
	PMTK_Q_LOCUS_DATA = 622,
	PMTK_DT_RELEASE = 705,
	PMTK_SAT_DATA  = 721,
	PMTK_DT_UTC  = 740,
	PMTK_DT_POS  = 741,
	PMTK_WAKE_UP = 850,
}enumPMTKCmd;

void gps_set_search_mode(BOOL bGPS, BOOL bGlonass, BOOL bCalileo, BOOL bCalileoFull, BOOL bBeido); 	
void gps_set_report_condition(BOOL bEnable); 	
void gps_set_report_interval(int interval, BOOL bPowerSaving);
void gps_set_glp_mode(BOOL bEnable);
void gps_set_periodic_mode(int type, int runtime, int sleeptime, int sec_runtime, int sec_sleeptime) ;
void gps_set_dt_pos(int lon, int lat, int alt, int year, int month, int day, int hour, int minute, int second) ;
void gps_set_dt_uts(int year, int month, int day, int hour, int minute, int second) ;
void gps_set_static_nav_thd(double speed) ;
void gps_host_start(void) ;
void gps_cold_start(void);
void gps_warm_start(void);
void gps_entry_glonass_mode(void) ;
void gps_speed_lock(void);
void gps_entry_periodic_mode(void);
void gps_wake_up(void); 	

void gps_driver_init(void);
void GPS_POWERON(void);
void GPS_POWEROFF(void);
void gps_entry_sleep(void);
void gps_exit_sleep(void);
int gps_read_data(char * pBuf, int iBufSize);
BOOL gps_receive_data(char * pData, int iDataLen);
void CheckIfTurnOffGPS(void);
BOOL Is_fence_speed_gps_on(void);
BOOL Is_gps_searching(void);
BOOL Is_get_gps_on(void);
void FENCE_OR_SPEED_COMMAND_TURN_ON_GPS(void);
int get_gps_onoff_total_sec(void);
void GET_GPS_COMMAND_TURN_ON_GPS(void);
void GET_GPS_COMMAND_TURN_OFF_GPS(void);
BOOL gps_2d_3d_fix_with_hdop_and_fix_sec(U16 fix_sec);
void gps_last_20s_sat_info_q_reset(void);
void sat_info_q_push(U8 sat_number, U8 max_snr);
BOOL all_sat_info_zero_in_last_20s(void);
BOOL gps_searching_all_zero(void);
void XMODE_COMMAND_TURN_OFF_GPS(void);
BOOL Is_xmode_gps_on(void);
void XMODE_COMMAND_TURN_ON_GPS(void);
void gps_set_sat_data(int * pData, int iIndex);
void gps_set_dt_uts_by_stamp(structTimeStamp time);
void nmea_parse_time(char* pTime, structUTC * pUTC);
void nmea_parse_date(char* pDate, structUTC * pUTC);
#if defined(__cplusplus)
}
#endif 

#endif //MY_GPS_FHQ_20190722_H



