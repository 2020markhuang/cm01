
#ifndef MY_BUFFER_H_20190906
#define MY_BUFFER_H_20190906

#include "MyDefine.h"
#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#define MAX_BUFFER_ITEMS 30

typedef struct{
char * pBuffer;
int iItemSize;
int iCount;
}structMyBuffer;


BOOL my_buffer_init(structMyBuffer * pMyBuffer, char * pBuffer, int iItemSize, int iCount);
char * my_buffer_malloc(structMyBuffer * pMyBuffer);
BOOL my_buffer_free(structMyBuffer * pMyBuffer, char * pBuffer);

#if defined(__cplusplus)
}
#endif 

#endif //MY_BUFFER_H_20190906



