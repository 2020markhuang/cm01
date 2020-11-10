#ifndef WATCH_DATETIME_H
#define WATCH_DATETIME_H

#include "MyDefine.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
typedef  I64 structTimeStamp;
typedef  I64 structTimeStampDiff;

//��׼ʱ�� 1970-1-1 00:00:00
//8λ�ַ�������  (4λ��2λ��2λ��)����20180525
//6λ�ַ���ʱ��	 (2λСʱ2λ����2λ����)����142023

/*
	��������:��ȡ��ǰ��������ʱ��
	����˵��:��
	�� �� ֵ:����
	������Ա:����ǿ
*/
structTimeStamp getCurrentTime(void);
/*
	��������:��ȡ��ǰ����ʱ��
	����˵��:��
	�� �� ֵ:����
	������Ա:����ǿ
*/
structTimeStamp getCurrentLocalTime(void);

/*
	��������:ʱ���Ƿ�Ϊ��
	����˵��:stamp	ʱ��ֵ
	�� �� ֵ:����
	������Ա:����ǿ
*/
BOOL isTimeStampEmpty(structTimeStamp stamp);
/*
	��������:ʱ����Ƿ�Ϊ��
	����˵��:diff	ʱ���ֵ
	�� �� ֵ:����
	������Ա:����ǿ
*/
BOOL isTimeStampDiffEmpty(structTimeStampDiff diff);
/*
	��������:ʱ���Ƿ�����
	����˵��:year	4λ���
	�� �� ֵ:����
	������Ա:����ǿ
*/
BOOL isLeapYear(int year);
/*
	��������:���ʱ��
	����˵��:pStamp(out)	ʱ��
	�� �� ֵ:��
	������Ա:����ǿ
*/
void resetTimeStamp(structTimeStamp * pStamp);
/*
	��������:��ȡָ���·ݵ�����
	����˵��:month	�·�(1~12)
			 bLeap		�Ƿ���Ϊ����
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getDaysOfMonth(int month, BOOL bLeap);
/*
	��������:�ӻ�׼ʱ�䵽ָ�����1��1��֮�������
	����˵��:year		ָ�����
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getDaysFromStartForYear(int year);
/*
	��������:��ָ����1��1�յ�����ָ����1��֮�������
	����˵��:year		ָ�����
			 month		ָ���·�
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getDaysFormThisYearForMonth(int year, int month);
/*
	��������:��ָ����1��1�յ�ָ�����ڵ�����
	����˵��:year		ָ�����
			 month		ָ���·�
			 day		ָ������
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getDaysFromThisYearForDay(int year, int month, int day);
/*
	��������:�ӻ�׼ʱ�䵽ָ��ʱ��֮������
	����˵��:time	ָ��ʱ��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getDaysFromStart(structTimeStamp time);
/*
	��������:��ָ��ʱ���ڣ���0�㵽����ʾʱ��ĺ�����
	����˵��:time	ָ��ʱ��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getMillSeconsThatDay(structTimeStamp time);
/*
	��������:����ָ��ʱ�����������
	����˵��:time	ָ��ʱ��
			 days	�µ���������
	�� �� ֵ:�µ�ʱ��
	������Ա:����ǿ
*/
structTimeStamp setTimeStampDays(structTimeStamp time, int days);
/*
	��������:����ָ��ʱ��ĺ��벿��
	����˵��:time	ָ��ʱ��
			 millSeconds ���벿��
	�� �� ֵ:�µ�ʱ��
	������Ա:����ǿ
*/
structTimeStamp setTimeStampMillSeconds(structTimeStamp time, int millSeconds);

