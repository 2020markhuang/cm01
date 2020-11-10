#include <string.h>
#include <stdio.h>
#include "MessageSystem.h"
#include "MyCommon.h"
#include "My4G.h"
#include "MyTimer.h"
#include "TrackimoFrame.h"
#include "MainTask.h"
#include "Module4GTask.h"
#include "TrackerQueue.h"
#include "TrackerMain.h"
#include "My4GQueue.h"
#include "MyLed.h"
#include "MyBuzzer.h"
#include "MyGPSEpo.h"
#ifdef SUPPORT_MY_4G
#define MOUDULE4G_TASK_EVENT_QUEUE_LENGTH 10
static const int task = TASK_4G;
static int static_iCmdATSend = 0;
static int static_iSmsIndex = 0;
static int send_at_command_fail_times = 0;
module_context_struct module_context;
module_context_struct *module_context_p = &module_context;
static char static_synchronous_buffer[TRACKER_COMM_QUEUE_BUFFER_SIZE];
static int static_iSynchronousLen = 0;
extern U32 s_module_current_head;
extern U32 s_module_at_current_head;

static const char * module4g_at_cmds[]={
	"+CMEE",
	"+PBREADY",
	"+SIM",
	"+KSUP",		
	"+CPIN",	
	"+COPS",    //5
	"+CSQ",
	"+CGCAT",
	"+CGDCONT",
	"+KCNXCFG",		
	"+KCNXUP",	//10
	"+KCGPADDR",	//11
	"+KUDPCFG",   
	"+KCNX_IND",
	"+KUDP_IND",
	"+KUDPSND",//15 
	"+KCNXTIMER", 
	"+CGREG",   
	"+CIMI",
	"+CGSN",
	"+KUDP_NOTIF", //20
	"CONNECT",	//21
	//"+CMGF",
	//"+CNMI",			
	//"+CEDRXS",		
	//"+SQNSCFG",
	//"+SQNSCFGEXT",
	//"+SQNSD",
	//"+SQNSSENDEXT",	
	//"+SQNDRDY",		
	//"+CMTI",
	//"+CMGR",
	//"+CMGD",
	//">",			
	"+KHTTPCFG",
	"+KHTTPCNX",
	"+KHTTPHEADER",
	"+KHTTPGET",
	"+KHTTPHEAD",
	"+KHTTPPOST",
	"+KHTTPCLOSE",
	"+KHTTPDEL",
	"+KHTTP_IND",
	"+KHTTPSCFG",
	"+KHTTPSCNX",
	"+KHTTPSHEADER",
	"+KHTTPSGET",
	"+KHTTPSHEAD",
	"+KHTTPSPOST",
	"+KHTTPSCLOSE",
	"+KHTTPSDEL",
	"+KHTTPS_IND",		
};

void idle_module_set_usim_func(void){
	//module4g_task_send_at_cmd(AT_4G_CMD_SET_USIM, NULL);
}

void idle_module_set_cpin_func(void){
	module4g_task_send_at_cmd(AT_4G_CMD_SET_CPIN, "?");
}

void idle_module_set_csq_func(void){
	module4g_task_send_at_cmd(AT_4G_CMD_SET_CSQ, NULL);
}

void idle_module_set_kcnxcfg_func(void){
	module4g_task_send_at_cmd(AT_4G_CMD_KCNXCFG,"=1,\"GPRS\",\"szsjmc.gd\"");
}

void idle_module_set_kcnxtimer_func(void){
	module4g_task_send_at_cmd(AT_4G_CMD_KCNXTIMER,"=1,60,2,70");
}

void idle_module_set_edrx_func(void){
	//module4g_task_send_at_cmd(AT_4G_CMD_SET_CEDRXS, "=%d", 0);
}

void idle_module_set_apn_func(void){
	//module4g_task_send_at_cmd(AT_4G_CMD_SET_CGDCONT, NULL);
}

