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
	��������:��ȡAUDIO��Դװ�ط�ʽ
	����˵��:��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getAudioLoadMode(void);

/*
	��������:����AUDIO��Դװ�ط�ʽ
	����˵��:mode		װ�ط�ʽ(enumAudioLoadMode)
	�� �� ֵ:��
	������Ա:����ǿ
*/
void setAudioLoadMode(int mode);


/*
	��������:��ȡAUDIO��Դ
	����˵��:id		AUDIOid
	�� �� ֵ:����
	������Ա:����ǿ
*/
const AUDIO * getAudioByID(int id);


#if defined(__cplusplus)
}
#endif

#endif // CONTROLLER_AUDIO_20190124_FHQ_H
