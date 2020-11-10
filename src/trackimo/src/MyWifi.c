#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "hal.h"
#include "wifi_host_api.h"
#include "wifi_host_private_api.h"
#include "lwip_network.h"
#include "MyCommon.h"
#include "MainTask.h"
#include "MyWifi.h"
#include "TrackerMain.h"
#include "MyLog.h"
#include "MyTimer.h"
#define WIFI_MAX_SCAN_LIST_ITEM 5
static wifi_scan_list_item_t static_aplist[WIFI_MAX_SCAN_LIST_ITEM];
BOOL wifi_read_from[WIFI_READ_FROM_TOTAL];
BOOL wifi_running = FALSE;
#ifdef SUPPORT_MY_WIFI
extern const char BSP_WIFI_ENABLE_PIN;
extern const char BSP_WIFI_32K_PIN;
extern const char BSP_WIFI_32K_PIN_M_GPIO;
extern const char BSP_WIFI_32K_PIN_M_CLKO;
extern const char BSP_WIFI_RESET_PIN;
extern U32 g_scan_list_size;
extern void lwip_network_deinit(void);
void tracker_read_wifi_reset(void)
{
	main_printf("tracker_read_wifi_reset");
	 U8 i=0;
	 stopMyTimer(TRACKER_WIFI_TIMER_1);
	 for(i=0;i<WIFI_READ_FROM_TOTAL;i++)
	 {
	 	  wifi_read_from[i] = FALSE;
	 }	  
	 wifi_running = FALSE;
}	
void wlan_scan_only_tracker_bootup(void)
{
	BOOL chip_valid;
	main_printf("...............................wlan_scan_only_tracker_bootup..................\r\n");
	if(wifi_running == FALSE)
	{
		main_printf("wifi_running == KAL_FALSE GO!\r\n");
		wifi_running = TRUE;
		stopMyTimer(TRACKER_WIFI_TIMER_1);
		startMyTimer(TRACKER_WIFI_TIMER_1, 10000, tracker_read_wifi_reset);
	}
	else
	{
		main_printf("wifi_running == KAL_TRUE dont go wifi\r\n");
		stopMyTimer(TRACKER_WIFI_TIMER_1);
		startMyTimer(TRACKER_WIFI_TIMER_1, 10000,tracker_read_wifi_reset);	
		return;
	}
	memset(& g_trk_var.g_trk_content_info.wifi_info, 0, sizeof(trk_comm_wifi_repoter_info));
	wifi_off();
	wifi_on();
}

void tracker_read_wifi_and_send(wifi_read_request_enum wifi_request)
{
	 main_printf("tracker_read_wifi_and_send\r\n");
	 wifi_read_from[wifi_request] = TRUE;	
	  //start wifi
	 wlan_scan_only_tracker_bootup(); 
}

void tracker_send_gsm_wifi_without_reply(void)
{
	main_printf("tracker_send_gsm_wifi_without_reply\r\n");
	tracker_read_wifi_and_send(WIFI_READ_FROM_NO_SERVER_REPLY);
}

int wlan_get_FrequencyByChannel(U8 channel) 
{
	int frequency = 2412;
	switch (channel) {
		case 1:
		 	frequency = 2412;
		 	break;
		 case 2:
			 frequency = 2417;
			 break;
	 	case 3:
			 frequency = 2422;
		 	break;
		 case 4:
		 	frequency = 2427;
			 break;
		 case 5:
			 frequency = 2432;
		 	break;
	 	case 6:
		 	frequency = 2437;
		 	break;
		 case 7:
			 frequency = 2442;
		 	break;
		 case 8:
		 	frequency = 2447;
			 break;
	 	case 9:
			 frequency = 2452;
			 break;
	 	case 10:
		 	frequency = 2457;
		 	break;
		 case 11:
			 frequency = 2462;
		 	break;
	 	case 12:
			 frequency = 2467;
			 break;
	 	case 13:
		 	frequency = 2472;
		 	break;
		 case 14:
		 	frequency = 2484;
		 	break;
	   }
	 return frequency;
}

