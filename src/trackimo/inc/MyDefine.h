#ifndef MY_DEFINE_20190722_H
#define MY_DEFINE_20190722_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#include "MyCompilerDefine.h"
#include "MyBaseDefine.h"
#include "MyMemory.h"
#include "MyLog.h"

#define SUPPORT_MY_4G
#define SUPPORT_MY_BLE
#define SUPPORT_MY_BUZZER
#define SUPPORT_MY_GPS
#define SUPPORT_MY_GSENSOR
#define SUPPORT_MY_KEY
#define SUPPORT_MY_LED
#define SUPPORT_MY_WIFI
#define SUPPORT_MY_TIMER
//#define SUPPORT_WATCH_DOG
//#define SUPPORT_MY_TP
//#define SUPPORT_MY_VIBURATOR
//#define TRACKIMO_TEST_MODE
typedef void (*pfDefaultFunction)(void);
typedef void (*pfCmdFun)(const char * pCmd, int iLen);
#if defined(__cplusplus)
}
#endif 

#endif //MY_DEFINE_20190722_H



