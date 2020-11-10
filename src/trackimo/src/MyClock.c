#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "WatchDateTime.h"
#include "MyClock.h"


static structTimeStamp static_baseTimeStamp = 0;
static U32 static_baseClick = 0;
static U32 static_iSystemOnTime = 0;


static U32 getMySystemTimeTick(void){
	return xTaskGetTickCount();
}
static void clock_refresh_base(U32 click, structTimeStamp stamp){
	static_iSystemOnTime += (click - static_baseClick) / 1000;
	static_baseClick = click;
	static_baseTimeStamp = stamp;
}
static void clock_read_rtc(void){
	structTimeStamp timeStamp;
	hal_rtc_time_t rtc;

	hal_rtc_get_time(&rtc);
	timeStamp = getTimeStampByPara(rtc.rtc_year + 2000, rtc.rtc_mon, rtc.rtc_day, rtc.rtc_hour, rtc.rtc_min, rtc.rtc_sec, 0);
	clock_refresh_base(getMySystemTimeTick(), timeStamp);
}
static void clock_write_rtc(structTimeStamp stampUTC){
	hal_rtc_time_t rtc;
	int year,month,day;

	getDateOfYear(stampUTC, &year, &month, &day);

	hal_rtc_get_time(&rtc);
	rtc.rtc_year = year - 2000;
	rtc.rtc_mon = month;
	rtc.rtc_day = day;
	rtc.rtc_hour = getHourOfTime(stampUTC);
	rtc.rtc_min = getMinuteOfTime(stampUTC);
	rtc.rtc_sec = getSecondOfTime(stampUTC);
	hal_rtc_set_time(&rtc);
}
void clock_init(void){
	clock_read_rtc();
	if(1)
		{
			structTimeStamp time = getTimeStampByPara(2020,10,15,12,12,0, 0);
			setCurrentHardwareTime(time);
		}		
}

structTimeStamp getCurrentHardwareTime(void)
{ 
	//structTimeStamp time; 
	//hal_rtc_time_t rtc;
	//hal_rtc_get_time(&rtc); 
	//printf("xxx:%d,%d,%d\r\n", rtc.rtc_year, rtc.rtc_mon, rtc.rtc_day);  
	//time = getTimeStampByPara(rtc.rtc_year + 2000, rtc.rtc_mon, rtc.rtc_day, rtc.rtc_hour, rtc.rtc_min, rtc.rtc_sec, 0); 
	//printf("yyyy:%d,%d,%d,%d,%d,%d\r\n", getYearOfTime(time), getMonthOfTime(time), getDayOfTime(time), getHourOfTime(time), getMinuteOfTime(time), getSecondOfTime(time)); 
	//return time;
	 return addTimeStamp(static_baseTimeStamp, getMySystemTimeTick() - static_baseClick);
}


void setCurrentHardwareTime(structTimeStamp stamp){
	clock_write_rtc(stamp);
	clock_read_rtc();
}


