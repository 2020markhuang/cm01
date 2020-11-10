#ifndef WATCH_DATETIME_H
#define WATCH_DATETIME_H

#include "MyDefine.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
typedef  I64 structTimeStamp;
typedef  I64 structTimeStampDiff;

//基准时间 1970-1-1 00:00:00
//8位字符串日期  (4位年2位月2位日)例如20180525
//6位字符串时间	 (2位小时2位分钟2位秒钟)例如142023

/*
	功能作用:获取当前格林威治时间
	参数说明:无
	返 回 值:如题
	开发人员:付华强
*/
structTimeStamp getCurrentTime(void);
/*
	功能作用:获取当前当地时间
	参数说明:无
	返 回 值:如题
	开发人员:付华强
*/
structTimeStamp getCurrentLocalTime(void);

/*
	功能作用:时间是否为空
	参数说明:stamp	时间值
	返 回 值:如题
	开发人员:付华强
*/
BOOL isTimeStampEmpty(structTimeStamp stamp);
/*
	功能作用:时间差是否为空
	参数说明:diff	时间差值
	返 回 值:如题
	开发人员:付华强
*/
BOOL isTimeStampDiffEmpty(structTimeStampDiff diff);
/*
	功能作用:时间是否闰年
	参数说明:year	4位年份
	返 回 值:如题
	开发人员:付华强
*/
BOOL isLeapYear(int year);
/*
	功能作用:清空时间
	参数说明:pStamp(out)	时间
	返 回 值:无
	开发人员:付华强
*/
void resetTimeStamp(structTimeStamp * pStamp);
/*
	功能作用:获取指定月份的天数
	参数说明:month	月份(1~12)
			 bLeap		是否当年为闰年
	返 回 值:如题
	开发人员:付华强
*/
int getDaysOfMonth(int month, BOOL bLeap);
/*
	功能作用:从基准时间到指定年份1月1日之间的天数
	参数说明:year		指定年份
	返 回 值:如题
	开发人员:付华强
*/
int getDaysFromStartForYear(int year);
/*
	功能作用:从指定年1月1日到该年指定月1日之间的天数
	参数说明:year		指定年份
			 month		指定月份
	返 回 值:如题
	开发人员:付华强
*/
int getDaysFormThisYearForMonth(int year, int month);
/*
	功能作用:从指定年1月1日到指定日期的天数
	参数说明:year		指定年份
			 month		指定月份
			 day		指定日期
	返 回 值:如题
	开发人员:付华强
*/
int getDaysFromThisYearForDay(int year, int month, int day);
/*
	功能作用:从基准时间到指定时间之间天数
	参数说明:time	指定时间
	返 回 值:如题
	开发人员:付华强
*/
int getDaysFromStart(structTimeStamp time);
/*
	功能作用:在指定时间内，从0点到它表示时间的毫秒数
	参数说明:time	指定时间
	返 回 值:如题
	开发人员:付华强
*/
int getMillSeconsThatDay(structTimeStamp time);
/*
	功能作用:更新指定时间的天数部分
	参数说明:time	指定时间
			 days	新的天数部分
	返 回 值:新的时间
	开发人员:付华强
*/
structTimeStamp setTimeStampDays(structTimeStamp time, int days);
/*
	功能作用:更新指定时间的毫秒部分
	参数说明:time	指定时间
			 millSeconds 毫秒部分
	返 回 值:新的时间
	开发人员:付华强
*/
structTimeStamp setTimeStampMillSeconds(structTimeStamp time, int millSeconds);