void idle_module_set_cfun_func(void){
	//module4g_task_send_at_cmd(AT_4G_CMD_CFUN, "=%d", MODULE4G_FUN_FULL);
}

void idle_module_set_cimi_func(void){
	module4g_task_send_at_cmd(AT_4G_CMD_SET_CIMI, NULL);
}

void idle_module_set_imei_func(void){
	module4g_task_send_at_cmd(AT_4G_CMD_CGSN, NULL);
}

void idle_module_set_sms_config_func(void){
	//module4g_task_send_at_cmd(AT_4G_CMD_CMGF, "=%d", MODULE4G_CMGF_TEXT);
}

void idle_module_set_sms_read_noti_func(void){
	//module4g_task_send_at_cmd(AT_4G_CMD_CNMI, "=%d,%d", 2,1);
}

void idle_module_set_sms_read_func(void)
{ 
	if(static_iSmsIndex<1){
	return;
	}
	//module4g_task_send_at_cmd(AT_4G_CMD_CMGR, "=%d", static_iSmsIndex);
}

static void module4g_socket_config(int connID, int cid){
	module4g_task_send_at_cmd(AT_4G_CMD_KUDPCFG, "=%d,%d", connID, cid);
}

static void module4g_socket_config_ext(int connID, int srModel, int recvDataModel, int keepAlive, int listenAutoRsp, int sendDataModel){
	//module4g_task_send_at_cmd(AT_4G_CMD_SQNSCFGEXT, "=%d,%d,%d,%d,%d,%d,0,0", connID, srModel, recvDataModel, keepAlive, listenAutoRsp, sendDataModel);
}

static void module4g_socket_dial(int connID, int txProt, int rProt, char * ip, int closureType, int connMode, int acceptAnyRemote){
	//module4g_task_send_at_cmd(AT_4G_CMD_SQNSD, "=%d,%d,%d,\"%s\",%d,%d,%d", connID, txProt, rProt, ip, closureType, connMode, acceptAnyRemote);
}

void lte_send_data_set(void)
{

   if(module_context_p->module_soc_info.is_data_account_ok == FALSE)
	{
		module4g_task_send_at_cmd(AT_4G_CMD_KCNXUP,"=1");
		module4g_task_send_at_cmd(AT_4G_CMD_KCGPADDR,"=1");
		module4g_socket_config(1, 0);
	}
   else
	{
		 module_send_data_callback_func();
	}
}

void idle_lte_socket_set_callback_func(void){
	module4g_printf("idle_lte_socket_set_callback_func\r\n");
	lte_send_data_set();
}
	
void read_sms_queue(void){
	static_iSmsIndex = SMS_POP();
	module4g_printf("read_sms_queue module4g_printf=%d\r\n",static_iSmsIndex);
	idle_module_set_sms_read_func();
}

void at_command_lte_write_sms_del(int index)
{
	module4g_printf("at_command_lte_write_sms_del=%d\r\n",index );
	if(index<1){
	return;
	}
	module_context_p->at_send_enable=1;
 	//module4g_task_send_at_cmd(AT_4G_CMD_CMGD, "=%d,%d", index,3);
}

void at_command_lte_write_sms_del_all(void)
{
	module4g_printf("at_command_lte_write_sms_del_all\r\n");
	//module4g_task_send_at_cmd(AT_4G_CMD_CMGD, "=%d,%d", 1,4);		
}