/*
	��������:�Ƚ�����ʱ���Ƿ���ͬһ��
	����˵��:time1	ָ��ʱ��1
			 time2  ָ��ʱ��2
	�� �� ֵ:����
	������Ա:����ǿ
*/
BOOL isInSameWeek(structTimeStamp time1, structTimeStamp time2);
/*
	��������:�Ƚ�����ʱ���Ƿ���ͬһ��
	����˵��:time1	ָ��ʱ��1
			 time2  ָ��ʱ��2
	�� �� ֵ:����
	������Ա:����ǿ
*/
BOOL isInSameMonth(structTimeStamp time1, structTimeStamp time2);
/*
	��������:�Ƚ�����ʱ���Ƿ���ͬһ��
	����˵��:time1	ָ��ʱ��1
			 time2  ָ��ʱ��2
	�� �� ֵ:����
	������Ա:����ǿ
*/
BOOL isTheSameDay(structTimeStamp time1, structTimeStamp time2);
/*
	��������:ָ��ʱ���Ƿ�Ϊ����
	����˵��:time	ָ��ʱ��
	�� �� ֵ:����
	������Ա:����ǿ
*/
BOOL isToday(structTimeStamp time);
/*
	��������:ָ��ʱ���Ƿ�Ϊ����
	����˵��:time	ָ��ʱ��
	�� �� ֵ:����
	������Ա:����ǿ
*/
BOOL isThisWeek(structTimeStamp time);
/*
	��������:ָ��ʱ���Ƿ�Ϊ����
	����˵��:time	ָ��ʱ��
	�� �� ֵ:����
	������Ա:����ǿ
*/
BOOL isThisMonth(structTimeStamp time);
/*
	��������:��ȡָ��ʱ���������
	����˵��:time	ָ��ʱ��
			 pYear(out)	��
			 pMonth(out) ��
			 pDay(out)	��
	�� �� ֵ:�ɹ����
	������Ա:����ǿ
*/
BOOL getDateOfYear(structTimeStamp time, int *pYear, int *pMonth, int *pDay);
/*
	��������:��ȡָ��ʱ�����
	����˵��:time	ָ��ʱ��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getYearOfTime(structTimeStamp time);
/*
	��������:��ȡָ��ʱ�����
	����˵��:time	ָ��ʱ��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getMonthOfTime(structTimeStamp time);
/*
	��������:��ȡָ��ʱ�����
	����˵��:time	ָ��ʱ��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getDayOfTime(structTimeStamp time);
/*
	��������:��ȡָ��ʱ������ڼ�
	����˵��:time	ָ��ʱ��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getWeekOfTime(structTimeStamp time);
/*
	��������:��ȡָ��ʱ���Сʱ
	����˵��:time	ָ��ʱ��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getHourOfTime(structTimeStamp time);
/*
	��������:��ȡָ��ʱ��ķ���
	����˵��:time	ָ��ʱ��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getMinuteOfTime(structTimeStamp time);
/*
	��������:��ȡָ��ʱ�����
	����˵��:time	ָ��ʱ��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getSecondOfTime(structTimeStamp time);
/*
	��������:��ȡָ��ʱ��ĺ���(0~999)
	����˵��:time	ָ��ʱ��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getMillSecondOfTime(structTimeStamp time);
/*
	��������:���������ֺͺ��벿�ֺϳ�ʱ��
	����˵��:days	����(�ӻ�׼ʱ�����)
			 millSecons	��0�����	
	�� �� ֵ:����
	������Ա:����ǿ
*/
structTimeStamp getTimeStamp(int days, int millSecons);
/*
	��������:����ӻ�׼ʱ�䵽ָ�������յ�����
	����˵��:year	��
			 month	��
			 day	��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getDaysForTimeStamp(int year, int month, int day);
/*
	��������:�����0�㵽ָ��ʱ��ĺ�����
	����˵��:hour	Сʱ
			 minute	��
			 second	��
			 millSecons	����
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getMillSecondsForTimeStamp(int hour, int minute, int second, int millSecons);
/*
	��������:����ָ�������ϳɱ�׼ʱ��
	����˵��:year	��
			 month	��
			 day	��
			 hour	Сʱ
			 minute	��
			 second	��
			 millSecons	����
	�� �� ֵ:����
	������Ա:����ǿ
*/
structTimeStamp getTimeStampByPara(int year, int month, int day, int hour, int minute, int second, int millSecons);
/*
	��������:����8λ�ַ������ڻ�ȡ��������
	����˵��:pString(in)	8λ�ַ�������
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getDaysForTimeStampByString(char * pString);
/*
	��������:����6λ�ַ���ʱ���ȡ���벿��
	����˵��:pString(in)	6λ�ַ���ʱ��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getMillSecondsForTimeStampByString(char * pString);
/*
	��������:����14λ�ַ�������ʱ���ȡ��׼ʱ��
	����˵��:pString(in)	8λ�ַ�������6λ�ַ���ʱ��
	�� �� ֵ:����
	������Ա:����ǿ
*/
structTimeStamp getTimeStampByString(char * pString);
/*
	��������:��ȡָ��ʱ���8λ���ڲ���
	����˵��:stamp(in)	ָ��ʱ��
			 pBuffer(out)	8λ�����ַ���
			 iBufLen(in)	pBuffer����,�������8
	�� �� ֵ:�ɹ����
	������Ա:����ǿ
*/
BOOL getDateStringForTimeStamp(structTimeStamp stamp, char * pBuffer, int iBufLen);
/*
	��������:��ȡָ��ʱ���6λʱ�䲿��
	����˵��:stamp(in)	ָ��ʱ��
			 pBuffer(out)	6λʱ���ַ���
			 iBufLen(in)	pBuffer����,�������6
	�� �� ֵ:�ɹ����
	������Ա:����ǿ
*/
BOOL getTimeStringForTimeStamp(structTimeStamp stamp, char * pBuffer, int iBufLen);
/*
	��������:��ȡָ��ʱ���14λ����ʱ��
	����˵��:stamp(in)	ָ��ʱ��
			 pBuffer(out)	14λ����ʱ��
			 iBufLen(in)	pBuffer����,�������14
	�� �� ֵ:�ɹ����
	������Ա:����ǿ
*/
BOOL getDateTimeStringForTimeStamp(structTimeStamp stamp, char * pBuffer, int iBufLen);
/*
	��������:��ָ��ʱ��������ƫ��ʱ�䣬��ȡ��ʱ��
	����˵��:stamp		ָ��ʱ��
			 millSecons	ƫ�ƺ�����(�з���)
	�� �� ֵ:����
	������Ա:����ǿ
*/
structTimeStamp addTimeStamp(structTimeStamp stamp, int millSecons);
/*
	��������:�Ƚ�ʱ���С
	����˵��:time1		ָ��ʱ��1
			 time2		ָ��ʱ��2
	�� �� ֵ:1   time1 > time2
			 0	 time1 = time2
			 -1  time1 < time2
	������Ա:����ǿ
*/
int compareTimeStamp(structTimeStamp time1, structTimeStamp time2);
/*
	��������:�����ڲ��ֺͺ��벿��,�γ�ʱ���
	����˵��:days		����
			 millSecons	������
	�� �� ֵ:����
	������Ա:����ǿ
*/
structTimeStampDiff getTimeDiffByPara(int days, int millSecons);
/*
	��������:��ȡ����ʱ���ʱ���
	����˵��:time1		ָ��ʱ��1
			 time2		ָ��ʱ��2
	�� �� ֵ:����
	������Ա:����ǿ
*/
structTimeStampDiff getTimeDiff(structTimeStamp stamp1, structTimeStamp stamp2);
/*
	��������:����ʱ��������,��ȡ�µ�ʱ���
	����˵��:diff1		ָ��ʱ���1
			 diff2		ָ��ʱ���2
	�� �� ֵ:����
	������Ա:����ǿ
*/
structTimeStampDiff addTimeStampDiff(structTimeStampDiff diff1, structTimeStampDiff diff2);
/*
	��������:ʱ���ͺ���ƫ�Ƶ���,��ȡ�µ�ʱ���
	����˵��:diff		ָ��ʱ���
			 millSecons	����ƫ�Ʋ�
	�� �� ֵ:����
	������Ա:����ǿ
*/
structTimeStampDiff addTimeStampDiffByMillSeconds(structTimeStampDiff diff, int millSecons);
/*
	��������:��ȡ��ǰ��(��1970�꿪ʼ)
	����˵��:��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getYearOfCurrentTime(void);
/*
	��������:��ȡ��ǰ�·�(1~12)
	����˵��:��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getMonthOfCurrentTime(void);
/*
	��������:��ȡ��ǰ��(0~30)
	����˵��:��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getDayOfCurrentTime(void);
/*
	��������:��ȡ��ǰ����(0~6)
	����˵��:��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getWeekOfCurrentTime(void);
/*
	��������:��ȡ��ǰСʱ(0~59)
	����˵��:��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getHourOfCurrentTime(void);
/*
	��������:��ȡ��ǰ����(0~59)
	����˵��:��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getMinuteOfCurrentTime(void);
/*
	��������:��ȡ��ǰ��(0~59)
	����˵��:��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getSecondOfCurrentTime(void);
/*
	��������:��ȡ������(0~999)
	����˵��:��
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getMillSecondOfCurrentTime(void);
/*
	��������:��ȡʱ��������
	����˵��:diff		ָ��ʱ���
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getDayOfDiff(structTimeStampDiff diff);
/*
	��������:��ȡʱ����Сʱ(0~23)
	����˵��:diff		ָ��ʱ���
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getHourOfDiff(structTimeStampDiff diff);
/*
	��������:��ȡʱ���ķ���(0~59)
	����˵��:diff		ָ��ʱ���
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getMinuteOfDiff(structTimeStampDiff diff);
/*
	��������:��ȡʱ������(0~59)
	����˵��:diff		ָ��ʱ���
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getSecondOfDiff(structTimeStampDiff diff);
/*
	��������:��ȡʱ���ĺ���(0~59)
	����˵��:diff		ָ��ʱ���
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getMillSecondOfDiff(structTimeStampDiff diff);
/*
	��������:��ȡʱ���ĺ��벿��
	����˵��:diff		ָ��ʱ���
	�� �� ֵ:����
	������Ա:����ǿ
*/
int getTotalMillSecondsOfDiff(structTimeStampDiff diff);


#if defined(__cplusplus)
}
#endif 

#endif //WATCH_DATETIME_H



