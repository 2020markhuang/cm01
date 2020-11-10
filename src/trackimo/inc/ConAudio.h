#ifndef CONTROLLER_AUDIO_20190124_FHQ_H
#define CONTROLLER_AUDIO_20190124_FHQ_H

#include "MyDefine.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum{
AUDIO_LOADMODE_BUFFER,
AUDIO_LOADMODE_FILE,
AUDIO_LOADMODE_MAX
}enumAudioLoadMode;

/*
	功能作用:获取AUDIO资源装载方式
	参数说明:无
	返 回 值:如题
	开发人员:付华强
*/
int getAudioLoadMode(void);

/*
	功能作用:设置AUDIO资源装载方式
	参数说明:mode		装载方式(enumAudioLoadMode)
	返 回 值:无
	开发人员:付华强
*/
void setAudioLoadMode(int mode);


/*
	功能作用:获取AUDIO资源
	参数说明:id		AUDIOid
	返 回 值:如题
	开发人员:付华强
*/
const AUDIO * getAudioByID(int id);


#if defined(__cplusplus)
}
#endif

#endif // CONTROLLER_AUDIO_20190124_FHQ_H