void setLastATCmdSend(moudule_at_struct_msg * msg){
	if(strstr(msg->body_msg,"setadminband")!=NULL)
	{
		//static_iCmdATSend = AT_4G_CMD_SET_BAND;
	}
	else if(strstr(msg->body_msg,"CPIN")!=NULL)
	{
		static_iCmdATSend = AT_4G_CMD_SET_CPIN;
	}
	else if(strstr(msg->body_msg,"CSQ")!=NULL)
	{
		static_iCmdATSend = AT_4G_CMD_SET_CSQ;
	}
	else if(strstr(msg->body_msg,"KCNXCFG")!=NULL)
	{
		static_iCmdATSend = AT_4G_CMD_KCNXCFG;
	}
	else if(strstr(msg->body_msg,"KCNXUP")!=NULL)
	{
		static_iCmdATSend = AT_4G_CMD_KCNXUP;
	}
	else if(strstr(msg->body_msg,"KCGPADDR")!=NULL)
	{
		static_iCmdATSend = AT_4G_CMD_KCGPADDR;
	}
	else if(strstr(msg->body_msg,"KCNXTIMER")!=NULL)
	{
		static_iCmdATSend = AT_4G_CMD_KCNXTIMER;
	}
	else if(strstr(msg->body_msg,"CGREG")!=NULL)
	{
		static_iCmdATSend = AT_4G_CMD_SET_CGREG;
	}
	else if(strstr(msg->body_msg,"KHTTPSHEADER")!=NULL)
	{
		static_iCmdATSend = AT_4G_CMD_KHTTPHEADER;
	}
	else if(strstr(msg->body_msg,"CIMI")!=NULL)
	{
		static_iCmdATSend = AT_4G_CMD_SET_CIMI;
	}
	else if(strstr(msg->body_msg,"AT+CGSN")!=NULL)
	{
		static_iCmdATSend = AT_4G_CMD_CGSN;
	}
	else if(strstr(msg->body_msg,"AT+KUDPCFG")!=NULL)
	{
		static_iCmdATSend = AT_4G_CMD_KUDPCFG;
	}
	else if(strstr(msg->body_msg,"AT+CFUN")!=NULL)
	{
		//static_iCmdATSend = AT_4G_CMD_CFUN;
	}
	else if(strstr(msg->body_msg,"AT+CMGD")!=NULL)
	{
		//static_iCmdATSend = AT_4G_CMD_CMGD;
	}
	else if(strstr(msg->body_msg,"AT+CMGF")!=NULL)
	{
		//static_iCmdATSend = AT_4G_CMD_CMGF;
	}
	else if(strstr(msg->body_msg,"AT+CNMI")!=NULL)
	{
		//static_iCmdATSend = AT_4G_CMD_CNMI;
	}
	else if(strstr(msg->body_msg,"AT+CNMI")!=NULL)
	{
		//static_iCmdATSend = AT_4G_CMD_CNMI;
	}
	else if(strstr(msg->body_msg,"AT+SQNSCFG")!=NULL)
	{
		//static_iCmdATSend = AT_4G_CMD_SQNSCFG;
	}
	else if(strstr(msg->body_msg,"AT+SQNSCFGEXT")!=NULL)
	{
		//static_iCmdATSend = AT_4G_CMD_SQNSCFGEXT;
	}
	else if(strstr(msg->body_msg,"AT+SQNSD")!=NULL)
	{
		//static_iCmdATSend = AT_4G_CMD_SQNSD;
	}
	else if(strstr(msg->body_msg,"AT+SQNSSENDEXT")!=NULL)
	{
		//static_iCmdATSend = AT_4G_CMD_SQNSSENDEXT;
	}
	else if(strstr(msg->body_msg,"AT+CMGR")!=NULL)
	{
		//static_iCmdATSend = AT_4G_CMD_CMGR;
	}
	else if(strstr(msg->body_msg,"setusim")!=NULL)
	{
		//static_iCmdATSend = AT_4G_CMD_SET_USIM;
	}
	else 
	{
		return 0;
	}
}

int getLastATCmdSend(void){
	module4g_printf("getLastATCmdSend=%d\r\n",static_iCmdATSend);
	return static_iCmdATSend;
}

