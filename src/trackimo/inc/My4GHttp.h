#ifndef __MY_4G_HTTP_H_20200926__
#define __MY_4G_HTTP_H_20200926__

#include "MyDefine.h"
#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

void resetDownloadPara(void);
BOOL cacheDownloadBuffer(char * pSrc, int iLen);
int getDownloadLength(void);
BOOL isDownloading(void);
char * getDownloadFileBuffer(void);
int getDownloadFileLength(void);

void module4g_poweron_init_synchronous(void);
BOOL module4g_download(const char * pWebSite, int port, const char * pDir);

#if defined(__cplusplus)
}
#endif 

#endif //__MY_4G_HTTP_H_20200926__