static int scan_event_handler_sample(wifi_event_t event_id, unsigned char *payload, unsigned int len)
{
	int handled = 0;
	int i = 0;
	wifi_scan_list_item_t * pItem = static_aplist;
	switch (event_id) {
	case WIFI_EVENT_IOT_SCAN_COMPLETE:
	handled = 1;
	for (i = 0; i < WIFI_MAX_SCAN_LIST_ITEM; i++,pItem++) {
	main_printf("\nCh\tSSID\t\t\t\t\tBSSID\t\t\tAuth\tCipher\tRSSI\tWPS_EN\tCM\tDPID\tSR\r\n");
	main_printf("%d\t", pItem->channel);
	main_printf("%-33s\t", pItem->ssid);
	main_printf("%02x:%02x:%02x:%02x:%02x:%02x\t",
	       pItem->bssid[0],
	       pItem->bssid[1],
	       pItem->bssid[2],
	       pItem->bssid[3],
	       pItem->bssid[4],
	       pItem->bssid[5]);
	main_printf("%d\t", pItem->auth_mode);
	main_printf("%d\t", pItem->encrypt_type);
	main_printf("%d\t", pItem->rssi);
	main_printf("%d\t", pItem->is_wps_supported);
	main_printf("%d\t", pItem->wps_element.configuration_methods);
	main_printf("%d\t", pItem->wps_element.device_password_id);
	main_printf("%d\t", pItem->wps_element.selected_registrar);
	main_printf("\r\n");
	g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[0] = pItem->bssid[0];
	g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[1] = pItem->bssid[1];
	g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[2] = pItem->bssid[2];
	g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[3] = pItem->bssid[3];
	g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[4] = pItem->bssid[4];
	g_trk_var.g_trk_content_info.wifi_info.ap_list[i].bssid[5] = pItem->bssid[5];
	g_trk_var.g_trk_content_info.wifi_info.ap_list[i].rssi = pItem->rssi;
	g_trk_var.g_trk_content_info.wifi_info.ap_list[i].channel_number = pItem->channel;
	g_trk_var.g_trk_content_info.wifi_info.ap_list[i].frequency = wlan_get_FrequencyByChannel(pItem->channel);
	g_trk_var.g_trk_content_info.wifi_info.ap_list[i].channel_number=pItem->channel;
	memcpy ((char*) g_trk_var.g_trk_content_info.wifi_info.ap_list[i].ssid, (const char*)pItem->ssid, sizeof(pItem->ssid));
	}
	if(g_trk_var.g_trk_content_info.wifi_info.ap_list[0].channel_number > 0)
	{
		setScan(TRUE);
		tracker_protocol_send_deal(REPLY_AGPS_BOOTSTRAP_REQUEST,g_trk_var.g_user_settings.data_transfer_channel);     
	}
	//main_printf("[MTK Event Callback Sample]: Scan Done \r\n");
	main_task_post_message(MAIN_MSG_WIFI_EVENT, WIFI_EVENT_SCAN_COMPLETE, 0);
	break;
	}
	return handled;
}
void wifi_driver_init(void){
	tcpip_init(NULL, NULL);
}
void lwip_on(void){
	main_printf("lwip_on\r\n");
	setLwipOn(TRUE);
	lwip_network_init(WIFI_MODE_STA_ONLY);
	lwip_net_start(WIFI_MODE_STA_ONLY);		
}
void lwip_off(void){
	main_printf("lwip_off\r\n");
	setLwipOn(FALSE);
	lwip_network_deinit();
	lwip_net_stop(WIFI_MODE_STA_ONLY);
}

void wifi_rf_on(void){
hal_pinmux_set_function(HAL_GPIO_8, HAL_GPIO_8_GPIO8);
hal_gpio_set_direction(HAL_GPIO_8, HAL_GPIO_DIRECTION_OUTPUT);
hal_gpio_set_output(HAL_GPIO_8, HAL_GPIO_DATA_LOW);
hal_pinmux_set_function(HAL_GPIO_9, HAL_GPIO_9_GPIO9);
hal_gpio_set_direction(HAL_GPIO_9, HAL_GPIO_DIRECTION_OUTPUT);
hal_gpio_set_output(HAL_GPIO_9, HAL_GPIO_DATA_HIGH);
}

void wifi_rf_off(void){
hal_pinmux_set_function(HAL_GPIO_8, HAL_GPIO_8_GPIO8);
hal_gpio_set_direction(HAL_GPIO_8, HAL_GPIO_DIRECTION_OUTPUT);
hal_gpio_set_output(HAL_GPIO_8, HAL_GPIO_DATA_LOW);
hal_pinmux_set_function(HAL_GPIO_9, HAL_GPIO_9_GPIO9);
hal_gpio_set_direction(HAL_GPIO_9, HAL_GPIO_DIRECTION_OUTPUT);
hal_gpio_set_output(HAL_GPIO_9, HAL_GPIO_DATA_LOW);
}

void wifi_on(void){
	main_printf("wifi_on\r\n");
	setWifiOn(TRUE);
	wifi_rf_on();
	hal_pinmux_set_function((hal_gpio_pin_t)BSP_WIFI_32K_PIN, (uint8_t)BSP_WIFI_32K_PIN_M_CLKO);
	hal_gpio_set_clockout(HAL_GPIO_CLOCK_4, HAL_GPIO_CLOCK_MODE_32K);
	hal_gpio_set_output((hal_gpio_pin_t)BSP_WIFI_ENABLE_PIN, HAL_GPIO_DATA_HIGH); //OUTPUT HIGH
	hal_gpt_delay_ms(10);
	wifi_host_config_set_wifi_start();	
//	wifi_host_config_set_wifi_sleep(0);
}
void wifi_off(void){
	main_printf("wifi_off\r\n");
	setWifiOn(FALSE);
	wifi_rf_off();
	wifi_host_config_set_wifi_stop();
	hal_pinmux_set_function((hal_gpio_pin_t)BSP_WIFI_32K_PIN, 0);   // BSP_WIFI_32K_PIN_M_GPIO
	hal_gpio_set_output((hal_gpio_pin_t)BSP_WIFI_ENABLE_PIN, HAL_GPIO_DATA_LOW); 
//	wifi_host_config_set_wifi_sleep(1);
}

void wifi_start_scan(void){
	//wifi_stop_scan();
	setScan(TRUE);
	main_printf("wifi_start_scan\r\n");
	memset(static_aplist, 0, sizeof(static_aplist));
	wifi_host_connection_register_event_handler(WIFI_EVENT_IOT_SCAN_COMPLETE, (wifi_event_handler_t) scan_event_handler_sample);
	wifi_host_connection_scan_init(static_aplist, sizeof(static_aplist) / sizeof(static_aplist[0]));
	wifi_host_connection_start_scan(NULL, 0, NULL, 0, 0);


}
void wifi_stop_scan(void){
	setScan(FALSE);
	main_printf("wifi_stop_scan\r\n");
	wifi_host_connection_stop_scan();
	wifi_host_connection_scan_deinit();
	wifi_host_connection_unregister_event_handler(WIFI_EVENT_IOT_SCAN_COMPLETE, (wifi_event_handler_t) scan_event_handler_sample);
}
#endif
