# Copyright Statement:
#
# (C) 2005-2016  MediaTek Inc. All rights reserved.
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
# Without the prior written permission of MediaTek and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
# You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
# if you have agreed to and been bound by the applicable license agreement with
# MediaTek ("License Agreement") and been granted explicit permission to do so within
# the License Agreement ("Permitted User").  If you are not a Permitted User,
# please cease any access or use of MediaTek Software immediately.
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
# ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
# WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
 
#######################################################
# Project vairables assignment

SOURCE_DIR = ../../../../..
SDK_PATH    = $(abspath $(PWD)/$(SOURCE_DIR))

PWD= $(shell pwd)
FEATURE ?= feature.mk
DEBUG = 0
FLOAT_TYPE = hard
BUILD_DIR = $(PWD)/build

# Project name
PROJ_NAME = $(shell basename $(dir $(PWD)))
PROJ_PATH = $(abspath $(dir $(PWD)))/GCC
OUTPATH = $(PWD)/build

# LD Flags
LDFLAGS = $(ALLFLAGS) $(FPUFLAGS) --specs=nano.specs -lnosys -nostartfiles
LDFLAGS += -Wl,-Tflash.ld -Wl,--gc-sections
LDFLAGS += -Wl,-wrap=malloc -Wl,-wrap=calloc -Wl,-wrap=realloc -Wl,-wrap=free
ifeq ($(DEBUG_IO), 1)
LDFLAGS += --specs=rdimon.specs -lrdimon
CFLAGS += -DDEBUG_IO
endif
#######################################################
# Include Module

include $(FEATURE)
ifeq ($(MTK_SWLA_ENABLE), y)
CFLAGS += -DPRODUCT_VERSION_STR=\"$(PRODUCT_VERSION)\"
endif
# Gloabl Config
-include $(SOURCE_DIR)/.config
# IC Config
-include $(SOURCE_DIR)/config/chip/$(IC_CONFIG)/chip.mk
# Board Config
-include $(SOURCE_DIR)/config/board/$(BOARD_CONFIG)/board.mk
# EPT Config
-include $(SOURCE_DIR)/driver/board/mt25x3_hdk/ept/module.mk

# HAL driver files
include $(SOURCE_DIR)/driver/board/mt25x3_hdk/module.mk
include $(SOURCE_DIR)/driver/chip/mt2523/module.mk

# Keypad
-include $(SOURCE_DIR)/driver/board/mt25x3_hdk/keypad/module.mk

# Sources or Lib
# RTOS source files
include $(SOURCE_DIR)/kernel/rtos/FreeRTOS/module.mk

# kernel service files
include $(SOURCE_DIR)/kernel/service/module.mk

#ping lib
include $(SOURCE_DIR)/middleware/third_party/ping/module.mk

#include lwip
include $(SOURCE_DIR)/middleware/third_party/lwip/module.mk

#include iperf
include $(SOURCE_DIR)/middleware/MTK/iperf/module.mk

# HAL driver files
include $(SOURCE_DIR)/driver/chip/mt2523/module.mk

# NVDM files
include $(SOURCE_DIR)/middleware/MTK/nvdm/module.mk

# add ATCI files
include $(SOURCE_DIR)/middleware/MTK/atci/module.mk

ifeq ($(MTK_PORT_SERVICE_ENABLE), y)
# add PORT SERVICE files
include $(SOURCE_DIR)/middleware/MTK/port_service/module.mk
endif

ifeq ($(MTK_USB_DEMO_ENABLED), y)
# add USB CLASS files
include $(SOURCE_DIR)/middleware/MTK/usb/module.mk
endif

ifeq ($(MTK_WIFI_STUB_CONF_ENABLE),y)
# wifi stub config
include $(SOURCE_DIR)/middleware/MTK/wifi_host/wfcm/module.mk
include $(SOURCE_DIR)/middleware/MTK/wifi_host/xboot/module.mk
include $(SOURCE_DIR)/middleware/MTK/wifi_host/bwcs/module.mk      
ifeq ($(MTK_WIFI_STUB_CONF_SDIO_MSDC_ENABLE),y)
include $(SOURCE_DIR)/middleware/MTK/wifi_host/platform/freertos/hif/sdio/module.mk      
endif
endif

include $(SOURCE_DIR)/middleware/MTK/wifi_host/common/module.mk

include $(SOURCE_DIR)/middleware/MTK/wifi_host/platform/freertos/kal/module.mk

# mems driver files
include $(SOURCE_DIR)/driver/board/component/mems/module.mk
include $(SOURCE_DIR)/driver/board/mt25x3_hdk/mems_init/module.mk
# GNSS middleware files
include $(SOURCE_DIR)/middleware/MTK/gnss/module.mk

ifeq ($(MTK_SMART_BATTERY_ENABLE), y)
CFLAGS	+= -DMTK_SMART_BATTERY_ENABLE
endif

