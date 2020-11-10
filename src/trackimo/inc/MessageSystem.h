#ifndef MESSAGE_SYSTEM_FHQ_20190722_H
#define MESSAGE_SYSTEM_FHQ_20190722_H

#include "MyDefine.h"


#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

typedef enum{
TASK_MAIN,
TASK_WATCH,
TASK_4G,
TASK_MAX
}enumTask;

typedef struct{
int from;
int to;
U32 id;
U32 wParam;
int lpParam;
U32 time;
}MYMSG;

typedef void (*pfProcessMsg)(MYMSG * pMsg);
typedef BOOL (*pfProcessIdle)(void);

/*
	功能作用:设置消息处理函数
	参数说明:task		任务
			 process	处理函数
	返 回 值:无
	开发人员:付华强
*/
void MySetMsgProcess(int task, pfProcessMsg process);
/*
	功能作用:获取消息idle函数
	参数说明:task		任务
	返 回 值:如题
	开发人员:付华强
*/
pfProcessIdle MyGetMsgIdle(int task);
/*
	功能作用:设置消息idle函数
	参数说明:task		任务
			 idle		idle函数
	返 回 值:无
	开发人员:付华强
*/
void MySetMsgIdle(int task, pfProcessIdle idle);

/*
	功能作用:初始化消息系统
	参数说明:task		任务
	返 回 值:成功与否
	开发人员:付华强
*/
BOOL MyInitMessageSystem(int task);
/*
	功能作用:将消息邮递到任务
	参数说明:from	从哪个任务
			 to		到哪个任务
			 id		消息id
			 wParam	32位参数1
			lpParam	指针参数
	返 回 值:成功与否
	开发人员:付华强
*/
BOOL MyPostMessage(int from, int to, U32 id, U32 wParam, int lpParam);


/*
	功能作用:消息循环
	参数说明:task	任务
	返 回 值:成功与否
	开发人员:付华强
*/
void MyMessageLoop(int task);

void MySetMsgPoolHandle(int task, void *handle);
void MyMessageStandTask(int iTask, int iMsgs, pfProcessMsg process);

#if defined(__cplusplus)
}
#endif 

#endif //MESSAGE_SYSTEM_FHQ_20190722_H



