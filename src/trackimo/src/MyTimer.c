#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "MyTimer.h"

#ifdef SUPPORT_MY_TIMER
typedef struct{
U32 tickOver;
pfDefaultFunction cb;
}structTimer;


static void onMyTimerCB(TimerHandle_t pxTimer);


static structTimer static_arrayTimer[TIMER_MAX];
static TimerHandle_t static_hTimer = NULL;


static U32 getSystemTimeTick(void){
	return xTaskGetTickCount();
}
static structTimer * getMyTimerByID(int id){
	if((id < 0) || (id >= TIMER_MAX))return NULL;
	return static_arrayTimer + id;
}

static int getWaitTimeForMyTimer(void){
	int i;
	U32 iMin = 0;
	structTimer * p;
	U32 now = getSystemTimeTick();

	p = static_arrayTimer;
	for(i = 0; i < TIMER_MAX; i++, p++){
		if(p->tickOver == 0)continue;
		if(iMin == 0)iMin = p->tickOver;
		else if(iMin > p->tickOver)iMin = p->tickOver;
	}
	if(iMin == 0)return -1;
	if(iMin <= now)return 0;
	return iMin - now;
}

static void restartMyTimer(void){
	int time = getWaitTimeForMyTimer();
	TimerHandle_t handle = static_hTimer;

	if(time < 0)return;
	else if(time == 0){
		onMyTimerCB(handle);
		return;
	}
	xTimerStop(handle, 0);
	xTimerChangePeriod(handle, time, 0);
	xTimerStart(handle, 0);
}

void startMyTimer(int id, int time, pfDefaultFunction cb){
	structTimer * p;

	p = getMyTimerByID(id);
	if(!p)return;
	p->tickOver = time + getSystemTimeTick();
	p->cb = cb;

	restartMyTimer();
}

void stopMyTimer(int id){
	structTimer * p;

	p = getMyTimerByID(id);
	if(!p)return;
	memset(p, 0, sizeof(structTimer));

	restartMyTimer();
}

static void executeMyTimerOver(void){
	int i;
	structTimer * p;
	U32 now = getSystemTimeTick();
	pfDefaultFunction cb;


	p = static_arrayTimer;
	for(i = 0; i < TIMER_MAX; i++, p++){
		if(p->tickOver == 0)continue;
		if(p->tickOver <= now){
			cb = p->cb;
			memset(p, 0, sizeof(structTimer));
			if(cb)cb();
		}
	}
}
static void onMyTimerCB(TimerHandle_t pxTimer){
	xTimerStop(pxTimer, 0);
	executeMyTimerOver();
	restartMyTimer();
}
void initMyTimer(void){
	memset(static_arrayTimer, 0, sizeof(static_arrayTimer));

	static_hTimer  = xTimerCreate("myTimer", 1, FALSE, NULL, onMyTimerCB);
}
void deinitMyTimer(void){
	if(static_hTimer){
		xTimerStop(static_hTimer, 0);
		xTimerDelete(static_hTimer, 0);
	}
	memset(static_arrayTimer, 0, sizeof(static_arrayTimer));
	
}
#endif
