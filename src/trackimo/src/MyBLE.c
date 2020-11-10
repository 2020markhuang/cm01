#include <stddef.h>
#include <string.h>

#include "MyCommon.h"
#include "MyBLE.h"

#ifdef SUPPORT_MY_BLE
extern void bt_trigger_interrupt(uint32_t is_from_isr);

void prepare_for_fota(void){
#ifdef MTK_FOTA_ENABLE
    fota_download_manager_init();
    ble_app_common_init();
#endif	
}
void ble_driver_init(void){
}
void ble_on(void){
//	bt_create_task();
//	prepare_for_fota();
}
void ble_off(void){
	//fota should reset, so no need off
}


#endif
