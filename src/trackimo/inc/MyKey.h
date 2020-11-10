#ifndef MY_KEY_FHQ_20190722_H
#define MY_KEY_FHQ_20190722_H

#include "MyDefine.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

typedef enum{
MYKEY_RESET,	
MYKEY_POWER,
MYKEY_ENTRY,
MYKEY_MAX	
}enumMyKey;
typedef enum{
KEYEVENT_RELEASE,
KEYEVENT_PRESS,	
KEYEVENT_LONGPRESS,
KEYEVENT_REPEAT,
KEYEVENT_DOUBLECLICK,	
KEYEVENT_MAX	
}enumMyKeyEvent;

typedef enum{
	WATCH_MSG_KEY_INT_KEYPAD,
	WATCH_MSG_KEY_INT_POWER,
	WATCH_MSG_KEY_INT_MAX		
}enumWatchKeyInt;
typedef struct{
BOOL bDown;
BOOL bLongPress;
int iTime;
}structKeyState;

//�����ص�����
typedef pfDefaultFunction pfKeyCB;

/*
	��������:��������¼�
	����˵��:��
	�� �� ֵ:��
	������Ա:����ǿ
*/
void clearMyKeyEvents(void);
/*
	��������:���ð����¼���Ӧ����
	����˵��:key	ָ������
			 event	ָ�������¼�
			 func	��Ӧ����
	�� �� ֵ:�ɹ����
	������Ա:����ǿ
*/
BOOL setMyKeyEvent(int key, int event, pfKeyCB func);
/*
	��������:��ȡ�����¼���Ӧ����
	����˵��:key	ָ������
			 event	ָ�������¼�			 
	�� �� ֵ:����
	������Ա:����ǿ
*/
pfKeyCB getMyKeyEvent(int key, int event);
void tracker_sos_key_longpress(void);
void my_key_driver_init(void);
BOOL setDoubleClick(void);
#if defined(__cplusplus)
}
#endif 

#endif //MY_KEY_FHQ_20190722_H