void module_check_enable_to_send(void){
	//module4g_printf("--module4g_receive_sms_handler--\r\n");
	module4g_printf("module_check_enable_to_send[at_send_enable]=%d\r\n",module_context_p->at_send_enable);
	if((module_context_p->at_send_enable==1)&&(module_at_message_buffer_count() != 0)){
		send_at_command_fail_times = 0;
		moudule_at_struct_msg * pTrackimoMsg = NULL;
		pTrackimoMsg = (moudule_at_struct_msg *)myMalloc(sizeof(moudule_at_struct_msg));
		memset(pTrackimoMsg,0,sizeof(moudule_at_struct_msg));
		pTrackimoMsg = (moudule_at_struct_msg *)module_at_message_buffer_pop();
		if(!pTrackimoMsg)return;
		setATResult(0);
		setLastATCmdSend(pTrackimoMsg);
		if(module_4g_send_uart_data((char *)pTrackimoMsg->body_msg, pTrackimoMsg->msgLen) == TRUE){
		module_context_p->at_send_enable=0;
		s_module_at_current_head++;
		}
	}
	else if((module_message_buffer_count()>0)&&(module_context_p->at_send_enable==1)){
		module_data_to_send();
	}
	else if((module_message_buffer_count()>0)&&(module_context_p->at_send_enable==0)){
		module4g_printf("**at_send_enable=1**\r\n");
	   //	module_context_p->at_send_enable=1;
	}
	else if((isSMSFIFOEmpty()==FALSE)&&(module_context_p->at_send_enable==1)){
		read_sms_queue();	
	}
	stopMyTimer(TIMER_ID_READ_SMS);
	startMyTimer(TIMER_ID_READ_SMS, 2*1000, module_check_enable_to_send);
}

void module4g_task_post_message(int msg, uint32_t wPara, int lpPara){
	MyPostMessage(task, task, msg, wPara, lpPara);
}

bool module4g_task_send_at_cmd(int cmd, char* strFormat, ...){
	moudule_at_struct_msg * pMsg = NULL;
	char * p = NULL;
	va_list args;
	if(cmd < 0 || cmd >= AT_4G_CMD_MAX)return FALSE;
	pMsg = (moudule_at_struct_msg *)myMalloc(sizeof(moudule_at_struct_msg));
	memset(pMsg,0,sizeof(moudule_at_struct_msg));
	if(!pMsg)return;
	p = (char *)pMsg->body_msg;
	sprintf(p, "AT%s", module4g_at_cmds[cmd]);
	p += strlen(p);
	if(strFormat){
		va_start (args, strFormat);
		vsprintf (p, strFormat, args);
		va_end (args);
	}
	strcat(p, "\r");
	pMsg->msgLen = strlen((char *)pMsg->body_msg);
	printf("module4g_task_send_at_cmd:cmd=%s,len=%d\r\n", pMsg->body_msg,pMsg->msgLen);
	module_at_message_buffer_push(pMsg);
	myFree(pMsg);
	return TRUE;
}

int parseRecevieCmd(char * pCmd){
	int i;
	char ** ppStandCmd = (char **)module4g_at_cmds;
	if(!pCmd)return 0;
	for(i = 0; i < AT_4G_CMD_MAX; i++, ppStandCmd++){
		if(strncmp(pCmd, *ppStandCmd, strlen(*ppStandCmd)) == 0){
			return i;
		}
	}
	return 0;
}

static char * module4g_process_lines(char * pString){
	char *pContent = NULL;
	char * pRet = NULL;
	if(!pString)return NULL;
	do{
		pContent = pString;
		pString = myReadLine(pContent, pString);
		if(strcmp(pContent, "OK") == 0){
			setATResult(1);
		}else if(strcmp(pContent, "ERROR") == 0){
			setATResult(2);
		}else if (*pContent){
			pRet = pContent;
		}
	}while(*pString);
	return pRet;
}

