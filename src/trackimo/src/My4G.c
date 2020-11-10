#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "MyLog.h"
#include "MyCommon.h"
#include "MainTask.h"
#include "Module4GTask.h"
#include "MyBuffer.h"
#include "My4G.h"
#include "My4GQueue.h"
#ifdef SUPPORT_MY_4G
static const  hal_uart_port_t port = HAL_UART_1;
#define VFIFO_SIZE (256)
#define SEND_THRESHOLD_SIZE (100)
#define RECEIVE_THRESHOLD_SIZE (100)
#define RECEIVE_ALERT_SIZE (50)
volatile int static_bTrackimoSendingLen = 0;
static char static_trackimo_data_sending[TRACKER_COMM_QUEUE_BUFFER_SIZE]={0};
static char static_uart_receiving[TRACKER_COMM_QUEUE_BUFFER_SIZE]={0};
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static U8 g_uart_send_buffer[VFIFO_SIZE];
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static U8 g_uart_receive_buffer[VFIFO_SIZE];
static int static_iUartBufWriteIndex = 0;
static volatile char * static_pSynchronousReceiveBuffer = NULL;
static volatile int * static_pSynchronousLen = NULL;
static volatile SemaphoreHandle_t static_semaphore = NULL;
uint8_t modle_sleep_handle;
extern module_context_struct *module_context_p;
extern U32 s_module_current_head;
extern U32 s_module_current_tail;

void delete_4g_semaphore(void){
	if(static_semaphore)vSemaphoreDelete(static_semaphore);
	static_semaphore = NULL;
}

void take_4g_semaphore(char * pReceive, int * pReceiveLen, char * pSend, BOOL bAutoDelete)
{
	if(static_semaphore){
		xSemaphoreTake(static_semaphore, portMAX_DELAY); //portMAX_DELAY
		return;
	}
	static_pSynchronousReceiveBuffer = pReceive;
	static_pSynchronousLen = pReceiveLen;
	static_semaphore = xSemaphoreCreateBinary();
 	if(pSend)module_4g_send_uart_data((char *)pSend, strlen(pSend));
	else{
		xSemaphoreTake(static_semaphore, portMAX_DELAY); //portMAX_DELAY
	}
	if(bAutoDelete)delete_4g_semaphore();
}
void set_4g_waitbuffer(char * pReceive, int * pReceiveLen){
	static_pSynchronousReceiveBuffer = pReceive;
	static_pSynchronousLen = pReceiveLen;
}

void give_4g_semaphore(void)
{
    BaseType_t xHigherPriorityTaskWoken;
    /* We have not woken a task at the start of the ISR*/
    xHigherPriorityTaskWoken = pdFALSE;

//	static_pSynchronousReceiveBuffer = NULL;
    if(static_semaphore != NULL){
		xSemaphoreGiveFromISR(static_semaphore, &xHigherPriorityTaskWoken);
    }
}

static BOOL isWaitFinish(void){
	char * p = static_uart_receiving;
	if(strstr(p, "\r\nOK\r\n"))return TRUE;
	if(strstr(p, "\r\nERROR\r\n"))return TRUE;
	if(strstr(p, "_ERROR:"))return TRUE;
	if(strstr(p, "\r\nCONNECT\r\n"))return TRUE;
	if(strstr(p, "_IND:"))return TRUE;
	return FALSE;
}

void module_4g_set_rf_select(U32 index){
	switch(index)
	{
		case MODULE_4G_RF0:
			hal_gpio_init(HAL_GPIO_24);
			hal_pinmux_set_function(HAL_GPIO_24, HAL_GPIO_24_GPIO24);
			hal_gpio_set_direction(HAL_GPIO_24, HAL_GPIO_DIRECTION_OUTPUT);
			hal_gpio_set_output(HAL_GPIO_24, HAL_GPIO_DATA_HIGH);
			hal_gpio_init(HAL_GPIO_25);
			hal_pinmux_set_function(HAL_GPIO_25, HAL_GPIO_25_GPIO25);
			hal_gpio_set_direction(HAL_GPIO_25, HAL_GPIO_DIRECTION_OUTPUT);
			hal_gpio_set_output(HAL_GPIO_25, HAL_GPIO_DATA_HIGH);
		break;
		case MODULE_4G_RF1:
		break;	
	}
}

