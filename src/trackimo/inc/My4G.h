#ifndef MY_4G_FHQ_20190722_H
#define MY_4G_FHQ_20190722_H

#include "MyDefine.h"
#include "TrackimoFrame.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

typedef enum {
	MODULE_4G_EVENT_POWER_ON,
	MODULE_4G_EVENT_POWER_OFF,
	MODULE_4G_EVENT_RESTART_OK,
	MODULE_4G_EVENT_SEND_GPS_DATA,
	MODULE_4G_EVENT_SEND_WIFI_DATA,
	MODULE_4G_EVENT_SEND_OVER,
	MODULE_4G_EVENT_SEND_FAILURE,
	MODULE_4G_EVENT_NEW_DATA_INCOMING,
	MODULE_4G_EVENT_SMS_PARSE,
	MODULE_4G_EVENT_MAX,
} enum4GEvent;

typedef enum{
	NET_STATE_NOT_INIT,
	NET_STATE_SEARCHING,
	NET_STATE_NOT_INSERT,
	NET_STATE_LOCAL_OK,
	NET_STATE_ROAM,
	NET_STATE_LIMIT,
	NET_STATE_MAX
}enumNetStatus;

typedef enum{
	NET_SYSTEM_GERAN,
	NET_SYSTEM_UTRAN,
	NET_SYSTEM_3GPP,
	NET_SYSTEM_EUTRAN,
	NET_SYSTEM_GERAN_UTRAN,
	NET_SYSTEM_GERAN_EUTRAN,
	NET_SYSTEM_UTRAN_EUTRAN,
	NET_SYSTEM_MAX
}enumNetSystem;

typedef enum{
	MODULE_4G_RF0,
	MODULE_4G_RF1,
	MODULE_4G_RF2,
	MODULE_4G_RF3,
	MODULE_4G_RF4,
	MODULE_4G_RF_MAX
}enum4grf;

typedef struct
{
	U32 mode;                                                       
	U32 connid;                       
	BOOL  is_data_account_ok;
}module_struct_soc_info;

typedef enum
{
	MODULE_POWEROFF_STATE = 0,
	MODULE_POWERON_STATE,
	MODULE_STATE_TOTAL
} module_state_enum;

typedef struct
{
	U32 config;                                                       
	U32 index;                       
}sms_struct_soc_info;

typedef struct
{
	module_state_enum module_state;
	U8 module_work_flag;
	U8 sim_insert_flag;
	U8 lte_network_resgister_flag;
	U8 at_send_enable;
	module_struct_soc_info module_soc_info;                                  /** socket参数设置 **/
	sms_struct_soc_info sms_default_set;                                  /** sms参数设置 **/
} module_context_struct;

//BOOL my_4g_buffer_msgin_free(tracker_struct_msg * pMsg);
//BOOL my_4g_buffer_msgout_free(tracker_struct_msg * pMsg);
//tracker_struct_msg * my_4g_buffer_msgin_malloc(void);
//tracker_struct_msg * my_4g_buffer_msgout_malloc(void);
void module_4g_driver_init(void);
void module_4g_on(void);
void module_4g_off(void);
void module_4g_reset(void);
void module_4g_entry_sleep(void);
void module_4g_exit_sleep(void);
void module_4g_send_trackimo_data_step2(void);
void module_4g_send_sms(char * pData, int iDataLen);
BOOL module_4g_receive_data(char * pData, int iDataLen);
void module_data_to_send(void);
void module_send_data_callback_func(void);
BOOL module_4g_send_uart_data(char * pData, int iDataLen);
BOOL isSendingTrackimoData(void);
void module_read_uart_buffer_clear(void);
void take_4g_semaphore(char * pReceive, int * pReceiveLen, char * pSend, BOOL bAutoDelete);
void give_4g_semaphore(void);
#if defined(__cplusplus)
}
#endif 

#endif //MY_4G_FHQ_20190722_H



