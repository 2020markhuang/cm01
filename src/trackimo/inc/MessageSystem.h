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
	��������:������Ϣ������
	����˵��:task		����
			 process	������
	�� �� ֵ:��
	������Ա:����ǿ
*/
void MySetMsgProcess(int task, pfProcessMsg process);
/*
	��������:��ȡ��Ϣidle����
	����˵��:task		����
	�� �� ֵ:����
	������Ա:����ǿ
*/
pfProcessIdle MyGetMsgIdle(int task);
/*
	��������:������Ϣidle����
	����˵��:task		����
			 idle		idle����
	�� �� ֵ:��
	������Ա:����ǿ
*/
void MySetMsgIdle(int task, pfProcessIdle idle);

/*
	��������:��ʼ����Ϣϵͳ
	����˵��:task		����
	�� �� ֵ:�ɹ����
	������Ա:����ǿ
*/
BOOL MyInitMessageSystem(int task);
/*
	��������:����Ϣ�ʵݵ�����
	����˵��:from	���ĸ�����
			 to		���ĸ�����
			 id		��Ϣid
			 wParam	32λ����1
			lpParam	ָ�����
	�� �� ֵ:�ɹ����
	������Ա:����ǿ
*/
BOOL MyPostMessage(int from, int to, U32 id, U32 wParam, int lpParam);


/*
	��������:��Ϣѭ��
	����˵��:task	����
	�� �� ֵ:�ɹ����
	������Ա:����ǿ
*/
void MyMessageLoop(int task);

void MySetMsgPoolHandle(int task, void *handle);
void MyMessageStandTask(int iTask, int iMsgs, pfProcessMsg process);

#if defined(__cplusplus)
}
#endif 

#endif //MESSAGE_SYSTEM_FHQ_20190722_H