void module_read_uart_buffer_clear(void)
{
	//module4g_printf("-----module_read_uart_buffer_clear------\r\n");
	memset(static_uart_receiving,0,sizeof(TRACKER_COMM_QUEUE_BUFFER_SIZE));
	static_iUartBufWriteIndex = 0;
}

static BOOL lte_read_uart_data_new_line(void){
	int read_len = 0;
	int iBufLen = sizeof(static_uart_receiving);
	char * pBuffer = NULL;
	read_len = hal_uart_get_available_receive_bytes(port);
	if(read_len <= 0)return FALSE;
	if(read_len >= iBufLen)return FALSE;
	if((read_len + static_iUartBufWriteIndex) >= iBufLen){
		static_iUartBufWriteIndex = 0;
	}
	//module4g_printf("lte_read_uart_data_new_line0000puff=%s,receiving=%s,read_len=%d,WriteIndex=%d\r\n",pBuffer,static_uart_receiving,read_len,static_iUartBufWriteIndex);
	pBuffer = static_uart_receiving + static_iUartBufWriteIndex;
	read_len = hal_uart_receive_dma(port, pBuffer, read_len);
	//module4g_printf("lte_read_uart_data_new_line1111puff=%s,receiving=%s,read_len=%d,WriteIndex=%d\r\n",pBuffer,static_uart_receiving,read_len,static_iUartBufWriteIndex);
	static_iUartBufWriteIndex += read_len;
	pBuffer += read_len;
	//module4g_printf("lte_read_uart_data_new_line2222puff=%s,receiving=%s,read_len=%d,WriteIndex=%d\r\n",pBuffer,static_uart_receiving,read_len,static_iUartBufWriteIndex);
	*pBuffer = 0;
	if((static_bTrackimoSendingLen > 0) && (!strcmp(pBuffer - 2, "> "))){
		strcat(pBuffer, "\r\n");
		static_iUartBufWriteIndex += 2;
		return TRUE;
		}
	return !strcmp(pBuffer - 2, "\r\n");
}

static void lte_io_uart_irq(hal_uart_callback_event_t event, void *parameter){
	if(isDownloading()){
	int read_len;
	read_len = hal_uart_get_available_receive_bytes(port);
	read_len = hal_uart_receive_dma(port, static_uart_receiving, read_len);
	cacheDownloadBuffer(static_uart_receiving, read_len);
	return;
	}
	if(event == HAL_UART_EVENT_READY_TO_READ){
	//module4g_printf("lte_io_uart_irqREAD\r\n");
	moudule_at_struct_msg * pTrackimoMsg = NULL;
	pTrackimoMsg = (moudule_at_struct_msg *)myMalloc(sizeof(moudule_at_struct_msg));
	memset(pTrackimoMsg,0,sizeof(moudule_at_struct_msg));
	if(!pTrackimoMsg)return;
	if(!lte_read_uart_data_new_line())return;
	if(static_iUartBufWriteIndex <= 2)return;
		//printf("RRRR1:%d,%d\r\n", static_semaphore,static_iUartBufWriteIndex);
		if(static_semaphore){
			if(isWaitFinish()){
			memcpy(static_pSynchronousReceiveBuffer, static_uart_receiving, static_iUartBufWriteIndex);
			*static_pSynchronousLen = static_iUartBufWriteIndex;
			static_iUartBufWriteIndex = 0;
			give_4g_semaphore();
			//	hal_gpt_delay_ms(5);
			}
		}else{
			pTrackimoMsg->msgLen = static_iUartBufWriteIndex;
			memcpy(pTrackimoMsg->body_msg, static_uart_receiving, pTrackimoMsg->msgLen);
			module4g_task_post_message(MODULE4G_MSG_PARSE_DATA, 0, (int)pTrackimoMsg);
			myFree(pTrackimoMsg);
		}
	}else{
	//module4g_printf("lte_io_uart_irqWRITE\r\n");
	}
}

