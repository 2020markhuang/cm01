IC_CONFIG                             = mt2523
BOARD_CONFIG                          = mt2523_hdk

MTK_SENSOR_ACCELEROMETER_USE = MC3410
# NVDM
MTK_GNSS_ENABLE = y
MTK_NVDM_ENABLE                       = y
MTK_ATCI_ENABLE                       = y
MTK_WIFI_AT_COMMAND_ENABLE = n
MTK_BT_AT_COMMAND_ENABLE =  y
MTK_USB_DEMO_ENABLED = y
MTK_SYSTEM_AT_COMMAND_ENABLE = y
MTK_PORT_SERVICE_ENABLE = y
MTK_ATCI_VIA_PORT_SERVICE = y
TRACKIMO_ATCI_4GMOUDLE = y

# debug level: none, error, warning, and info
MTK_DEBUG_LEVEL                       = info

# WiFi
MTK_WIFI_CHIP_USE_MT5932              = y
MTK_WIFI_STUB_CONF_ENABLE             = y
#MTK_WIFI_STUB_CONF_SPI_ENABLE        = y
#MTK_WIFI_STUB_CONF_SPIM_ENABLE       = y
MTK_WIFI_STUB_CONF_SDIO_MSDC_ENABLE   = y
MTK_FOTA_ENABLE                       = y
MTK_FOTA_CM4_FS_ENABLE                = y
MTK_FOTA_FS_ENABLE = y
MTK_FOTA_GNSS_ENABLE = n
MTK_BT_ENABLE                       = y
MTK_BLE_ONLY_ENABLE                 = n
MTK_BT_SPP_ENABLE                   = y
MTK_SMART_BATTERY_ENABLE = y

TRACKIMO_ENABLE = y