static void onATCReg(char * pValidData){
	module4g_printf("[LTE]iCodepValidData=%s\r\n",pValidData);
	int iCode = geConnectState(pValidData);
	module4g_printf("[LTE]iCode=%d\r\n",iCode);
	module_context_p->at_send_enable=1;	
	if((iCode == 1)||(iCode == 5))
	{
		module4g_printf("[LTE]Network status REGISTERED\r\n");
		g_trk_var.g_trk_status.work_mode=TRACKER_MODE_SWITCH_4G;
		setIs4GNetOK(TRUE);
	}
	else
	{
		module4g_printf("[LTE]Network status NO REGISTERED\r\n");
		setIs4GNetOK(FALSE);
	}	
}

static void onkcnx_ind(char * pValidData){
	int sessionid = getATRetCode(pValidData);
	int iCode = geConnectState(pValidData);
	module_context_p->at_send_enable=1;
	module4g_printf("sessionid = %d,icode = %d\r\n",sessionid,iCode);
	module_context_p->module_soc_info.connid = iCode;
	if(iCode == 1)
	{
		module4g_printf("[LTE]soc status connect\r\n");
		module_context_p->module_soc_info.connid = sessionid;
		module_context_p->module_soc_info.is_data_account_ok=TRUE;
	}
	else
	{
		module4g_printf("[LTE]soc status NO connect\r\n");
		module_context_p->module_soc_info.is_data_account_ok=FALSE;
	}	
}

static int parseATCmd(char * pCmd){
	int iCmd = 0;
//	if(!pCmd)return -1;	
	iCmd = parseRecevieCmd(pCmd);
	if(iCmd == 0)iCmd = getLastATCmdSend();
	return iCmd;
}

static void onNewSms(char * pValidData){
	U32 index = getATRetCode(pValidData + 5);
	module4g_printf("onNewSms:%d\r\n", index);
	SMS_Push(index);
}

static void onCMGR(char * pValidData){
	if(!pValidData)return;
	pdu_data_struct *pdu_data = NULL;
	pdu_data = (pdu_data_struct *)myMalloc(sizeof(pdu_data_struct));
	if(!pdu_data)return;
	pdu_data->pdu_length = strlen(pValidData);
	memcpy(pdu_data->pdu, pValidData, pdu_data->pdu_length);
	printf("onCMGR:%s,%d\r\n", pdu_data->pdu,pdu_data->pdu_length);
	main_task_post_message(MAIN_MSG_4G_EVENT, MODULE_4G_EVENT_SMS_PARSE,(int)pdu_data);
	myFree(pdu_data);
	//INCOMING_MSG_Q_PUSH(pValidData,len);
}

static void module4g_on_sysstart(void){
	module_at_message_buffer_clear();
	SMS_cleardata();
	module_context_p->module_soc_info.is_data_account_ok = FALSE; 
	module_context_p->sms_default_set.index = 0;
	module_context_p->sim_insert_flag = 0;
	module_context_p->module_soc_info.connid=1;
}