# battery_management files
include $(SOURCE_DIR)/middleware/MTK/battery_management/module.mk

#######################################################
# Main APP files

# Main APP files
APP_PATH        = $(patsubst $(SDK_PATH)/%,%,$(abspath $(dir $(PWD))))

$(info $(SDK_PATH))
$(info $(PROJ_PATH))
$(info $(APP_PATH))

APP_PATH_SRC = $(APP_PATH)/src

APP_FILES = $(APP_PATH_SRC)/main.c \
            $(APP_PATH_SRC)/regions_init.c \
            $(APP_PATH_SRC)/lwip_network.c \
            $(APP_PATH)/GCC/syscalls.c

ifeq ($(TRACKIMO_ENABLE), y)
APP_FILES += $(APP_PATH_SRC)/task_def.c	\
			$(APP_PATH_SRC)/battery_message.c
APP_PATH_SRC_TRACKIMO = $(APP_PATH_SRC)/trackimo/src
TRACKIMO_FILES = $(APP_PATH_SRC_TRACKIMO)/DriverInterface.c \
            $(APP_PATH_SRC_TRACKIMO)/WatchTask.c \
            $(APP_PATH_SRC_TRACKIMO)/MessageSystem.c \
            $(APP_PATH_SRC_TRACKIMO)/My4G.c \
            $(APP_PATH_SRC_TRACKIMO)/My4GQueue.c\
            $(APP_PATH_SRC_TRACKIMO)/MyBLE.c \
            $(APP_PATH_SRC_TRACKIMO)/MyBuzzer.c \
            $(APP_PATH_SRC_TRACKIMO)/MyCommon.c \
            $(APP_PATH_SRC_TRACKIMO)/MyGPS.c \
            $(APP_PATH_SRC_TRACKIMO)/MygSensor.c \
            $(APP_PATH_SRC_TRACKIMO)/MyKey.c \
            $(APP_PATH_SRC_TRACKIMO)/MyLed.c \
            $(APP_PATH_SRC_TRACKIMO)/MyMemory.c \
            $(APP_PATH_SRC_TRACKIMO)/MyTP.c \
            $(APP_PATH_SRC_TRACKIMO)/MyViburator.c \
            $(APP_PATH_SRC_TRACKIMO)/MyWifi.c \
	       $(APP_PATH_SRC_TRACKIMO)/MyTimer.c \
		$(APP_PATH_SRC_TRACKIMO)/MyBuffer.c \
		$(APP_PATH_SRC_TRACKIMO)/MyTest.c \
		$(APP_PATH_SRC_TRACKIMO)/TrackimoFrame.c \
		$(APP_PATH_SRC_TRACKIMO)/Module4GTask.c \
		$(APP_PATH_SRC_TRACKIMO)/TrackerQueue.c \
		$(APP_PATH_SRC_TRACKIMO)/TrackerDecodeIn.c\
		$(APP_PATH_SRC_TRACKIMO)/TrackerEncodeOut.c\
		$(APP_PATH_SRC_TRACKIMO)/GpsDecode.c\
		$(APP_PATH_SRC_TRACKIMO)/MyFile.c\
		$(APP_PATH_SRC_TRACKIMO)/TrackerMain.c\
		$(APP_PATH_SRC_TRACKIMO)/MyClock.c \
		$(APP_PATH_SRC_TRACKIMO)/MyGPSEpo.c \
		$(APP_PATH_SRC_TRACKIMO)/epo.c \
		$(APP_PATH_SRC_TRACKIMO)/WatchDateTime.c \
		$(APP_PATH_SRC_TRACKIMO)/My4GHttp.c \
		$(APP_PATH_SRC_TRACKIMO)/MainTask.c
C_FILES += $(TRACKIMO_FILES)
CFLAGS += -I$(SOURCE_DIR)/project/mt2523_hdk/apps/$(PROJ_NAME)/src/trackimo/inc
endif			
include $(SOURCE_DIR)/middleware/third_party/fatfs/module.mk

ifeq ($(MTK_FOTA_ENABLE), y)
include $(SOURCE_DIR)/middleware/MTK/fota/module.mk
include $(SOURCE_DIR)/middleware/MTK/bluetooth/module.mk
include $(SOURCE_DIR)/middleware/MTK/bt_notify/module.mk
include $(SOURCE_DIR)/middleware/MTK/bt_callback_manager/module.mk
APP_PATH_SRC_FOTA = $(APP_PATH_SRC)/fota/src
FOTA_FILES = $(APP_PATH_SRC_FOTA)/fota_main.c \
				$(APP_PATH_SRC_FOTA)/bt_init.c \
				$(APP_PATH_SRC_FOTA)/bt_common.c \
              $(APP_PATH_SRC_FOTA)/ble_app_common.c
