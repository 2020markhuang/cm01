
/**
@brief 本文件定义了消息函数
对输入信息信息进行解码，然后丢进队列进行执行
$Id: TrackerDecodeIn.h  $
**/

#include "TrackimoFrame.h"

void server_msg_analyze_ack(tracker_struct_msg *message);

void server_msg_analyze_tracking_mode_start(tracker_struct_msg *message);

void server_msg_analyze_tracking_mode_stop(tracker_struct_msg *message);

void server_msg_analyze_general_setup(tracker_struct_msg *message);

void server_msg_analyze_testmode(tracker_struct_msg *message);

void server_msg_analyze_status(tracker_struct_msg *message);

void server_msg_analyze_delete_file(tracker_struct_msg *message);

void server_msg_analyze_beep(tracker_struct_msg *message);

void server_msg_analyze_reboot(tracker_struct_msg *message);

void server_msg_analyze_backup_enable(tracker_struct_msg *message);

void server_msg_analyze_gps_search_mode(tracker_struct_msg *message);

void server_msg_analyze_clear_all_alarm(tracker_struct_msg *message);

void server_msg_analyze_get_history(tracker_struct_msg *message);

void server_msg_analyze_voice_call(tracker_struct_msg *message);

void server_msg_analyze_get_carriers(tracker_struct_msg *message);

void server_msg_analyze_get_gps(tracker_struct_msg *message);

void server_msg_analyze_get_gps_no_ack(void);

void server_msg_analyze_fence_alarm(tracker_struct_msg *message);

void server_msg_analyze_fence_alarm_onoff(tracker_struct_msg *message);

void server_msg_analyze_speed_alarm(tracker_struct_msg *message);

void server_msg_analyze_moving_alarm(tracker_struct_msg *message);

void server_msg_analyze_battery_alarm(tracker_struct_msg *message);	

void server_msg_analyze_set_apn(tracker_struct_msg *message); 

void server_msg_analyze_set_active_carrier(tracker_struct_msg *message);

void server_msg_analyze_set_session_max_bytes(tracker_struct_msg *message);

void server_msg_analyze_ota_start(tracker_struct_msg *message);

void server_msg_analyze_ota_start(tracker_struct_msg *message);

void server_msg_analyze_ack_setting(tracker_struct_msg *message);

void server_msg_analyze_start_tracking_mode_with_measurement(tracker_struct_msg *message);

void server_msg_analyze_schedule_sleep(tracker_struct_msg *message);

void server_msg_analyze_get_bt_mac(tracker_struct_msg *message);

void server_msg_analyze_schedule_sleep_off(tracker_struct_msg *message);

void server_msg_analyze_general_setup_with_net_identifiers(tracker_struct_msg *message);

void server_msg_analyze_test_component(tracker_struct_msg *message);

void server_msg_analyze_agps_bootstrap_location(tracker_struct_msg *message);

void server_msg_analyze_set_components_dynamic(tracker_struct_msg *message);

#ifdef __TRACKIMO_SMART_WIFI__
void server_msg_analyze_set_smart_wifi(tracker_struct_msg *message);

void server_msg_analyze_set_smart_wifi_table(tracker_struct_msg *message);
#endif

void server_msg_analyze_reset_session(tracker_struct_msg *message);

void server_msg_analyze_ussd_q_timer(tracker_struct_msg *message);

void server_msg_analyze_motion_count(tracker_struct_msg *message);

void server_msg_analyze_auto_change_carrier(tracker_struct_msg *message);

