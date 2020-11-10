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

//按键回调函数
typedef pfDefaultFunction pfKeyCB;

/*
	功能作用:清除按键事件
	参数说明:无
	返 回 值:无
	开发人员:付华强
*/
void clearMyKeyEvents(void);
/*
	功能作用:设置按键事件响应函数
	参数说明:key	指定按键
			 event	指定按键事件
			 func	响应函数
	返 回 值:成功与否
	开发人员:付华强
*/
BOOL setMyKeyEvent(int key, int event, pfKeyCB func);
/*
	功能作用:获取按键事件响应函数
	参数说明:key	指定按键
			 event	指定按键事件			 
	返 回 值:如题
	开发人员:付华强
*/
pfKeyCB getMyKeyEvent(int key, int event);
void tracker_sos_key_longpress(void);
void my_key_driver_init(void);
BOOL setDoubleClick(void);
#if defined(__cplusplus)
}
#endif 

#endif //MY_KEY_FHQ_20190722_H



