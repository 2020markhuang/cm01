/**
@brief 本文件定义了USSD发送队列
$Id: TrackerQueue.h  2019-03-13  viva huang $
**/

#ifndef TRACKERQUEUE_H
#define TRACKERQUEUE_H
#include "MyCompilerDefine.h"
#include "MyBaseDefine.h"
#include "MyMemory.h"
#include "MyLog.h"
#include "TrackimoFrame.h"
#define TRACKER_MESSAGE_QUEUE_MAXNUM 20   //最大队列数
#define TRACKER_MAX_PDU_SIZE 176

typedef struct pdu_data_struct
{
    U16  pdu_length;
    U8   pdu[TRACKER_MAX_PDU_SIZE];
} pdu_data_struct;

/*******************************************************************
上传MODULE的Q
*******************************************************************/
void module_message_buffer_init(void);

void  module_message_buffer_push(tracker_struct_msg *pmessage);

tracker_struct_msg* module_message_buffer_pop(void);

tracker_struct_msg* module_message_buffer_get(void);

void module_message_buffer_clear(void);

U32 module_message_buffer_count(void);

void INCOMING_MSG_Q_PUSH(U8* msg_str,int len);

void Tracker_SMS_parse(int iPara);

#endif
