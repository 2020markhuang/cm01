/**
@brief 本文件定义了消息函数
对输出信息进行编码，然后丢进队列保存发送
$Id: TrackerEncodeOut.h $
**/


#include "TrackimoFrame.h"
// 业务相关的常量
#define  TRACKER_PROTOCOL_CONTENT_MAX_LENGTH 160

void tracker_reply_server_msg_success(reply_channel data_channel);

void tracker_server_msg_analyze_status_1(reply_channel data_channel); 

void tracker_alm_msg_send(reply_channel data_channel);

void tracker_server_msg_analyze_get_imsi(reply_channel data_channel);

void tracker_sent_carrier(reply_channel data_channel);

void tracker_sent_carrier_error(reply_channel data_channel);

void tracker_send_gps_info_to_server(reply_channel data_channel);

void tracker_send_first_tower_info(reply_channel data_channel);

void tracker_server_msg_analyze_status_2(reply_channel data_channel);

void tracker_protocol_encode_powerup(reply_channel data_channel);

void tracker_powerdown_notify(reply_channel data_channel);

void tracker_send_history_data(reply_channel data_channel);

void tracker_caller_id_notify(reply_channel data_channel);

void tracker_data_session_start_msg(reply_channel data_channel);

void tracker_data_session_stop_msg(reply_channel data_channel);

void tracker_heart_pulse_send(reply_channel data_channel);

void trakcer_send_multi_tower_info(reply_channel data_channel);

void tracker_shutdown_message_send(reply_channel data_channel);

void tracker_server_msg_analyze_get_bt_mac(reply_channel data_channel);

void tracker_server_msg_analyze_status_3(reply_channel data_channel);

void tracker_send_button_without_wifi(reply_channel data_channel);

void tracker_server_msg_analyze_status_4(reply_channel data_channel);

void tracker_reply_server_sequence_success(reply_channel data_channel);

void tracker_server_msg_analyze_test_component(reply_channel data_channel);


void tracker_smart_wifi_key_send_only_wifi_info(reply_channel data_channel);

void tracker_send_only_wifi_info(reply_channel data_channel,BOOL server_reply);

void tracker_send_wifi_info_to_server(reply_channel data_channel);

void tracker_server_msg_analyze_status_dynamic(reply_channel data_channel);

void tracker_device_send_get_ack(reply_channel data_channel);

void tracker_reply_server_msg_error(reply_channel data_channel);
