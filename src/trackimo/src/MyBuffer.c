#include <string.h>
#include <stdio.h>

#include "MyBuffer.h"

#define MAX_BUFFER_ITEMS 30


static char static_flags[MAX_BUFFER_ITEMS / 8 + (MAX_BUFFER_ITEMS % 8 ? 1 : 0)];

BOOL my_buffer_init(structMyBuffer * pMyBuffer, char * pBuffer, int iItemSize, int iCount){
	if((!pMyBuffer) || (!pBuffer) || (iCount <= 0) || (iCount > MAX_BUFFER_ITEMS) || (iItemSize <= 0))return FALSE;
	memset(static_flags, 0, sizeof(static_flags));
	memset(pMyBuffer, 0, sizeof(structMyBuffer));
	pMyBuffer->pBuffer = pBuffer;
	pMyBuffer->iItemSize = iItemSize;
	pMyBuffer->iCount = iCount;
	return TRUE;
}

char * my_buffer_malloc(structMyBuffer * pMyBuffer){
	int i, iByte, iBit;
	
	if(!pMyBuffer)return NULL;
	for(i = 0; i < pMyBuffer->iCount; i++){
		iByte = i / 8;
		iBit = i % 8;
		if(!(static_flags[iByte] & (1 << iBit))){
			static_flags[iByte] |= (1 << iBit) ;
			return pMyBuffer->pBuffer + pMyBuffer->iItemSize * i;
		}
	}
	printf("!!!!\r\n");
	return NULL;
}
BOOL my_buffer_free(structMyBuffer * pMyBuffer, char * pBuffer){
	int iByte, iBit;
	int iIndex;
	
	if(!pMyBuffer || !pBuffer)return FALSE;
	iIndex = pBuffer - pMyBuffer->pBuffer;
	if(iIndex % pMyBuffer->iItemSize)return FALSE;
	iIndex /= pMyBuffer->iItemSize;
	
	iByte = iIndex / 8;
	iBit = iIndex % 8;
	static_flags[iByte] &=~ (1 << iBit) ;
	return TRUE;
}