static void module4g_process_receive_data(moudule_at_struct_msg * pTrackimoMsg){
	int iCmd = 0;
	int data_account = 0;
	char * pValidData = NULL;
	char * pCmd = NULL;
	if(!pTrackimoMsg)return;
	if(strcmp((char *)pTrackimoMsg->body_msg, "\r\n") == 0)return;
	module4g_printf("parseRecevieCmd1111:%s\r\n", (char *)pTrackimoMsg->body_msg);
	module_read_uart_buffer_clear();
	pCmd = module4g_process_lines((char *)pTrackimoMsg->body_msg);
	iCmd = parseATCmd(pCmd);
	if(iCmd >= 0){
	pValidData = getATValidContent(pCmd, module4g_at_cmds[iCmd]);
	}
	module4g_printf("icmd:%d,atresult:%d,validdata:%s\r\n", iCmd, getATResult(), pValidData);
	switch(iCmd){
		case AT_4G_CMD_GET_PBREADY:
			module4g_on_sysstart();
			//idle_module_set_cfun_func();
			module_context_p->at_send_enable=1;
			//module4g_task_send_at_cmd(AT_4G_CMD_SET_CGREG,"=0");
			module4g_task_send_at_cmd(AT_4G_CMD_SET_CPIN, "?");
			module4g_task_send_at_cmd(AT_4G_CMD_SET_CSQ, NULL);
			module4g_task_send_at_cmd(AT_4G_CMD_KCNXCFG,"=1,\"GPRS\",\"m2m.tele2.com\"");
			module4g_task_send_at_cmd(AT_4G_CMD_SET_CGREG,"?");
			if(module_context_p->module_soc_info.is_data_account_ok == FALSE){
			module4g_task_send_at_cmd(AT_4G_CMD_KCNXUP,"=1");
			module4g_task_send_at_cmd(AT_4G_CMD_KCGPADDR,"=1");
			module4g_socket_config(1, 0);
			}
			//module4g_task_send_at_cmd(AT_4G_CMD_KCNXTIMER,"=1,60,2,70");
			//module4g_task_send_at_cmd(AT_4G_CMD_SET_CGREG,"=?");
			//idle_module_set_cimi_func();
			//idle_module_set_imei_func();
			//module4g_task_post_message(MODULE4G_MSG_STARTUP, 0, 0);
			main_task_post_message(MAIN_MSG_4G_EVENT, MODULE_4G_EVENT_NEW_DATA_INCOMING, 0);
			break;
		case AT_4G_CMD_SET_CPIN:
			if(getATResult() == 1)
			{	
				module_context_p->at_send_enable=1;
				if(module_context_p->sim_insert_flag == 0){
				idle_module_set_cimi_func();
				idle_module_set_imei_func();
				at_command_lte_write_sms_del_all();
				}
			}
			else if(getATResult() == 2)
			{
				module_4g_reset();
			}
			break;
		case AT_4G_CMD_SET_CSQ:
			if(getATResult() == 1)
			{
				module_context_p->at_send_enable=1;		
			}
			else if(getATResult() == 2)
			{
				//module_4g_reset();
			}
			break;	
		case AT_4G_CMD_KCNXCFG:
			if(getATResult() == 1)
			{
				module_context_p->at_send_enable=1;
			}
			else if(getATResult() == 2)
			{
				//module_4g_reset();
			}
			break;
		case AT_4G_CMD_KCNXUP:
			if(getATResult() == 1)
			{
			module_context_p->at_send_enable=1;
			}
			else if(getATResult() == 2)
			{
			//module_4g_reset();
			}
			break;
		case AT_4G_CMD_KCGPADDR:
			if(getATResult() == 1)
			{
			module_context_p->at_send_enable=1;
			}
			else if(getATResult() == 2)
			{
			module_4g_reset();
			}
			break;	
		case AT_4G_CMD_KCNXTIMER:
			if(getATResult() == 1)
			{
				module_context_p->at_send_enable=1;
			}
			else if(getATResult() == 2)
			{
				module_4g_reset();
			}
			break;
		case AT_4G_CMD_SET_CGREG:
			onATCReg(pValidData);
			if(is4GNetOK())
			{
				#ifdef SUPPORT_MY_LED
				green_led_on();
				#endif
				#ifdef SUPPORT_MY_BUZZER
				buzzer_power_on();
				#endif
				if(module_context_p->sms_default_set.index == 0){
				idle_module_set_sms_config_func();
				idle_module_set_sms_read_noti_func();
				module_context_p->sms_default_set.index = 1;
				}
			}
			else if(getATResult() == 2)
			{
				module_4g_reset();
			}
			break;	
		case AT_4G_CMD_SET_CIMI:
			//    此处写个NV保存IMSI号
			module4g_printf("[LTE]cmdDataIMSI:%s\r\n",pValidData);
			//pValidData = deleteQuotation(pValidData);
			if(strlen(pValidData))
			{
				strcpy(getDeviceImsi(), pValidData);
			}
			idle_lte_sim_status_callback_func(getDeviceImsi(),strlen(getDeviceImsi()));
			if(getATResult() == 1)
			{	
				module_context_p->at_send_enable=1;
				module_context_p->sim_insert_flag = 1;
			}
			else if(getATResult() == 2)
			{
				module_context_p->sim_insert_flag = 0;
				module_4g_off();
			}
			break;
		case AT_4G_CMD_CGSN:
			module4g_printf("[LTE]cmdDataIMEI:%s\r\n",pValidData);
			pValidData = deleteQuotation(pValidData);
			if(strlen(pValidData))
			{
				strcpy(getDeviceImei(), pValidData);
			}
			idle_lte_set_imei_callback_func(getDeviceImei(),strlen(getDeviceImei()));
			if(getATResult() == 1)
			{	
				module_context_p->at_send_enable=1;
			}
			else if(getATResult() == 2)
			{
				//module_4g_reset();
			}
			break;
		case AT_4G_CMD_KUDPCFG:
			if(getATResult() == 1)
			{
			module_context_p->at_send_enable=1;
			}
			else if(getATResult() == 2)
			{
			//module_4g_reset();
			}
			break;	
		case AT_4G_CMD_KCNXIND:
			if(getATResult() == 1)
			{
			module_context_p->at_send_enable=1;
			}
			else if(getATResult() == 2)
			{
			//module_4g_reset();
			}
			break;	
		case AT_4G_CMD_KUDPIND:
			onkcnx_ind(pValidData);
			break;
		case AT_4G_CMD_UDP_NOTIF:
			onkcnx_ind(pValidData);
			break;
		case AT_4G_CMD_READY_TO_SEND:
			module_4g_send_trackimo_data_step2(); 
			break;	
		/**case AT_4G_CMD_CMGF:
			if(getATResult() == 1)
			{
			module_context_p->at_send_enable=1;
			}
			else if(getATResult() == 2)
			{
			module_4g_reset();
			}
			break;**/		
		/**case AT_4G_CMD_CNMI:
			if(getATResult() == 1)
			{
			module_context_p->at_send_enable=1;
			}
			else if(getATResult() == 2)
			{
			module_4g_reset();
			}
			break;**/	
		/**case AT_4G_CMD_SQNSCFG:
			if(getATResult() == 1)
			{
			module_context_p->at_send_enable=1;
			}
			else if(getATResult() == 2)
			{
			module_context_p->module_soc_info.is_data_account_ok=FALSE;
			module_4g_reset();
			}
			break;
		case AT_4G_CMD_SQNSCFGEXT:
			if(getATResult() == 1)
			{
			module_context_p->at_send_enable=1;
			}
			else if(getATResult() == 2)
			{
			module_context_p->module_soc_info.is_data_account_ok=FALSE;
			module_4g_reset();
			}
			break;
		case AT_4G_CMD_SQNSD:
			if(getATResult() == 1)
			{
				module_context_p->at_send_enable=1;
				module_context_p->module_soc_info.is_data_account_ok = TRUE;
				module_send_data_callback_func();
			}
			else if(getATResult() == 2)
			{
				module_context_p->module_soc_info.is_data_account_ok = FALSE;
				module_4g_reset();
			}
			else
		   	{
		   		module_context_p->module_soc_info.is_data_account_ok = FALSE;
		   	}   
			break;
		case AT_4G_CMD_SQNSSENDEXT:	
			if(getATResult() == 1)
			{
				module_context_p->at_send_enable=1;
			}
			else if(getATResult() == 2)
			{
				module_4g_reset();
			}
			break;
		case AT_4G_CMD_NEW_SMS:
			onNewSms(pValidData);
			break;
		case AT_4G_CMD_CMGR:
			if(getATResult() == 1)
			{
				onCMGR(pValidData);
				at_command_lte_write_sms_del(static_iSmsIndex);
			}
			else if(getATResult() == 2)
			{
				module_4g_reset();
			}
			break;
		case AT_4G_CMD_CMGD:
			if(getATResult() == 1){
				module_context_p->at_send_enable=1;
				module4g_printf("-----sms del------\r\n");
			}
			else if(getATResult() == 2)
			{
				module_4g_reset();
			}
			break;	
		case AT_4G_CMD_CARRIER:
			module_context_p->at_send_enable=1;
			if(module_context_p->module_soc_info.connid<7){
			module_context_p->module_soc_info.connid++;
			}
			module4g_socket_config(module_context_p->module_soc_info.connid, 1, 0, 0, 0, 50);
			module4g_socket_config_ext(module_context_p->module_soc_info.connid,0,0,0,0,0);
			module4g_socket_dial(module_context_p->module_soc_info.connid, 1, 9999, "54.200.49.10", 0, 0, 1);
			break;		
		case AT_4G_CMD_READY_TO_SEND:
			module_context_p->at_send_enable=1;
			module_4g_send_trackimo_data_step2(); 
			break;**/
	}
}