static hal_uart_status_t lte_io_uart_init(void){
	hal_uart_status_t ret;
	hal_uart_config_t uart_config = {
		.baudrate = HAL_UART_BAUDRATE_115200,
		.word_length = HAL_UART_WORD_LENGTH_8,
		.stop_bit = HAL_UART_STOP_BIT_1,
		.parity = HAL_UART_PARITY_NONE
	};

	hal_uart_dma_config_t   dma_config = {
		.send_vfifo_buffer              = g_uart_send_buffer,
		.send_vfifo_buffer_size         = VFIFO_SIZE,
		.send_vfifo_threshold_size      = SEND_THRESHOLD_SIZE,
		.receive_vfifo_buffer           = g_uart_receive_buffer,
		.receive_vfifo_buffer_size      = VFIFO_SIZE,
		.receive_vfifo_threshold_size   = RECEIVE_THRESHOLD_SIZE,
		.receive_vfifo_alert_size       = RECEIVE_ALERT_SIZE
	};
	
	ret = hal_uart_init(port, &uart_config);
	if (HAL_UART_STATUS_OK != ret) return ret;
	ret = hal_uart_set_dma(port, &dma_config);
	ret = hal_uart_register_callback(port, lte_io_uart_irq, NULL);
	//	ret =  hal_uart_set_hardware_flowcontrol(port);  //cm01不需要硬件流控
	return ret;
}

static void module_4g_eint_handler(void *parameter){
	module4g_printf("module_4g_eint_handler\r\n");
	// Mask eint_number at here is optional as it had been masked in HAL EINT driver.
	// hal_eint_mask(HAL_GPIO_38);
	 // User's handler
	 // Please call hal_eint_unmask() to unmask eint_number to receive an EINT interrupt.
	 //hal_eint_unmask(HAL_GPIO_38);
	 //module_4g_exit_sleep();
}

static void module_4g_init_handler(void){
	module4g_printf("module_4g_init_handler\r\n");
	/**
	hal_eint_config_t eint1_config;
	hal_gpio_init(HAL_GPIO_38);
	hal_pinmux_set_function(HAL_GPIO_38, HAL_GPIO_38_EINT21);
	hal_eint_mask(HAL_GPIO_38);
	eint1_config.trigger_mode = HAL_EINT_EDGE_RISING;
	eint1_config.debounce_time =10;
	hal_eint_init(HAL_GPIO_38, &eint1_config);
	hal_eint_register_callback(HAL_GPIO_38, module_4g_eint_handler, NULL);
	hal_eint_unmask(HAL_GPIO_38);**/
}

hal_uart_status_t module_uart_init(void){
	module4g_printf("module_uart_init\r\n");
	modle_sleep_handle = hal_sleep_manager_set_sleep_handle("moduleTaskSleepLock");
	hal_uart_status_t ret;
  	hal_gpio_init(HAL_GPIO_4);
	hal_pinmux_set_function(HAL_GPIO_4, HAL_GPIO_4_U1RXD);
	hal_gpio_init(HAL_GPIO_5);
	hal_pinmux_set_function(HAL_GPIO_5, HAL_GPIO_5_U1TXD);
	//hal_gpio_init(HAL_GPIO_23);
	//hal_pinmux_set_function(HAL_GPIO_23, HAL_GPIO_23_U1CTS);
	//hal_gpio_init(HAL_GPIO_21);
	//hal_pinmux_set_function(HAL_GPIO_21, HAL_GPIO_21_U1RTS);
	hal_pinmux_set_function(HAL_GPIO_38, HAL_GPIO_38_GPIO38);  //wakeup pin
	hal_gpio_set_direction(HAL_GPIO_38, HAL_GPIO_DIRECTION_OUTPUT);
	hal_gpio_set_output(HAL_GPIO_38, HAL_GPIO_DATA_LOW);

	hal_pinmux_set_function(HAL_GPIO_39, HAL_GPIO_39_GPIO39);   //rst PIN default high level
	hal_gpio_set_direction(HAL_GPIO_39, HAL_GPIO_DIRECTION_OUTPUT);
	hal_gpio_set_output(HAL_GPIO_39, HAL_GPIO_DATA_HIGH);
	ret = lte_io_uart_init();
	return ret;
}

