#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "WatchDateTime.h"
#include "MyClock.h"

#define BASE_YEAR 1970
#define BASE_MONTH 1
#define BASE_DAY 1

static const int MillSecondsOneDay = 1000 * 3600 * 24;

const static U8 days_of_month[]={
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
const static U8 days_of_month_for_leap_year[]={
	31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

structTimeStamp getCurrentTime(void){
	return getCurrentHardwareTime();
}

structTimeStamp getCurrentLocalTime(void){
	return addTimeStamp(getCurrentTime(),  getTimeDiffByTimeZone());
}
static int getYearOfDateString(const char * pBuf){
	char temp[5];

	if(!pBuf)return 0;
	temp[0] = pBuf[0];
	temp[1] = pBuf[1];
	temp[2] = pBuf[2];
	temp[3] = pBuf[3];
	temp[4] = 0;
	return atoi(temp);
}
static int getMonthOfDateString(const char * pBuf){
	char temp[3];

	if(!pBuf)return 0;
	temp[0] = pBuf[4];
	temp[1] = pBuf[5];
	temp[2] = 0;
	return atoi(temp);
}
static int getDayOfDateString(const char * pBuf){
	char temp[3];

	if(!pBuf)return 0;
	temp[0] = pBuf[6];
	temp[1] = pBuf[7];
	temp[2] = 0;
	return atoi(temp);
}
static int getHourOfTimeString(const char * pBuf){
	char temp[3];

	if(!pBuf)return 0;
	temp[0] = pBuf[0];
	temp[1] = pBuf[1];
	temp[2] = 0;
	return atoi(temp);
}
static int getMinuteOfTimeString(const char * pBuf){
	char temp[3];

	if(!pBuf)return 0;
	temp[0] = pBuf[2];
	temp[1] = pBuf[3];
	temp[2] = 0;
	return atoi(temp);
}
static int getSecondOfTimeString(const char * pBuf){
	char temp[3];

	if(!pBuf)return 0;
	temp[0] = pBuf[4];
	temp[1] = pBuf[5];
	temp[2] = 0;
	return atoi(temp);
}

int getDaysOfMonth(int month, BOOL bLeap){
	const U8 * pDays = bLeap ? days_of_month_for_leap_year : days_of_month;
	if((month <= 0) || (month > 12))return 0;
	return *(pDays + month - 1);
}

BOOL isTimeStampEmpty(structTimeStamp stamp){
	return stamp == 0;
}
BOOL isTimeStampDiffEmpty(structTimeStampDiff diff){
	return diff == 0;
}
void resetTimeStamp(structTimeStamp * pStamp){
	if(!pStamp)return;
	memset(pStamp, 0, sizeof(structTimeStamp));
}
BOOL isLeapYear(int year){
	return (((year % 4) == 0) && ((year % 100) != 0)) || (year % 400 == 0);
}

int getDaysFromStartForYear(int year){
	int iYear = BASE_YEAR;
	int days = 0;

	while(iYear < year){
		days += 365;
		if(isLeapYear(iYear))days++;
		iYear++;
	}
	return days;
}

int getDaysFormThisYearForMonth(int year, int month){
	BOOL leap = isLeapYear(year);
	int days = 0;
	int i;

	for(i = 1; i < month; i++){
		days += getDaysOfMonth(i, leap);
	}
	return days;
}
structTimeStamp setTimeStampDays(structTimeStamp time, int days){
	time %= MillSecondsOneDay;
	time += (I64)MillSecondsOneDay * days;
	return time;
}
structTimeStamp setTimeStampMillSeconds(structTimeStamp time, int millSeconds){
	time = time / MillSecondsOneDay * MillSecondsOneDay +  millSeconds;
	return time;
}

int getDaysFromThisYearForDay(int year, int month, int day){
	return getDaysFormThisYearForMonth(year, month) + (day - 1);
}

int getDaysFromStart(structTimeStamp time){
	return (int)(time / MillSecondsOneDay);
}
int getMillSeconsThatDay(structTimeStamp time){
	return (int)(time % MillSecondsOneDay);
}


BOOL isInSameWeek(structTimeStamp time1, structTimeStamp time2){
	int days1 = getDaysFromStart(time1);
	int days2 = getDaysFromStart(time2);
	return  ((4 + days1) / 7) == ((4 + days2) / 7);
}

BOOL isInSameMonth(structTimeStamp time1, structTimeStamp time2){
	int iYear1,iMonth1,iDay1;
	int iYear2,iMonth2,iDay2;

	if(!getDateOfYear(time1, &iYear1, &iMonth1, &iDay1))return FALSE;
	if(!getDateOfYear(time2, &iYear2, &iMonth2, &iDay2))return FALSE;

	return (iYear1 == iYear2) && (iMonth1 == iMonth2);
}
BOOL isTheSameDay(structTimeStamp time1, structTimeStamp time2){
	return getDaysFromStart(time1) == getDaysFromStart(time2);
}
BOOL isToday(structTimeStamp time){
	return isTheSameDay(time, getCurrentTime());
}
BOOL isThisMonth(structTimeStamp time){
	return isInSameMonth(time, getCurrentTime());
}
BOOL isThisWeek(structTimeStamp time){
	return isInSameWeek(time, getCurrentTime());
}

BOOL getDateOfYear(structTimeStamp time, int *pYear, int *pMonth, int *pDay){
	int iYear, iMonth;
	int days = getDaysFromStart(time);
	int iDaysOfYear, iDaysOfMonth;
	BOOL bLeap;

	if((!pYear) || (!pMonth) || (!pDay))return FALSE;

	iYear = BASE_YEAR;
	bLeap = isLeapYear(iYear);
	iDaysOfYear = bLeap ? 366 : 365;
	while(days >= iDaysOfYear){
		days -= iDaysOfYear;
		iYear++;
		bLeap = isLeapYear(iYear);
		iDaysOfYear = bLeap ? 366 : 365;
	}
	iMonth = 1;	
	iDaysOfMonth = getDaysOfMonth(iMonth, bLeap);
	while(days >= iDaysOfMonth){
		days -= iDaysOfMonth;
		iMonth++;
		iDaysOfMonth = getDaysOfMonth(iMonth, bLeap);
	}

	*pYear = iYear;
	*pMonth = iMonth;
	*pDay = days + 1;
	return TRUE;

}
int getYearOfTime(structTimeStamp time){
	int iYear, iMonth, iDay;

	if(!getDateOfYear(time, &iYear, &iMonth, &iDay))return 0;
	return iYear;
}
int getMonthOfTime(structTimeStamp time){
	int iYear, iMonth, iDay;

	if(!getDateOfYear(time, &iYear, &iMonth, &iDay))return 0;
	return iMonth;
}

int getDayOfTime(structTimeStamp time){
	int iYear, iMonth, iDay;

	if(!getDateOfYear(time, &iYear, &iMonth, &iDay))return 0;
	return iDay;
}

int getWeekOfTime(structTimeStamp time){
	int days = getDaysFromStart(time);
	return  (4 + (days % 7)) % 7;
}

int getHourOfTime(structTimeStamp time){
	return (getMillSeconsThatDay(time) / 1000 / 3600);
}

int getMinuteOfTime(structTimeStamp time){
	return ((getMillSeconsThatDay(time) / 1000) % 3600) / 60;
}

int getSecondOfTime(structTimeStamp time){
	return (getMillSeconsThatDay(time) / 1000) % 60;
}

int getMillSecondOfTime(structTimeStamp time){
	return (getMillSeconsThatDay(time) % 1000);
}

structTimeStamp getTimeStamp(int days, int millSecons){
	structTimeStamp timeStamp = days;

	timeStamp *= MillSecondsOneDay;
	timeStamp +=  millSecons;
	return timeStamp;
}

int getDaysForTimeStamp(int year, int month, int day){
	return getDaysFromStartForYear(year) + getDaysFromThisYearForDay(year, month, day);
}
int getMillSecondsForTimeStamp(int hour, int minute, int second, int millSecons){
	return ((hour * 60 + minute) * 60 + second) * 1000 + millSecons;
}
structTimeStamp getTimeStampByPara(int year, int month, int day, int hour, int minute, int second, int millSecons){
	int days = getDaysForTimeStamp(year, month, day);
	int millSeconds = getMillSecondsForTimeStamp(hour, minute, second, millSecons);

	return getTimeStamp(days, millSeconds);
}

int getDaysForTimeStampByString(char * pString){
	int year, month, day;

	if(!pString)return 0;
	if(strlen(pString) < 8)return 0;
	year = getYearOfDateString(pString);
	month = getMonthOfDateString(pString);
	day = getDayOfDateString(pString);
	return getDaysForTimeStamp(year, month, day);
}

int getMillSecondsForTimeStampByString(char * pString){
	int hour, minute, second;

	if(!pString)return 0;
	if(strlen(pString) < 6)return 0;
	hour = getHourOfTimeString(pString);
	minute = getMinuteOfTimeString(pString);
	second = getSecondOfTimeString(pString);
	return getMillSecondsForTimeStamp(hour, minute, second, 0);
}
structTimeStamp getTimeStampByString(char * pString){
	char temp[10];
	int days,millSeconds;

	if(!pString)return 0;
	memset(temp, 0, sizeof(temp));
	strncpy(temp, pString, 8);
	temp[8] = 0;
	days = getDaysForTimeStampByString(temp);

	strncpy(temp, pString + 8, 6);
	temp[6] = 0;
	millSeconds = getMillSecondsForTimeStampByString(temp);
	return getTimeStamp(days, millSeconds);

}
BOOL getDateStringForTimeStamp(structTimeStamp stamp, char * pBuffer, int iBufLen){
	int year, month, day;

	if(!pBuffer)return FALSE;
	if(iBufLen <= 8)return FALSE;
	if(!getDateOfYear(stamp, &year, &month, &day))return FALSE;

	sprintf(pBuffer, "%04d%02d%02d", year, month, day);
	pBuffer[8] = 0;
	return TRUE;
}
BOOL getTimeStringForTimeStamp(structTimeStamp stamp, char * pBuffer, int iBufLen){
	if(!pBuffer)return FALSE;
	if(iBufLen <= 6)return FALSE;
	sprintf(pBuffer, "%02d%02d%02d", getHourOfTime(stamp), getMinuteOfTime(stamp), getSecondOfTime(stamp));
	pBuffer[6] = 0;
	return TRUE;
}
BOOL getDateTimeStringForTimeStamp(structTimeStamp stamp, char * pBuffer, int iBufLen){
	int year, month, day;

	if(!pBuffer)return FALSE;
	if(iBufLen <= 14)return FALSE;
	if(!getDateOfYear(stamp, &year, &month, &day))return FALSE;

	sprintf(pBuffer, "%04d%02d%02d%02d%02d%02d", year, month, day, getHourOfTime(stamp), getMinuteOfTime(stamp), getSecondOfTime(stamp));
	pBuffer[14] = 0;
	return TRUE;
}

structTimeStamp addTimeStamp(structTimeStamp stamp, int millSecons){
	return stamp + millSecons;
}

int compareTimeStamp(structTimeStamp time1, structTimeStamp time2){
	if(time1 == time2)return 0;
	return (time1 > time2) ? 1 : -1;
}




structTimeStampDiff getTimeDiffByPara(int days, int millSecons){
	structTimeStampDiff diff = days;

	diff *= MillSecondsOneDay;
	diff += millSecons;
	return diff;
}

structTimeStampDiff getTimeDiff(structTimeStamp stamp1, structTimeStamp stamp2){
	return stamp1 - stamp2;

}

structTimeStampDiff addTimeStampDiff(structTimeStampDiff diff1, structTimeStampDiff diff2){
	return diff1 + diff2;
}
structTimeStampDiff addTimeStampDiffByMillSeconds(structTimeStampDiff diff, int millSecons){
	return diff + millSecons;
}
int getYearOfCurrentTime(void){
	return getYearOfTime(getCurrentTime());
}

int getMonthOfCurrentTime(void){
	return getMonthOfTime(getCurrentTime());
}

int getDayOfCurrentTime(void){
	return getDayOfTime(getCurrentTime());	
}
int getWeekOfCurrentTime(void){
	return getWeekOfTime(getCurrentTime());	
}
int getHourOfCurrentTime(void){
	return getHourOfTime(getCurrentTime());	
}
int getMinuteOfCurrentTime(void){
	return getMinuteOfTime(getCurrentTime());	
}
int getSecondOfCurrentTime(void){
	return getSecondOfTime(getCurrentTime());	
}
int getMillSecondOfCurrentTime(void){
	return (int)(getCurrentTime() % 1000);	
}
int getDayOfDiff(structTimeStampDiff diff){
	return (int)(diff / MillSecondsOneDay);
}
int getHourOfDiff(structTimeStampDiff diff){
	return (int)(diff % MillSecondsOneDay) / 1000 / 3600;
}
int getMinuteOfDiff(structTimeStampDiff diff){
	return (((int)(diff % MillSecondsOneDay) / 1000) % 3600) / 60;
}
int getSecondOfDiff(structTimeStampDiff diff){
	return (((int)(diff % MillSecondsOneDay) / 1000) % 3600) % 60;
}
int getMillSecondOfDiff(structTimeStampDiff diff){
	return (int)(diff % 1000);
}
int getTotalMillSecondsOfDiff(structTimeStampDiff diff){
	return (int)diff;
}
