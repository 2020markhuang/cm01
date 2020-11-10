#ifndef TRACKMY4GQUEUE_H
#define TRACKMY4GQUEUE_H
#include "MyCompilerDefine.h"
#include "MyBaseDefine.h"
#include "MyMemory.h"
#include "MyLog.h"
#include "TrackimoFrame.h"
#define MODULE_AT_MESSAGE_QUEUE_MAXNUM 20   //最大队列数
#define  MOUDULE_AT_COMM_QUEUE_BUFFER_SIZE 256
#define SMS_FIFO_SIZE 10    //FIFO Size is dependent on the CPU performance 
typedef struct moudule_at_struct_msg
{
     int msgLen;
     U8  body_msg[MOUDULE_AT_COMM_QUEUE_BUFFER_SIZE];
} moudule_at_struct_msg;
/*******************************************************************
上传MODULE的Q
*******************************************************************/
void  module_at_message_buffer_push(moudule_at_struct_msg *pmessage);

moudule_at_struct_msg* module_at_message_buffer_pop(void);

moudule_at_struct_msg* module_at_message_buffer_get(void);

void module_at_message_buffer_clear(void);

U32 module_at_message_buffer_count(void);
#endif
