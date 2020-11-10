/**
@brief ���ļ������˷��Ͷ���
$Id: TrackerQueue.h  2019-03-13  viva huang $
**/
#include "TrackerQueue.h"
#include "MyDefine.h"
#include "MyCommon.h"
#include "Module4GTask.h"
#include "My4G.h"
static tracker_struct_msg  module_message_buffer[TRACKER_MESSAGE_QUEUE_MAXNUM];
extern module_context_struct *module_context_p;

/** ��ʾĿǰ��ͷλ�� **/
 U32 s_module_current_head=0;

/** ��ʾĿǰ���һ���ڵ����һ���ڵ� **/
 U32 s_module_current_tail=0;


/** ��ʼ��λ�ô洢�� **/
void module_message_buffer_init(void)
{
	module_message_buffer_clear();
}

void module_Q_sending(void)
{
	printf("module_Q_sending\r\n");
	if(module_message_buffer_count()>0)
	{
		#ifdef SUPPORT_MY_4G
		if(module_context_p->at_send_enable=1){
		module_data_to_send();
		}
		#endif
	}    
}

/** ����Ϣ�洢���м���һ��Ԫ�� **/
void  module_message_buffer_push(tracker_struct_msg *pmessage)
{
	 if(s_module_current_tail == TRACKER_MESSAGE_QUEUE_MAXNUM)
	 {
	 	s_module_current_tail = 0;
	 }
	 memset(&(module_message_buffer[s_module_current_tail]), 0x00, sizeof(tracker_struct_msg));
	 memcpy(&(module_message_buffer[s_module_current_tail].body_msg),(pmessage->body_msg),pmessage->msgLen);  
	 module_message_buffer[s_module_current_tail].msgLen=pmessage->msgLen;	
	 s_module_current_tail++;
	 printf("module_data_message_buffer_push, head=%d, tail=%d\r\n", s_module_current_head,s_module_current_tail);
}

/** ����Ϣ�洢����ȡ��һ��Ԫ�� **/
tracker_struct_msg* module_message_buffer_pop(void)
{
	if(module_message_buffer_count()== 0)
	{
		return NULL;
	}
	printf("module_data_message_buffer_pop, head=%d, tail=%d\r\n", s_module_current_head,s_module_current_tail);
	if(s_module_current_head == TRACKER_MESSAGE_QUEUE_MAXNUM){
		s_module_current_head = 0;
	}  
	return &(module_message_buffer[s_module_current_head]);
}

/** ����Ϣ�洢����ȡ��һ��Ԫ�� **/
tracker_struct_msg* module_message_buffer_get(void)
{
    return &(module_message_buffer[s_module_current_head]);
}

/** �����Ϣ�洢�� **/
void module_message_buffer_clear(void)
{
    if((s_module_current_head==0)&&(s_module_current_tail==0))
          return;

    memset(&module_message_buffer, 0, sizeof(module_message_buffer));
	s_module_current_head = 0;
	s_module_current_tail = 0;
}

/** ��ȡ��Ϣ�洢����Ԫ�صĸ��� **/
U32 module_message_buffer_count(void)
{
	if(s_module_current_head == s_module_current_tail){
		return 0;
	}
	else if(s_module_current_head < s_module_current_tail){
		return s_module_current_tail - s_module_current_head;
	}
	else{
		return s_module_current_tail + TRACKER_MESSAGE_QUEUE_MAXNUM - s_module_current_head;
	}
}

            
/*******************************************************************
�������·�ָ���Q
*******************************************************************/
void INCOMING_MSG_Q_PUSH(U8* msg_str,int len)
{ 
	tracker_struct_msg *inmsg=NULL;
	if((msg_str == NULL)||(len==0)||(len > TRACKER_COMM_QUEUE_BUFFER_SIZE)){
	return;
	}
	inmsg = (tracker_struct_msg *)myMalloc(sizeof(tracker_struct_msg));
	memcpy(inmsg->body_msg, msg_str,len);
	inmsg->msgLen=len;
	trackimo_receive_deal(inmsg);
	myFree(inmsg);
}

void Tracker_SMS_parse(int iPara)
{
	pdu_data_struct * msg = (pdu_data_struct *)iPara;
    printf("Tracker_SMS_parse pdu_data->pdu_length=%d\r\n",msg->pdu_length);
    printf("pdu_data->pdu=%s\r\n",msg->pdu);
    INCOMING_MSG_Q_PUSH(msg->pdu,msg->pdu_length);
}



