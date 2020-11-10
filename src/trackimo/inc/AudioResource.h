#ifndef AUDIO_RESOURCE_H
#define AUDIO_RESOURCE_H

#include "MyDefine.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum{
AUDIO_ID_POWERON_AND_CONN,
AUDIO_ID_NEXT_TIME,
AUDIO_ID_DIDA,
AUDIO_ID_WOZAI,
AUDIO_ID_WIFI_OK,
AUDIO_ID_NOT_READY_YET,	//5
AUDIO_ID_MAX	
}enumAudioResourceID;

extern const AUDIO g_AudioResourceArray[];


#if defined(__cplusplus)
}
#endif

#endif // AUDIO_RESOURCE_H