void module_task_init(void)
{
	module_context_p->module_state = MODULE_POWEROFF_STATE; 
	module_context_p->module_soc_info.mode=1;
	module_context_p->module_soc_info.connid=1;
	module_context_p->module_soc_info.is_data_account_ok= FALSE; 
	module_context_p->sms_default_set.index = 0;
	module_context_p->module_work_flag = 0; 
	module_context_p->sim_insert_flag = 0; 
	module_context_p->lte_network_resgister_flag = 0; 
	module_context_p->at_send_enable = 0;
}

static void module4g_task_process(MYMSG * pMsg){
	if(!pMsg)return;
	switch(pMsg->id){
		case MODULE4G_MSG_PARSE_DATA:
			{
				moudule_at_struct_msg * pTrackimoMsg = (moudule_at_struct_msg *)pMsg->lpParam;
				if(!pTrackimoMsg)break;
				module4g_process_receive_data(pTrackimoMsg);
			}
			break;
		case MODULE4G_MSG_SEND_DATA:
			{
				//tracker_struct_msg * pTrackimoMsg = (tracker_struct_msg *)pMsg->lpParam;
				
			}
			break;
		case MODULE4G_MSG_STARTUP:
			module4g_poweron_init_synchronous();
			if(gps_epo_is_need_update())module4g_download("39.105.191.161", 80, "/EPO_GPS_3_1.DAT");
			break;	
		case MODULE4G_MSG_DOWNLOAD_FINISH:
			gps_epo_refresh();
			break;
	}
}