void module_uart_exit(void){
	module4g_printf("module_uart_exit\r\n");
	 hal_uart_deinit(port);
}

void module_4g_driver_init(void){
	module4g_printf("module_4g_driver_init\r\n");
	module_task_init();
	if(module_context_p->module_state==MODULE_POWEROFF_STATE)
	{
		hal_gpio_init(HAL_GPIO_4);
		hal_pinmux_set_function(HAL_GPIO_4, HAL_GPIO_4_U1RXD);
		hal_gpio_init(HAL_GPIO_5);
		hal_pinmux_set_function(HAL_GPIO_5, HAL_GPIO_5_U1TXD);
		module_4g_set_rf_select(0);
		module_4g_init_handler();
		//msg_pool_init();
		if(module_uart_init()==HAL_UART_STATUS_OK)
		{
			vTaskDelay(300);
			module_4g_on();
			module_4g_reset();
		}
		else
		{
            module4g_printf("module_4g_driver_initFALSE\r\n");
		}
	}
}

void module_4g_driver_deinit(void){
	module4g_printf("Track_module_deinit\r\n");
	if(module_context_p->module_state==MODULE_POWERON_STATE)
	{
	module_4g_off();
	module_uart_exit();
	module_context_p->module_state = MODULE_POWEROFF_STATE; 
	module_context_p->module_soc_info.is_data_account_ok = FALSE; 
	module_context_p->sms_default_set.index = 0;
	module_context_p->module_work_flag = 0; 
	module_context_p->sim_insert_flag = 0; 
	module_context_p->lte_network_resgister_flag = 0; 
	}
}

void module_4g_on(void){
	module4g_printf("module_4g_on\r\n");
	hal_sleep_manager_lock_sleep(modle_sleep_handle);
	static_bTrackimoSendingLen = 0;
	module_context_p->module_state=MODULE_POWERON_STATE;
	hal_gpio_init(HAL_GPIO_47);
	hal_pinmux_set_function(HAL_GPIO_47, HAL_GPIO_47_GPIO47);
	hal_gpio_set_direction(HAL_GPIO_47, HAL_GPIO_DIRECTION_OUTPUT);
	hal_gpio_set_output(HAL_GPIO_47, HAL_GPIO_DATA_HIGH);
}

void module_4g_off(void){
	module4g_printf("module_4g_off\r\n");
	if(hal_sleep_manager_get_lock_status() & (1 << modle_sleep_handle))
	{
	hal_sleep_manager_unlock_sleep(modle_sleep_handle);
	}
	module_context_p->module_state=MODULE_POWEROFF_STATE;
	hal_pinmux_set_function(HAL_GPIO_47, HAL_GPIO_47_GPIO47);
	hal_gpio_set_direction(HAL_GPIO_47, HAL_GPIO_DIRECTION_OUTPUT);
	hal_gpio_set_output(HAL_GPIO_47, HAL_GPIO_DATA_LOW);
	hal_gpio_deinit(HAL_GPIO_47);
}

void module_4g_reset(void){
	hal_pinmux_set_function(HAL_GPIO_39, HAL_GPIO_39_GPIO39);
	hal_gpio_set_direction(HAL_GPIO_39, HAL_GPIO_DIRECTION_OUTPUT);
	hal_gpio_set_output(HAL_GPIO_39, HAL_GPIO_DATA_LOW);
	vTaskDelay(10);
	hal_pinmux_set_function(HAL_GPIO_39, HAL_GPIO_39_GPIO39);
	hal_gpio_set_direction(HAL_GPIO_39, HAL_GPIO_DIRECTION_OUTPUT);
	hal_gpio_set_output(HAL_GPIO_39, HAL_GPIO_DATA_HIGH);
	//module_4g_off();
	//vTaskDelay(10);
	//module_4g_on();
}

