/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* device.h includes */
#include "mt2523.h"

/* hal includes */
#include "hal.h"

#include "sys_init.h"
#include "task_def.h"
#include "wifi_host_api.h"


#include "main.h"
#include "maintask.h"

#ifdef MTK_SMART_BATTERY_ENABLE
#include "MyCommon.h"
#include "MyLed.h"
#include "MyLog.h"
#include "cust_battery_meter.h"
#include "hal.h"
/* battery management includes */
#include "battery_management.h"
#include "battery_message.h"
static BatteryEventStruct gBatteryEvent;
#endif

/*wifi host includes*/
#ifdef MTK_WIFI_CHIP_USE_MT5932
#include "wifi_host_api.h"
#include "wifi_host_private_api.h"
#ifdef MTK_WIFI_AT_COMMAND_ENABLE
#include "at_command_wifi.h"
#endif
#endif

#if defined(MTK_PORT_SERVICE_ENABLE)
#include "serial_port.h"
#endif

#ifdef MTK_USB_DEMO_ENABLED
#include "usb.h"
#endif

#include "lwip/tcpip.h"
#include "lwip_network.h"
//xushuyi add start 20191230
#include "TrackerMain.h"
//xushuyi add end 20191230


extern const char BSP_WIFI_ENABLE_PIN;
extern const char BSP_WIFI_32K_PIN;
extern const char BSP_WIFI_32K_PIN_M_GPIO;
extern const char BSP_WIFI_32K_PIN_M_CLKO;
extern const char BSP_WIFI_RESET_PIN;
static SemaphoreHandle_t gWifiInitDoneSemaphore = NULL;
uint8_t main_sleep_handle;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/*Define this macro indicates The host will send wifi configuration to wifi chip for wifi initialization,  
   Otherwise, wifi chip will do initialization by the default settings in wifi chip's program*/
#define WIFI_CONFIG_BY_HOST 


/* Private variables ---------------------------------------------------------*/
//uint8_t _cli_iperf_remote_client(void); 
//uint8_t _cli_iperf_remote_server(void);


/* Private functions ---------------------------------------------------------*/

/* Create the log control block for freertos module.
 * The initialization of the log is in the sys_init.c.
 * Please refer to the log dev guide under /doc folder for more details.
 */
#ifndef MTK_DEBUG_LEVEL_NONE
#ifdef MTK_WIFI_STUB_CONF_ENABLE
log_create_module(wfcm, PRINT_LEVEL_INFO);
#endif
LOG_CONTROL_BLOCK_DECLARE(common);
LOG_CONTROL_BLOCK_DECLARE(hal);
LOG_CONTROL_BLOCK_DECLARE(lwip);
#ifdef MTK_WIFI_STUB_CONF_ENABLE
LOG_CONTROL_BLOCK_DECLARE(wfcm);
#endif
#ifdef MTK_SMART_BATTERY_ENABLE
LOG_CONTROL_BLOCK_DECLARE(bmt);
#endif

log_control_block_t *syslog_control_blocks[] = {
    &LOG_CONTROL_BLOCK_SYMBOL(common),
    &LOG_CONTROL_BLOCK_SYMBOL(hal),
    &LOG_CONTROL_BLOCK_SYMBOL(lwip),    
#ifdef MTK_WIFI_STUB_CONF_ENABLE
    &LOG_CONTROL_BLOCK_SYMBOL(wfcm),
#endif
#ifdef MTK_SMART_BATTERY_ENABLE
    &LOG_CONTROL_BLOCK_SYMBOL(bmt),
#endif
    NULL
};
#endif

#ifdef MTK_SMART_BATTERY_ENABLE
/**
* @brief       This function is use to get the battery information.
* @return      None.
*/
static void get_battery_information(void)
{
	 gBatteryEvent.capacity = battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY);  // 0 ~ 100
	 gBatteryEvent.capacity_level = battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY_LEVEL);
	 gBatteryEvent.charger_current = battery_management_get_battery_property(BATTERY_PROPERTY_CHARGING_CURRENT);
	 gBatteryEvent.charger_status = battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST);
	 gBatteryEvent.charger_type = battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_TYPE);
	 gBatteryEvent.battery_temperature = battery_management_get_battery_property(BATTERY_PROPERTY_TEMPERATURE);
	 gBatteryEvent.battery_volt = battery_management_get_battery_property(BATTERY_PROPERTY_VOLTAGE);
	 //maintask_printf("Battery capacity = %d",  gBatteryEvent.capacity );
	 //maintask_printf("Battery capacity level = %d",  gBatteryEvent.capacity_level);
	 //maintask_printf("Charger status = %d\r\n", gBatteryEvent.charger_status);
}

