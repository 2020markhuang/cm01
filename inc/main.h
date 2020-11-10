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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"


#ifdef MTK_SMART_BATTERY_ENABLE
#define BATTERY_MAX_LEVEL   (4)
#define MMI_BATT_LEVEL(cap)   (((cap) * BATTERY_MAX_LEVEL) / 100)
typedef struct _battery_event_struct
{
    int32_t capacity, charger_current, charger_type, battery_temperature, battery_volt, capacity_level;
    uint16_t isFull;
    uint16_t count;
    uint16_t charger_status;
    uint16_t old_charger_status;
}BatteryEventStruct;
#endif


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* User can use this section to tailor USARTx/UARTx instance used and the associated
   resources */
/* Definition for USARTx clock resources */
#define USARTx                           USART2
#define USARTx_CLK_ENABLE()              __USART2_CLK_ENABLE();
#define USARTx_RX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __USART2_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __USART2_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_2
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_TX_AF                     GPIO_AF7_USART2
#define USARTx_RX_PIN                    GPIO_PIN_3
#define USARTx_RX_GPIO_PORT              GPIOA
#define USARTx_RX_AF                     GPIO_AF7_USART2

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */



//====================== C start ==========================
#ifdef __cplusplus
extern "C" {
#endif

extern const int32_t __max_i32;
extern const int32_t __min_i32;
extern const int16_t __max_i16;
extern const int16_t __min_i16;

void fota_update_ind(bool isUpdating, bool reboot);

// for wdt
#define SYS_WDT_BT_TASK     0x01
#define SYS_WDT_SENSOR_TASK 0x02
#define SYS_WDT_FUSION_TASK 0x04
#define SYS_WDT_GPS_TASK    0x08
#define SYS_WDT_MMI_TASK    0x10
#define SYS_WDT_IDLE_TASK   0x20
#define SYS_WDT_SLEEP_IND   0x40
void sys_init_wdt(void);
void sys_feed_wdt(uint8_t task_flag);


extern uint32_t g_mmi_dbg_info;
extern uint32_t g_gps_dbg_info;
extern uint32_t g_ancs_dbg_info;

#define SYS_UPDATE_NONE  0x00
#define SYS_UPDATE_CLOCK 0x01
#define SYS_UPDATE_UNIT  0x02
#define SYS_UPDATE_TIME  0x04
#define SYS_UPDATE_LANG  0x08
#define SYS_UPDATE_USER  0x10


#ifdef __cplusplus
}
#endif
//====================== C end ==========================


#endif /* __MAIN_H */

/************************ (C) COPYRIGHT Mediatek *****END OF FILE****/