/*
	功能作用:比较两个时间是否在同一周
	参数说明:time1	指定时间1
			 time2  指定时间2
	返 回 值:如题
	开发人员:付华强
*/
BOOL isInSameWeek(structTimeStamp time1, structTimeStamp time2);
/*
	功能作用:比较两个时间是否在同一月
	参数说明:time1	指定时间1
			 time2  指定时间2
	返 回 值:如题
	开发人员:付华强
*/
BOOL isInSameMonth(structTimeStamp time1, structTimeStamp time2);
/*
	功能作用:比较两个时间是否在同一天
	参数说明:time1	指定时间1
			 time2  指定时间2
	返 回 值:如题
	开发人员:付华强
*/
BOOL isTheSameDay(structTimeStamp time1, structTimeStamp time2);
/*
	功能作用:指定时间是否为今天
	参数说明:time	指定时间
	返 回 值:如题
	开发人员:付华强
*/
BOOL isToday(structTimeStamp time);
/*
	功能作用:指定时间是否为本周
	参数说明:time	指定时间
	返 回 值:如题
	开发人员:付华强
*/
BOOL isThisWeek(structTimeStamp time);
/*
	功能作用:指定时间是否为本月
	参数说明:time	指定时间
	返 回 值:如题
	开发人员:付华强
*/
BOOL isThisMonth(structTimeStamp time);
/*
	功能作用:获取指定时间的年月日
	参数说明:time	指定时间
			 pYear(out)	年
			 pMonth(out) 月
			 pDay(out)	日
	返 回 值:成功与否
	开发人员:付华强
*/
BOOL getDateOfYear(structTimeStamp time, int *pYear, int *pMonth, int *pDay);
/*
	功能作用:获取指定时间的年
	参数说明:time	指定时间
	返 回 值:如题
	开发人员:付华强
*/
int getYearOfTime(structTimeStamp time);
/*
	功能作用:获取指定时间的月
	参数说明:time	指定时间
	返 回 值:如题
	开发人员:付华强
*/
int getMonthOfTime(structTimeStamp time);
/*
	功能作用:获取指定时间的日
	参数说明:time	指定时间
	返 回 值:如题
	开发人员:付华强
*/
int getDayOfTime(structTimeStamp time);
/*
	功能作用:获取指定时间的星期几
	参数说明:time	指定时间
	返 回 值:如题
	开发人员:付华强
*/
int getWeekOfTime(structTimeStamp time);
/*
	功能作用:获取指定时间的小时
	参数说明:time	指定时间
	返 回 值:如题
	开发人员:付华强
*/
int getHourOfTime(structTimeStamp time);
/*
	功能作用:获取指定时间的分钟
	参数说明:time	指定时间
	返 回 值:如题
	开发人员:付华强
*/
int getMinuteOfTime(structTimeStamp time);
/*
	功能作用:获取指定时间的秒
	参数说明:time	指定时间
	返 回 值:如题
	开发人员:付华强
*/
int getSecondOfTime(structTimeStamp time);
/*
	功能作用:获取指定时间的毫秒(0~999)
	参数说明:time	指定时间
	返 回 值:如题
	开发人员:付华强
*/
int getMillSecondOfTime(structTimeStamp time);
/*
	功能作用:由天数部分和毫秒部分合成时间
	参数说明:days	天数(从基准时间计算)
			 millSecons	从0点计算	
	返 回 值:如题
	开发人员:付华强
*/
structTimeStamp getTimeStamp(int days, int millSecons);
/*
	功能作用:计算从基准时间到指定年月日的天数
	参数说明:year	年
			 month	月
			 day	日
	返 回 值:如题
	开发人员:付华强
*/
int getDaysForTimeStamp(int year, int month, int day);
/*
	功能作用:计算从0点到指定时间的毫秒数
	参数说明:hour	小时
			 minute	分
			 second	秒
			 millSecons	毫秒
	返 回 值:如题
	开发人员:付华强
*/
int getMillSecondsForTimeStamp(int hour, int minute, int second, int millSecons);
/*
	功能作用:根据指定参数合成标准时间
	参数说明:year	年
			 month	月
			 day	日
			 hour	小时
			 minute	分
			 second	秒
			 millSecons	毫秒
	返 回 值:如题
	开发人员:付华强
*/
structTimeStamp getTimeStampByPara(int year, int month, int day, int hour, int minute, int second, int millSecons);
/*
	功能作用:根据8位字符串日期获取天数部分
	参数说明:pString(in)	8位字符串日期
	返 回 值:如题
	开发人员:付华强
*/
int getDaysForTimeStampByString(char * pString);
/*
	功能作用:根据6位字符串时间获取毫秒部分
	参数说明:pString(in)	6位字符串时间
	返 回 值:如题
	开发人员:付华强
*/
int getMillSecondsForTimeStampByString(char * pString);
/*
	功能作用:根据14位字符串完整时间获取标准时间
	参数说明:pString(in)	8位字符串日期6位字符串时间
	返 回 值:如题
	开发人员:付华强
*/
structTimeStamp getTimeStampByString(char * pString);
/*
	功能作用:获取指定时间的8位日期部分
	参数说明:stamp(in)	指定时间
			 pBuffer(out)	8位日期字符串
			 iBufLen(in)	pBuffer长度,必须大于8
	返 回 值:成功与否
	开发人员:付华强
*/
BOOL getDateStringForTimeStamp(structTimeStamp stamp, char * pBuffer, int iBufLen);
/*
	功能作用:获取指定时间的6位时间部分
	参数说明:stamp(in)	指定时间
			 pBuffer(out)	6位时间字符串
			 iBufLen(in)	pBuffer长度,必须大于6
	返 回 值:成功与否
	开发人员:付华强
*/
BOOL getTimeStringForTimeStamp(structTimeStamp stamp, char * pBuffer, int iBufLen);
/*
	功能作用:获取指定时间的14位完整时间
	参数说明:stamp(in)	指定时间
			 pBuffer(out)	14位完整时间
			 iBufLen(in)	pBuffer长度,必须大于14
	返 回 值:成功与否
	开发人员:付华强
*/
BOOL getDateTimeStringForTimeStamp(structTimeStamp stamp, char * pBuffer, int iBufLen);
/*
	功能作用:在指定时间上增加偏移时间，获取新时间
	参数说明:stamp		指定时间
			 millSecons	偏移毫秒数(有符号)
	返 回 值:如题
	开发人员:付华强
*/
structTimeStamp addTimeStamp(structTimeStamp stamp, int millSecons);
/*
	功能作用:比较时间大小
	参数说明:time1		指定时间1
			 time2		指定时间2
	返 回 值:1   time1 > time2
			 0	 time1 = time2
			 -1  time1 < time2
	开发人员:付华强
*/
int compareTimeStamp(structTimeStamp time1, structTimeStamp time2);
/*
	功能作用:由日期部分和毫秒部分,形成时间差
	参数说明:days		天数
			 millSecons	毫秒数
	返 回 值:如题
	开发人员:付华强
*/
structTimeStampDiff getTimeDiffByPara(int days, int millSecons);
/*
	功能作用:获取两个时间的时间差
	参数说明:time1		指定时间1
			 time2		指定时间2
	返 回 值:如题
	开发人员:付华强
*/
structTimeStampDiff getTimeDiff(structTimeStamp stamp1, structTimeStamp stamp2);
/*
	功能作用:两个时间差相叠加,获取新的时间差
	参数说明:diff1		指定时间差1
			 diff2		指定时间差2
	返 回 值:如题
	开发人员:付华强
*/
structTimeStampDiff addTimeStampDiff(structTimeStampDiff diff1, structTimeStampDiff diff2);
/*
	功能作用:时间差和毫秒偏移叠加,获取新的时间差
	参数说明:diff		指定时间差
			 millSecons	毫秒偏移差
	返 回 值:如题
	开发人员:付华强
*/
structTimeStampDiff addTimeStampDiffByMillSeconds(structTimeStampDiff diff, int millSecons);
/*
	功能作用:获取当前年(从1970年开始)
	参数说明:无
	返 回 值:如题
	开发人员:付华强
*/
int getYearOfCurrentTime(void);
/*
	功能作用:获取当前月份(1~12)
	参数说明:无
	返 回 值:如题
	开发人员:付华强
*/
int getMonthOfCurrentTime(void);
/*
	功能作用:获取当前日(0~30)
	参数说明:无
	返 回 值:如题
	开发人员:付华强
*/
int getDayOfCurrentTime(void);
/*
	功能作用:获取当前星期(0~6)
	参数说明:无
	返 回 值:如题
	开发人员:付华强
*/
int getWeekOfCurrentTime(void);
/*
	功能作用:获取当前小时(0~59)
	参数说明:无
	返 回 值:如题
	开发人员:付华强
*/
int getHourOfCurrentTime(void);
/*
	功能作用:获取当前分钟(0~59)
	参数说明:无
	返 回 值:如题
	开发人员:付华强
*/
int getMinuteOfCurrentTime(void);
/*
	功能作用:获取当前秒(0~59)
	参数说明:无
	返 回 值:如题
	开发人员:付华强
*/
int getSecondOfCurrentTime(void);
/*
	功能作用:获取当毫秒(0~999)
	参数说明:无
	返 回 值:如题
	开发人员:付华强
*/
int getMillSecondOfCurrentTime(void);
/*
	功能作用:获取时间差的天数
	参数说明:diff		指定时间差
	返 回 值:如题
	开发人员:付华强
*/
int getDayOfDiff(structTimeStampDiff diff);
/*
	功能作用:获取时间差的小时(0~23)
	参数说明:diff		指定时间差
	返 回 值:如题
	开发人员:付华强
*/
int getHourOfDiff(structTimeStampDiff diff);
/*
	功能作用:获取时间差的分钟(0~59)
	参数说明:diff		指定时间差
	返 回 值:如题
	开发人员:付华强
*/
int getMinuteOfDiff(structTimeStampDiff diff);
/*
	功能作用:获取时间差的秒(0~59)
	参数说明:diff		指定时间差
	返 回 值:如题
	开发人员:付华强
*/
int getSecondOfDiff(structTimeStampDiff diff);
/*
	功能作用:获取时间差的毫秒(0~59)
	参数说明:diff		指定时间差
	返 回 值:如题
	开发人员:付华强
*/
int getMillSecondOfDiff(structTimeStampDiff diff);
/*
	功能作用:获取时间差的毫秒部分
	参数说明:diff		指定时间差
	返 回 值:如题
	开发人员:付华强
*/
int getTotalMillSecondsOfDiff(structTimeStampDiff diff);


#if defined(__cplusplus)
}
#endif 

#endif //WATCH_DATETIME_H