C_FILES += $(FOTA_FILES)
ifeq ($(MTK_FOTA_GNSS_ENABLE), y)			  
APP_PATH_SRC_FOTA_GNSS = $(APP_PATH_SRC)/fota/src/gnss_src
FOTA_FILES_GNSS = $(APP_PATH_SRC_FOTA_GNSS)/flashtool.c \
             $(APP_PATH_SRC_FOTA_GNSS)/brom_mt3301.c \
			 $(APP_PATH_SRC_FOTA_GNSS)/da_cmd.c \
			 $(APP_PATH_SRC_FOTA_GNSS)/brom_base.c \
             $(APP_PATH_SRC_FOTA_GNSS)/gnss_uart.c
			  
C_FILES += $(FOTA_FILES_GNSS)
endif
CFLAGS += -I$(SOURCE_DIR)/project/mt2523_hdk/apps/$(PROJ_NAME)/src/fota/inc/gnss_inc
CFLAGS += -I$(SOURCE_DIR)/project/mt2523_hdk/apps/$(PROJ_NAME)/src/fota/inc
CFLAGS += -I$(SOURCE_DIR)/project/mt2523_hdk/apps/$(PROJ_NAME)/src/fota/inc/fota_inc
endif			
APP_FILES += $(APP_PATH_SRC)/ept_gpio_var.c
APP_FILES += $(APP_PATH_SRC)/ept_eint_var.c
APP_FILES += $(APP_PATH_SRC)/sys_init.c
APP_FILES += $(APP_PATH_SRC)/system_mt2523.c

# add WIFI ATCI files
ifeq ($(MTK_WIFI_AT_COMMAND_ENABLE),y)
APP_FILES      += $(APP_PATH_SRC)/at_command_wifi.c
endif

C_FILES += $(APP_FILES)

# add startup file to build
S_FILES += $(APP_PATH)/GCC/startup_mt2523.s

C_OBJS = $(C_FILES:%.c=$(BUILD_DIR)/%.o)
CXX_OBJS        = $(CXX_FILES:%.cpp=$(BUILD_DIR)/%.o)
S_OBJS = $(S_FILES:%.s=$(BUILD_DIR)/%.o)

#######################################################
# Include path

CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc
CFLAGS += -I$(SOURCE_DIR)/driver/CMSIS/Include
CFLAGS += -I$(SOURCE_DIR)/driver/CMSIS/Device/MTK/mt2523/Include
CFLAGS += -I$(SOURCE_DIR)/driver/chip/mt2523/inc
CFLAGS += -I$(SOURCE_DIR)/driver/chip/inc
CFLAGS += -I$(SOURCE_DIR)/kernel/service/inc
CFLAGS += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/ept/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/sensor_subsys/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/fatfs/src
CFLAGS += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/backlight
#######################################################
# Rules

.PHONY: proj clean $(MODULE_PATH)

all: cleanlog proj
	@mkdir -p $(BUILD_DIR)
	@$(SIZE) $(OUTPATH)/$(PROJ_NAME).elf
	@$(SOURCE_DIR)/tools/scripts/build/copy_firmware.sh $(SOURCE_DIR) $(OUTPATH) $(IC_CONFIG) $(BOARD_CONFIG) $(PROJ_NAME).bin $(PWD)

MOD_EXTRA = BUILD_DIR=$(BUILD_DIR) OUTPATH=$(OUTPATH) PROJ_PATH=$(PROJ_PATH)

$(LIBS): $(MODULE_PATH)

$(MODULE_PATH):
	@+make -C $@ $(MOD_EXTRA) $($@_EXTRA)

proj: $(OUTPATH)/$(PROJ_NAME).elf

$(OUTPATH)/$(PROJ_NAME).elf: $(C_OBJS) $(CXX_OBJS) $(S_OBJS) $(LIBS)
	@echo Linking...
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).map" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).map"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).dis" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).dis"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).hex" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).hex"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).bin" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).bin"; fi
	@echo $(patsubst $(SDK_PATH)/%,$(SOURCE_DIR)/%, $(LDFLAGS) -Wl,--start-group $^ -Wl,--end-group -Wl,-Map=$(patsubst $(SDK_PATH)/%,$(SOURCE_DIR)/%,$(OUTPATH)/$(PROJ_NAME).map) -lm -o $@) > $(OUTPATH)/link_option.tmp
	@$(CC) @$(OUTPATH)/link_option.tmp 2>>$(ERR_LOG)
	@rm -f $(OUTPATH)/link_option.tmp
	@$(OBJDUMP) -D $(OUTPATH)/$(PROJ_NAME).elf > $(OUTPATH)/$(PROJ_NAME).dis
	@$(OBJCOPY) -O ihex $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).hex
	@$(OBJCOPY) -O binary $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).bin
	@echo Done


include $(SOURCE_DIR)/.rule.mk

clean:
	rm -rf $(OUTPATH)


