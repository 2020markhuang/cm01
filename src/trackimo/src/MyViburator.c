#include <stddef.h>
#include <string.h>

#include "MyCommon.h"
#include "MyViburator.h"

#ifdef SUPPORT_MY_VIBURATOR

static int static_iOldViburatorWorkMode = -1;
static int static_iViburatorTimeCount = 0;

void viburator_driver_init(void){
}
void viburator_on(void){
}
void viburator_off(void){
}
void on_viburator_mode_control(void){
	BOOL bInit = FALSE;
	int mode = getViburatorWorkmode();
	
	//if(!isEnableViburator())return;
	if(static_iOldViburatorWorkMode != mode){
		bInit = TRUE;
		static_iViburatorTimeCount = 0;
	}
	static_iOldViburatorWorkMode = mode;
	switch(getViburatorWorkmode()){
		case VIBURATOR_OFF:
			if(bInit)viburator_off();
			break;
		case VIBURATOR_ON:
			if(bInit)viburator_on();
			break;
		case VIBURATOR_SHORT:
			break;
		case VIBURATOR_LONG:
			break;
		case VIBURATOR_BREATH:
			break;
	
	}
	static_iViburatorTimeCount++;
}
#endif