void module4g_task(void *pvParameters) {
	module4g_printf("module4g_task_start\r\n");
 	MyMessageStandTask(task, MOUDULE4G_TASK_EVENT_QUEUE_LENGTH, module4g_task_process);
}

char * getSynchronousBuffer(void){
	return static_synchronous_buffer;
}
int getSynchronousLen(void){
	return static_iSynchronousLen;
}

int * getSynchronousLenPtr(void){
	return &static_iSynchronousLen;
}
char * module4g_task_send_synchronous_cmd(const char * pString){
	char * pBuffer = getSynchronousBuffer();
	take_4g_semaphore(pBuffer, getSynchronousLenPtr(), pString, TRUE);
	return module4g_process_lines(pBuffer);
}

char * module4g_task_get_synchronous_result(int cmd, char* strFormat, ...){
	char * pBuffer = getSynchronousBuffer();
	char * p = pBuffer;
	va_list args;

	if((cmd < 0) || (cmd >= AT_4G_CMD_MAX))return NULL;
	sprintf(p, "AT%s", module4g_at_cmds[cmd]);

	p += strlen(p);
	if(strFormat){
		va_start (args, strFormat);
		vsprintf (p, strFormat, args);
		va_end (args);
	}
	strcat(p, "\r");
	return module4g_task_send_synchronous_cmd(pBuffer);
}
#endif
