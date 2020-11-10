#ifndef TRACKIMO_FRAME_FHQ_20190902_H
#define TRACKIMO_FRAME_FHQ_20190902_H
#include "MyDefine.h"
#define  TRACKER_COMM_QUEUE_BUFFER_SIZE 256
#define COMMAND_GET_GPS_COMPRESSION_TYPE  COMPRESSION_1

typedef struct tracker_struct_msg
{
     int msgLen;
     U8  body_msg[TRACKER_COMM_QUEUE_BUFFER_SIZE];
} tracker_struct_msg;

typedef struct
{
	U8 carrier_name_buffer[2048];//2048,最大可读到32个operator，多设点buffer防止溢出
	int len;
}carrier_name_struct;

typedef void (*pfTrackimoSendFunction)(int);
typedef void (*pfTrackimoReceivsFunction)(tracker_struct_msg *message); 
void XmodeStart(void);
void trackimo_receive_deal(tracker_struct_msg *pMsg);
void tracker_protocol_send_deal(int encode_id,int data_channel);
void do_cordinates_compression(double latitude,double longitude, double speed, char compression_type);
U8 get_one_byte_uint_from_hex_asc(U8 bit4_0, U8 bit4_1);
U8 bit_of_byte(U8 byte_1, U8 number_th);
BOOL check_ack_setting_bit(U8 check_bit);
U8 get_ussd_q_timer(void);
U8 get_motion_para1(void);
U8 get_motion_para2(void);
U8 get_motion_para3(void);
void fixed_backup_history_buffer_creat(void);
void fixed_backup_history_buffer_init(void);
U16 get_number_of_records_in_backup_requested_history_buffer(void);
U8 get_uint16_byte_high(U16 uint16_number);
U8 get_uint16_byte_low(U16 uint16_number);
U8 get_epo_power_up_status(void);
void resend_button_stop(void);
void stop_new_tracking_because_of_old_tracking(void);
void do_heart_pulse_fail_reboot(void);
void set_pulse_timer(U8 pulse_hour);
void heart_pulse_do(void);
void get_gps_send_gsm_1(void);
void BACKEND_GPS_POWERON_SPECIAL(void);
void bcd_transform_back(char* str_ori,char* str_back);
void fence_para_reset_sig(U8 fence_number);
void tracking_with_measurement_start(void); 
void check_no_gsm_history(U8 report_times);
void  tracking_with_measurement_in_min(void);
void tracking_with_measurement_in_more_than_1_min(void);
void schedule_sleep_start(void);
void check_smart_power_saving_setting(void);
void tracker_dynamic_analyze(U8 id1,U8 id2,U8 dynamic_item); 
void tracker_send_deivce_status(void);
void single_gsm_gps_location_report(void);
void tracker_sos_send(void);
#endif //TRACKIMO_PROTOCOL_FRAME_FHQ_20190902_H



