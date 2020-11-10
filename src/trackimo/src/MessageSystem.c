#include <stddef.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "MessageSystem.h"

typedef struct{
void * handleMsgPool;
pfProcessMsg process;
pfProcessIdle idle;
}structMsgPara;

static structMsgPara static_msgPool[TASK_MAX];


BOOL MyInitMessageSystem(int task){
	if((task < 0) || (task >= TASK_MAX))return FALSE;
	memset(&static_msgPool[task], 0, sizeof(structMsgPara));
	return TRUE;
}


static structMsgPara * MyGetMsgPara(int task){
	if((task < 0) || (task >= TASK_MAX))return NULL;
	return static_msgPool + task;
}



static pfProcessMsg MyGetMsgProcess(int task){
	structMsgPara * pPara = MyGetMsgPara(task);

	if(!pPara)return NULL;
	return pPara->process;
}
void MySetMsgProcess(int task, pfProcessMsg process){
	structMsgPara * pPara = MyGetMsgPara(task);

	if(!pPara)return;
	pPara->process = process;
}


void * MyGetMsgPoolHandle(int task){
	structMsgPara * pPara = MyGetMsgPara(task);

	if(!pPara)return NULL;
	return pPara->handleMsgPool;
}
void MySetMsgPoolHandle(int task, void *handle){
	structMsgPara * pPara = MyGetMsgPara(task);

	if(!pPara)return;
	pPara->handleMsgPool = handle;
}

pfProcessIdle MyGetMsgIdle(int task){
	structMsgPara * pPara = MyGetMsgPara(task);

	if(!pPara)return NULL;
	return pPara->idle;
}
void MySetMsgIdle(int task, pfProcessIdle idle){
	structMsgPara * pPara = MyGetMsgPara(task);

	if(!pPara)return;
	pPara->idle = idle;
}


static BOOL MyProcessMessage(int task, MYMSG * pMsg){
	pfProcessMsg fun;

	if(!pMsg)return FALSE;
	if((task < 0) || (task >= TASK_MAX))return FALSE;
	if(task != pMsg->to)return FALSE;
	fun = MyGetMsgProcess(task);
	if(!fun)return FALSE;
	fun(pMsg);
	return TRUE;
}



BOOL MyPostMessage(int from, int to, U32 id, U32 wParam, int lpParam){
	MYMSG msg;

	memset(&msg, 0, sizeof(msg));
	msg.from = from;
	msg.to = to;
	msg.id = id;
	msg.wParam = wParam;
	msg.lpParam = lpParam;

{
	structMsgPara * pPara = MyGetMsgPara(from);

	if(!pPara)return FALSE;	
	
    BaseType_t xHigherPriorityTaskWoken;
    /* We have not woken a task at the start of the ISR*/
    xHigherPriorityTaskWoken = pdFALSE;
    while (xQueueSendFromISR(pPara->handleMsgPool, (void*)&msg, &xHigherPriorityTaskWoken) != pdTRUE);

    /* Now the buffer is empty we can switch context if necessary.*/
    if (xHigherPriorityTaskWoken) {
        /*Actual macro used here is port specific.*/
        portYIELD_FROM_ISR(pdTRUE);
    }	
	return TRUE;
}
	
}



void MyMessageLoop(int task){
	MYMSG msg;
	
	structMsgPara * pPara = MyGetMsgPara(task);

	if(!pPara)return;	
	while(1){
		if (xQueueReceive(pPara->handleMsgPool, &msg, portMAX_DELAY)){
			MyProcessMessage(task, &msg);
		}
	}
}
void MyMessageStandTask(int iTask, int iMsgs, pfProcessMsg process)
{
	QueueHandle_t handle;

	handle = xQueueCreate(iMsgs, sizeof(MYMSG));	   
	MyInitMessageSystem(iTask);
	MySetMsgProcess(iTask, process);
	MySetMsgPoolHandle(iTask, handle);
	MyMessageLoop(iTask);
}