/**
* @brief       This function is use to check battery status.
* @return      None.
*/
static void check_battery_status(void)
{
	int32_t capacity, battery_temperature,charger_exist,battery_volt;

	capacity = battery_management_get_battery_property(BATTERY_PROPERTY_CAPACITY);
	battery_temperature = battery_management_get_battery_property(BATTERY_PROPERTY_TEMPERATURE);
	charger_exist = battery_management_get_battery_property(BATTERY_PROPERTY_CHARGER_EXIST);
	battery_volt = battery_management_get_battery_property(BATTERY_PROPERTY_VOLTAGE);

	if((capacity == 0 ) && (capacity != BATTERY_INVALID_VALUE)) {
		/* Low battery shutdown */
		maintask_printf("Low battery shutdown\r\n");
		vTaskDelay(3 * 1000 / portTICK_RATE_MS);
		hal_sleep_manager_enter_power_off_mode();
	}

	/* High temperature protection. It depends on your application and the battery specifications */
	if((battery_temperature >= 60 ) && (battery_temperature != BATTERY_INVALID_VALUE)) {
		/* High temperature  shutdown */
		maintask_printf("High temperature shutdown\r\n");
		vTaskDelay(3 * 1000 / portTICK_RATE_MS);
		hal_sleep_manager_enter_power_off_mode();
	}
	
	//modify by viva 
	if((charger_exist == 1 ) && (charger_exist != BATTERY_INVALID_VALUE))
	 {
	   if((capacity == 100 ) && (capacity != BATTERY_INVALID_VALUE) 
	   &&(battery_volt>4300)&& (battery_volt != BATTERY_INVALID_VALUE)) {
		/* Battery full */
		maintask_printf("Battery full\r\n");
		#ifdef SUPPORT_MY_LED 
		led_battery_full();
		#endif
	   }
	   else
	   {
	  	 maintask_printf("chargering.......\r\n");
		 #ifdef SUPPORT_MY_LED
	  	 led_battery_ischarging();
		 #endif
	   }
	 }
	else
	 {
		 maintask_printf("no charger#####\r\n");
	 }

}

void updateBatteryEvent(BatteryEventStruct* battery)
{
	BatteryEventStruct* batStatus = battery;
	main_task_post_message(MSG_TYPE_BATTERY_EVENT, 0,(int)batStatus);
}

/**
* @brief       This function is battery management message handler.
* @param[in]   message: The message should be process.
* @return      None.
*/
static void battery_management_message_handler(battery_message_context_t *message)
{
    switch (message->event) {
        case BATTERY_MANAGEMENT_EVENT_BATTERY_UPDATE: {
		get_battery_information();
		check_battery_status();
		updateBatteryEvent(&gBatteryEvent);
        }
        break;
        default: {
        }
        break;
    }
}

/**
* @brief       Task main function
* @param[in]   pvParameters: Pointer that will be used as the parameter for the task being created.
* @return      None.
*/
static void battery_message_task(void *pvParameters)
{
	uint32_t handle;
	battery_message_context_t message;
	battery_message_allocate(&handle);
	while (1) {
	if (battery_message_receive(handle, &message)) {
	battery_management_message_handler(&message);
	}
	}
}

/**
 * @brief     This function is to create task for battery message.
 * @return    None.
 */
TaskHandle_t battery_message_create_task()
{
    TaskHandle_t task_handler;
    BaseType_t ret;
    ret = xTaskCreate((TaskFunction_t) battery_message_task,
        BMT_APP_TASK_NAME,
        BMT_APP_TASK_STACKSIZE/(( uint32_t )sizeof( StackType_t )),
        NULL,
        BMT_APP_TASK_PRIO,
        &task_handler );
    if (ret != pdPASS) {
        assert(0);
    }
    return task_handler;
}
#endif //MTK_SMART_BATTERY_ENABLE

#if defined(MTK_PORT_SERVICE_ENABLE)
static void syslog_port_service_init(void)
{
    serial_port_dev_t syslog_port;
    serial_port_setting_uart_t uart_setting;
    if (serial_port_config_read_dev_number("syslog", &syslog_port) != SERIAL_PORT_STATUS_OK) {
        syslog_port = SERIAL_PORT_DEV_USB_COM2;
        serial_port_config_write_dev_number("syslog", syslog_port);
        uart_setting.baudrate = HAL_UART_BAUDRATE_115200;
        serial_port_config_write_dev_setting(syslog_port, (serial_port_dev_setting_t *)&uart_setting);
    }
}
#endif


#if 0//def MTK_FOTA_ENABLE
#include "fota_gprot.h"
#include "ble_app_common.h"
log_create_module(fota_dl_m, PRINT_LEVEL_INFO);
void bl_print(int level, char *fmt, ...){
}
static void fota_prepare(void){
    fota_download_manager_init();
    ble_app_common_init();	
}
#endif

int main(void)
{ 
	/* Do system initialization, eg: hardware, nvdm. */
	system_init();
	main_sleep_handle = hal_sleep_manager_set_sleep_handle("mainsleephandle");
    hal_sleep_manager_lock_sleep(main_sleep_handle);
	/* Enable I,F bits */
	__enable_irq();
	__enable_fault_irq();
	#if defined(MTK_PORT_SERVICE_ENABLE)
	syslog_port_service_init();
	#endif
	#if defined (MTK_PORT_SERVICE_ENABLE)
	/* Reduce bt-spp log to make syslog over bt-spp work. */
	//log_config_print_level(bt_spp, PRINT_LEVEL_WARNING);
	// log_config_print_level(SPP_PORT, PRINT_LEVEL_WARNING);
	#endif
	/* system log initialization.
	* This is the simplest way to initialize system log, that just inputs three NULLs
	* as input arguments. User can use advanced feature of system log along with NVDM.
	* For more details, please refer to the log dev guide under /doc folder or projects
	* under project/mtxxxx_hdk/apps/.
	*/
	log_init(NULL, NULL, syslog_control_blocks);

	#if 0//def MTK_FOTA_ENABLE
	bt_create_task();
	fota_prepare();
	#endif
	
	#ifdef MTK_USB_DEMO_ENABLED
	usb_boot_init();
	#endif

	#ifdef MTK_SMART_BATTERY_ENABLE
	battery_message_create_task();
	#endif

	#ifdef TRACKIMO_ENABLE
	task_def_init();
	task_def_create();
	#endif
	/* Start the scheduler. */
	vTaskStartScheduler();
	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for (;;);
}