void module_4g_entry_sleep(void){
	module4g_printf("module_4g_entry_sleep=%d,%d\r\n",hal_sleep_manager_is_sleep_locked(),hal_sleep_manager_get_lock_status() & (1 << modle_sleep_handle));
	if(hal_sleep_manager_get_lock_status() & (1 << modle_sleep_handle)) 
	{
		hal_sleep_manager_unlock_sleep(modle_sleep_handle);
		module4g_printf("module_4g_entry_sleep=%d,%d\r\n",hal_sleep_manager_is_sleep_locked(),hal_sleep_manager_get_lock_status() & (1 << modle_sleep_handle));
	}
	//hal_gpio_init(HAL_GPIO_21);
	//hal_pinmux_set_function(HAL_GPIO_21, HAL_GPIO_21_U1RTS);
	//hal_gpio_set_direction(HAL_GPIO_21, HAL_GPIO_DIRECTION_OUTPUT);
	//hal_gpio_set_output(HAL_GPIO_21, HAL_GPIO_DATA_HIGH);
}

void module_4g_exit_sleep(void){
	module4g_printf("module_4g_exit_sleep=%d,%d\r\n",hal_sleep_manager_is_sleep_locked(),hal_sleep_manager_get_lock_status() & (1 << modle_sleep_handle));
	hal_sleep_manager_lock_sleep(modle_sleep_handle);
	//hal_gpio_init(HAL_GPIO_21);
	//hal_pinmux_set_function(HAL_GPIO_21, HAL_GPIO_21_U1RTS);
	//hal_gpio_set_direction(HAL_GPIO_21, HAL_GPIO_DIRECTION_OUTPUT);
	//hal_gpio_set_output(HAL_GPIO_21, HAL_GPIO_DATA_LOW);
}

void module_data_to_send(void){
	if((module_context_p->sim_insert_flag==1)&&(is4GNetOK())&&(module_context_p->module_soc_info.is_data_account_ok == TRUE)){
	tracker_struct_msg* ptrModuleMsg= NULL;
	ptrModuleMsg=module_message_buffer_pop();
	memset(static_trackimo_data_sending,0,TRACKER_COMM_QUEUE_BUFFER_SIZE);
	memcpy(static_trackimo_data_sending, ptrModuleMsg->body_msg, ptrModuleMsg->msgLen);
	static_bTrackimoSendingLen=ptrModuleMsg->msgLen;
	idle_lte_socket_set_callback_func();
	}
}

void module_4g_send_trackimo_data_step2(void){
	char temp[TRACKER_COMM_QUEUE_BUFFER_SIZE];
	int length=0; 
	if(static_bTrackimoSendingLen <= 0)return FALSE;
	printf("module_4g_send_trackimo_data_step2, head=%d, tail=%d\r\n", s_module_current_head,s_module_current_tail);
	memset(temp,0,TRACKER_COMM_QUEUE_BUFFER_SIZE);
	sprintf(temp, "%s--EOF--Pattern--", static_trackimo_data_sending);
	length=strlen(temp);
	module4g_printf("length=%d\r\n",length);
	if(module_4g_send_uart_data(temp, length) == TRUE){
		s_module_current_head++;
	}
}

void module_send_data_callback_func(void){
	module4g_task_send_at_cmd(AT_4G_CMD_KUDPSEND, "=%d,\"54.200.49.10\",9999,%d", module_context_p->module_soc_info.connid,static_bTrackimoSendingLen);
}

BOOL module_4g_send_uart_data(char * pData, int iDataLen){
	int len = 0;
	if((!pData) || (iDataLen <= 0))return FALSE;
	module4g_printf("module_4g_send_uart_data:%s,%d\r\n", pData, iDataLen);
	len = hal_uart_send_dma(port,(U8 *)pData,iDataLen);
	if(static_semaphore){
//		printf("xxxxxxxxxx\r\n");
	xSemaphoreTake(static_semaphore, portMAX_DELAY); //portMAX_DELAY
	vSemaphoreDelete(static_semaphore);
	static_semaphore = NULL;
	}
	//myMemDump(pData, iDataLen);
	return len == iDataLen;
}
#endif
