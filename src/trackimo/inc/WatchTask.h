#ifndef WATCH_TASK_FHQ_20190722_H
#define WATCH_TASK_FHQ_20190722_H


#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
typedef enum {
	WATCH_MSG_IO_PROCESS,
	WATCH_MSG_GSENSOR_INT,
	WATCH_MSG_EXIT_SLEEP,
	WATCH_MSG_GPS_INT,
	WATCH_MSG_MAX
 } enumWatchMsg;

typedef enum{
	IO_STEP_KEY,
	IO_STEP_LED,
	IO_STEP_BUZZER,
}enumWatchStep;

void watch_task_post_message(int msg, U32 wPara, int lpPara);
void watch_task(void *pvParameters);

#if defined(__cplusplus)
}
#endif 

#endif	//WATCH_TASK_FHQ_20190722_H

