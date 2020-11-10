#ifndef MY_COMPILER_DEFINE_20190722_FHQ_H
#define MY_COMPILER_DEFINE_20190722_FHQ_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#undef I32
#undef U32
#undef I16
#undef U16
#undef U8
#undef I8
#undef I64
#undef TRUE
#undef FALSE
#undef BOOL



#ifdef WIN32
#define I8    signed char
#define U8  unsigned char     /* unsigned 8  bits. */
#define I16   signed short    /*   signed 16 bits. */
#define U16 unsigned short    /* unsigned 16 bits. */
#define I32   signed long   /*   signed 32 bits. */
#define U32 unsigned long   /* unsigned 32 bits. */
typedef int   BOOL;

#define I64 __int64 /* signed 64 bits. */

#else
	
#include <stdint.h>
	
#define I8    int8_t
#define U8  uint8_t     /* unsigned 8  bits. */
#define I16   int16_t    /*   signed 16 bits. */
#define U16 uint16_t    /* unsigned 16 bits. */
#define I32   int32_t   /*   signed 32 bits. */
#define U32 uint32_t   /* unsigned 32 bits. */
typedef int   BOOL;
	
#define I64 long long int /* signed 64 bits. */

#endif



#define TRUE (1)
#define FALSE (0)

#define ISBOOL(value) (((value)==0)||((value)==1))

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#ifdef WIN32
#else
#include "hal.h"
#include "hal_dcxo.h"
#include "hal_platform.h"
#include "hal_gpt.h"
#include "sensor_alg_interface.h"
#include "hal_keypad.h"
#include "keypad_custom.h"
#include "gnss_api.h"
#include "memory_attribute.h"
#include "hal_gpio.h"
#include "hal_pinmux_define.h"
#endif


#if defined(__cplusplus)
}
#endif 

#endif //MY_COMPILER_DEFINE_20190722_FHQ_H



