#ifndef MY_WIFI_FHQ_20190722_H
#define MY_WIFI_FHQ_20190722_H

#include "MyDefine.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
#define TRACKER_MAX_SCAN_AP_NUM 30
#define TRACKER_MAC_ADDRESS_LEN 6
#define TRACKER_SSID_MAX_LEN  32

typedef enum {
	WIFI_EVENT_INIT_DONE,
	WIFI_EVENT_CONNECTED,
	WIFI_EVENT_DISCONNECTED,
	WIFI_EVENT_SCAN_COMPLETE,
	WIFI_EVENT_MAX
} enumWifiEvent;

typedef enum
{
	WIFI_READ_FROM_NO_SERVER_REPLY,    
	WIFI_READ_FROM_NEED_SERVER_REPLY,  
	WIFI_READ_FROM_GET_GPS,
	WIFI_READ_FROM_BOTTON,
	#ifdef __TRACKIMO_SMART_WIFI__
	WIFI_READ_FROM_XMODE_WIFI_POWER_SAVING,
	WIFI_READ_FROM_FENCE_SPEED_WIFI_POWER_SAVING,
	WIFI_READ_FROM_SMART_WIFI_KEY,
	#endif
	WIFI_READ_FROM_TOTAL 
} wifi_read_request_enum;

typedef struct
{
    U8 ssid_len;
    U8 ssid [ TRACKER_SSID_MAX_LEN];    
    U8 bssid[TRACKER_MAC_ADDRESS_LEN]; /* MAC address */
    U8  channel_number;    
    U32 frequency;
    U8 rssi;	
}Tracker_wifi_ap_info_t;

typedef struct
{
    U8  ap_list_num;
    Tracker_wifi_ap_info_t ap_list[TRACKER_MAX_SCAN_AP_NUM];
} trk_comm_wifi_repoter_info;

void wifi_driver_init(void);
void wifi_on(void);
void wifi_off(void);
int wifi_get_rssi(void);
void wifi_get_mac_address(char * pBuffer, int iLen);
void wifi_send_data(char * pData, int iDataLen);
void wifi_start_scan(void);
void wifi_stop_scan(void);
void setScan(BOOL value);
void lwip_on(void);
void lwip_off(void);
void tracker_read_wifi_and_send(wifi_read_request_enum wifi_request);
void tracker_send_gsm_wifi_without_reply(void);
#if defined(__cplusplus)
}
#endif 

#endif //MY_WIFI_FHQ_20190722_H



