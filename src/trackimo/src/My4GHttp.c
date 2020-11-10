#include "MyCommon.h"
#include "Module4GTask.h"
#include "My4GHttp.h"

#define MAX_DOWNLOAD_SIZE 102400
static char static_bufferDownload[MAX_DOWNLOAD_SIZE];
static int static_iDownloadLen = 0;
static int static_iFileLength = 0;
static BOOL static_bDownloading = FALSE;
static int static_iDataOffset = 0;
void resetDownloadPara(void){
	static_iDownloadLen = 0;
	static_iFileLength = 0;
	static_bDownloading = TRUE;
	static_iDataOffset = 0;
	return TRUE;
}
char * getDownloadFileBuffer(void){
	return static_bufferDownload + static_iDataOffset;
}
int getDownloadFileLength(void){
	return static_iFileLength;
}
BOOL cacheDownloadBuffer(char * pSrc, int iLen){
	char * p = NULL;
	char * pFlag = "--EOF--Pattern--\r\nOK\r\n";
	int iCmp = strlen(pFlag);
	
	if(!pSrc)return FALSE;
//	if((iLen + static_iDownloadLen) > static_iFileLength)return FALSE;
	memcpy(static_bufferDownload + static_iDownloadLen, pSrc, iLen);
	static_iDownloadLen += iLen;
	printf("cacheDownloadBuffer:%d,%d\r\n", iLen, static_iDownloadLen);
	if(static_iFileLength && (static_iDownloadLen >= (static_iFileLength + static_iDataOffset))){
//		printf("bbb:%d,%d\r\n", static_iFileLength, static_iDataOffset);
		p = static_bufferDownload + static_iFileLength + static_iDataOffset;
		iCmp = static_iDownloadLen - static_iFileLength - static_iDataOffset;
//		myMemDump(p, iCmp);
		iCmp = strlen(pFlag);
		if(memcmp(p, pFlag, iCmp) == 0){
			printf("download finish\r\n");
			static_bDownloading = FALSE;
      			module4g_task_post_message(MODULE4G_MSG_DOWNLOAD_FINISH, 0, 0);
		}
	}
	if((!static_iFileLength) && (static_iDownloadLen >= 300)){
		const char * pKeyLength = "Content-Length:";
		const char * pKeyData = "\r\nAccept-Ranges: bytes\r\n\r\n";
		
		char * pFind1 = NULL;
		char * pFind2 = NULL;
		char temp[20];

		pFind1 = strstr(static_bufferDownload, pKeyLength);
		if(pFind1){
			pFind1 += strlen(pKeyLength);
			pFind2 = strchr(pFind1, '\r');
		}
		if(pFind1 && pFind2){
			memset(temp, 0, sizeof(temp));
			strncpy(temp, pFind1, pFind2 - pFind1);
			static_iFileLength = atoi(temp);
		}
		if(pFind2){
			pFind1 = strstr(pFind2, pKeyData);
			if(pFind1){
				static_iDataOffset = pFind1 - static_bufferDownload;
				static_iDataOffset += strlen(pKeyData);
			}
		}
	}
	return TRUE;
}
int getDownloadLength(void){
	return static_iDownloadLen;
}
BOOL isDownloading(void){
	return static_bDownloading;
}


void module4g_poweron_init_synchronous(void){
	while(1){
//		module4g_task_get_synchronous_result(AT_4G_CMD_CMEE, "=2");
		module4g_task_get_synchronous_result(AT_4G_CMD_SET_CPIN, "?");
		module4g_task_get_synchronous_result(AT_4G_CMD_SET_CSQ, NULL);
		module4g_task_get_synchronous_result(AT_4G_CMD_KCNXCFG,"=1,\"GPRS\",\"m2m.tele2.com\"");
		module4g_task_get_synchronous_result(AT_4G_CMD_KCNXTIMER,"=1,60,2,70");
		break;
	}
}

static char *  module4g_find_string(const char * pKey){
	int iPos = 0;
	int iLen = getSynchronousLen();
	int iStrLen = 0;
	char * pResult;
	char * pFind = NULL;
	
	pResult = getSynchronousBuffer();
	while(iPos < iLen){
		iStrLen = strlen(pResult);
		pFind = strstr(pResult, pKey);
		if(pFind)break;
		iPos += iStrLen;
		pResult += iStrLen + 1;
	}
	if(pFind)pFind += strlen(pKey) + 1;
	if(pFind)printf(pFind);
	
	return pFind;
}
static int module4g_http_config(const char * pWebSite, int port){
	char * pResult;
	char * pFind;
	int sessionid = 0;
	const char * pCmd = "\r\n+KHTTPCFG:";
	
	module4g_task_get_synchronous_result(AT_4G_CMD_KHTTPCFG,"=1,\"%s\",%d", pWebSite, port);
	pResult = module4g_find_string(pCmd);
	pFind = strchr(pResult, '\r');
	if(pFind)*pFind = 0;
	sessionid = atoi(pResult);
	return sessionid;
}
static BOOL module4g_wait_http_status(void){
	char * pBuffer = getSynchronousBuffer();
	char * pResult = NULL;
	const char * pCmd = "+KHTTP_IND:";
	const char * pError = "+KHTTP_ERROR:";
	char * pFind = NULL;

	while(!pResult){
		take_4g_semaphore(pBuffer, getSynchronousLenPtr(), NULL, FALSE);
//		printf(pBuffer);
		pResult = module4g_find_string(pError);
		if(pResult)	return FALSE;
		pResult = module4g_find_string(pCmd);
		if(pResult)break;
	}
	delete_4g_semaphore();
	if(pResult)pFind = strchr(pResult, ',');
	if(pFind){
		pFind++;
		return atoi(pFind) == 1;
	}
	return FALSE;
	
}
BOOL module4g_download(const char * pWebSite, int port, const char * pDir){
	char * pResult = NULL;
	int sessionid;
	const char * pHead = "Accept : application/octet-stream,application/download,application/force-download\r\n--EOF--Pattern--";
	const char * pConnect = "CONNECT";
	const char * pHTTP = "HTTP/";
	
	sessionid = module4g_http_config(pWebSite, port);
	if(sessionid <= 0)sessionid = 1;
	if(sessionid <= 0)return FALSE;

	module4g_task_get_synchronous_result(AT_4G_CMD_KHTTPHEADER,"=%d", sessionid);
	pResult = module4g_find_string(pConnect);
	if(!pResult)return FALSE;
	module4g_task_send_synchronous_cmd(pHead);
	if(!module4g_wait_http_status())return FALSE;
	module4g_task_get_synchronous_result(AT_4G_CMD_KHTTPGET,"=%d,\"%s\"", sessionid, pDir);
	pResult = module4g_find_string(pConnect);
	if(!pResult)return FALSE;
	resetDownloadPara();
	return TRUE;
}

