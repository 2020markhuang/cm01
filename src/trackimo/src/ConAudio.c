#include <stddef.h>

#include "AudioResource.h"
#include "ConAudio.h"


//音频资源装载方式
static int static_iMode = AUDIO_LOADMODE_BUFFER;

int getAudioLoadMode(void){
	return static_iMode;
}
void setAudioLoadMode(int mode){
	static_iMode = mode;
}
static const AUDIO * getAudioByID_BufferMode(int id){
	return &g_AudioResourceArray[id];
}

static const AUDIO * getAudioByID_FileMode(int id){
	return NULL;
}

const AUDIO * getAudioByID(int id){
	if((id < 0) || (id >= AUDIO_ID_MAX))return NULL;
	switch(getAudioLoadMode()){
		case AUDIO_LOADMODE_BUFFER:
			return getAudioByID_BufferMode(id);
		case AUDIO_LOADMODE_FILE:
			return getAudioByID_FileMode(id);
		default:
			return NULL;
	}
}
