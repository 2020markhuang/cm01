#include "MyDefine.h"
#include "MyCommon.h"
#include "Module4GTask.h"
#include "My4G.h"
#include "My4GQueue.h"
static moudule_at_struct_msg  module_at_message_buffer[MODULE_AT_MESSAGE_QUEUE_MAXNUM];
extern module_context_struct *module_context_p;
/** ��ʾĿǰ��ͷλ�� **/
 U32 s_module_at_current_head=0;

/** ��ʾĿǰ���һ���ڵ����һ���ڵ� **/
 U32 s_module_at_current_tail=0;

/** ����Ϣ�洢���м���һ��Ԫ�� **/
void  module_at_message_buffer_push(moudule_at_struct_msg *pmessage)
{
	 if(s_module_at_current_tail == MODULE_AT_MESSAGE_QUEUE_MAXNUM)
	 {
	 	s_module_at_current_tail = 0;
	 }
	 memset(&(module_at_message_buffer[s_module_at_current_tail]), 0x00, sizeof(moudule_at_struct_msg));
	 memcpy(&(module_at_message_buffer[s_module_at_current_tail].body_msg),(pmessage->body_msg),pmessage->msgLen);  
	 module_at_message_buffer[s_module_at_current_tail].msgLen=pmessage->msgLen;	
	 s_module_at_current_tail++;
	 //tracker_prompt_trace("��Ϣѹ����: %d\r\n",s_module_at_current_tail);
	 printf("module_at_message_buffer_push, head=%d, tail=%d\r\n", s_module_at_current_head,s_module_at_current_tail);
}

/** ����Ϣ�洢����ȡ��һ��Ԫ�� **/
moudule_at_struct_msg* module_at_message_buffer_pop(void)
{
	if((module_context_p->at_send_enable==1)&&(module_at_message_buffer_count()== 0))
	{
		return NULL;
	}
	printf("module_at_message_buffer_pop, head=%d, tail=%d\r\n", s_module_at_current_head,s_module_at_current_tail);
	if(s_module_at_current_head == MODULE_AT_MESSAGE_QUEUE_MAXNUM){
		s_module_at_current_head = 0;
	}  
	return &(module_at_message_buffer[s_module_at_current_head]);
}

/** ����Ϣ�洢����ȡ��һ��Ԫ�� **/
moudule_at_struct_msg* module_at_message_buffer_get(void)
{
    return &(module_at_message_buffer[s_module_at_current_head]);
}

/** �����Ϣ�洢�� **/
void module_at_message_buffer_clear(void)
{
    if((s_module_at_current_head==0)&&(s_module_at_current_tail==0))
          return;

    memset(&module_at_message_buffer, 0, sizeof(module_at_message_buffer));
	s_module_at_current_head = 0;
	s_module_at_current_tail = 0;
}

/** ��ȡ��Ϣ�洢����Ԫ�صĸ��� **/
U32 module_at_message_buffer_count(void)
{
	if(s_module_at_current_head == s_module_at_current_tail){
		return 0;
	}
	else if(s_module_at_current_head < s_module_at_current_tail){
		return s_module_at_current_tail - s_module_at_current_head;
	}
	else{
		return s_module_at_current_tail + MODULE_AT_MESSAGE_QUEUE_MAXNUM - s_module_at_current_head;
	}
}

/*******************************************************************
ģ����Ŵ���
*******************************************************************/
static int _sms_read_index = 0 ; 
static int _sms_write_index = 0 ; 
U8 Incoming_message_index[SMS_FIFO_SIZE];

BOOL isSMSFIFOEmpty(void)
{
   return(_sms_read_index ==_sms_write_index);
}

BOOL SMS_Push(U8 index) //Push return data must be true. If return false, please increase FIFO size. 
{ 
	int tmp = _sms_write_index ; 
	tmp++; 
	if(tmp >= SMS_FIFO_SIZE) 
	tmp = 0 ; 
	if(tmp ==_sms_read_index) 
	return FALSE; 

	Incoming_message_index[_sms_write_index]=index;
	printf("SMS_Push Incoming_message_index[%d] = %d\r\n",_sms_write_index,Incoming_message_index[_sms_write_index]);
	_sms_write_index = tmp ; 
	return TRUE; 
} 

U8 SMS_POP(void)
{ 
	int tmp ; 
	U8 index=0;
	if(isSMSFIFOEmpty()) 
	return index; 
	index=Incoming_message_index[_sms_read_index];
	printf("SMS_POP data[%d] = %d\r\n",_sms_read_index,index);
	tmp = _sms_read_index + 1; 
	if(tmp >= SMS_FIFO_SIZE) 
	tmp = 0 ; 
	_sms_read_index = tmp ; 
	return index; 
} 

void SMS_cleardata(void)
{
  _sms_read_index = 0;
  _sms_write_index = 0;
  memset(Incoming_message_index, 0, sizeof(Incoming_message_index));
}

