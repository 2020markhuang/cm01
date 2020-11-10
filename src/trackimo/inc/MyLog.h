#ifndef MY_LOG_FHQ_20190903_H
#define MY_LOG_FHQ_20190903_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#define SUPPORT_LOG_GPS
#define SUPPORT_LOG_MODULE_4G
#define SUPPORT_LOG_MAINTASK
#define SUPPORT_LOG_WATCHTASK
#define SUPPORT_LOG_GSENSOR
#define SUPPORT_LOG_BUZZER
#define SUPPORT_LOG_MAIN
#define SUPPORT_LOG_LED

#ifdef SUPPORT_LOG_MAIN
#define main_printf printf
#else
#define main_printf(...)
#endif

#ifdef SUPPORT_LOG_GPS
#define gps_printf printf
#else
#define gps_printf(...)
#endif

#ifdef SUPPORT_LOG_MODULE_4G
#define module4g_printf printf
#else
#define module4g_printf(...)
#endif

#ifdef SUPPORT_LOG_MAINTASK
#define maintask_printf printf
#else
#define maintask_printf(...)
#endif

#ifdef SUPPORT_LOG_WATCHTASK
#define watchtask_printf printf
#else
#define watchtask_printf(...)
#endif


#ifdef SUPPORT_LOG_GPS
#define gps_printf printf
#else
#define gps_printf(...)
#endif

#ifdef SUPPORT_LOG_GSENSOR
#define gsensor_printf printf
#else
#define gsensor_printf(...)
#endif

#ifdef SUPPORT_LOG_KEY
#define key_printf printf
#else
#define key_printf(...)
#endif

#ifdef SUPPORT_LOG_BUZZER
#define buzzer_printf printf
#else
#define buzzer_printf(...)
#endif

#ifdef SUPPORT_LOG_LED
#define led_printf printf
#else
#define led_printf(...)
#endif

#if defined(__cplusplus)
}
#endif 

#endif //MY_LOG_FHQ_20190903_H



