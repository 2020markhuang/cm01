
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "GpsDecode.h"
#include"MyDefine.h"
#include "MyCommon.h"
#include "TrackerMain.h"
#include "MyGPS.h"
#include "MyLog.h"
#include "MainTask.h"
U16 GN_GPS_2D_fix = 0;
U16 GN_GPS_3D_fix = 0;
#ifdef SUPPORT_MY_GPS
/**********************************处理函数*********************************************/
char NMEA_Comma_Pos(char *buf,char cx)
{	 		    
	char *p=buf;
	while(cx)
	{		 
		if(*buf=='*'||*buf<' '||*buf>'z')
		return 0XFF;//遇到'*'或者非法字符,则不存在第cx个逗号
		if(*buf==',')
		{
		  cx--;
		}
		buf++;
	}
	return buf-p;	 
}
//m^n函数
//返回值:m^n次方.
int NMEA_Pow(char m,char n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}
//str转换为数字,以','或者'*'结束
//buf:数字存储区
//dx:小数点位数,返回给调用函数
//返回值:转换后的数值
int NMEA_Str2num(char *buf,char*dx)
{
	char *p=buf;
	int ires=0,fres=0;
	char ilen=0,flen=0,i;
	char mask=0;
	int res;
	while(1) //得到整数和小数的长度
	{
		if(*p=='-'){mask|=0X02;p++;}//是负数
		if(*p==','||(*p=='*'))break;//遇到结束了
		if(*p=='.'){mask|=0X01;p++;}//遇到小数点了
		else if(*p>'9'||(*p<'0'))	//有非法字符
		{	
			ilen=0;
			flen=0;
			break;
		}	
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;	//去掉负号
	for(i=0;i<ilen;i++)	//得到整数部分数据
	{  
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen>5)flen=5;	//最多取5位小数
	*dx=flen;	 		//小数点位数
	for(i=0;i<flen;i++)	//得到小数部分数据
	{  
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	} 
	res=ires*NMEA_Pow(10,flen)+fres;
	if(mask&0X02)res=-res;		   
	return res;
}	  	
void NMEA_GPRMC_Analysis( char *pBuf, int data_len)
{
	// uint8_t *p_data  ="$GNRMC,060233.00,A,2233.46052,N,11356.81244,E,0.022,,010218,,,D*6B";
	U8 *p1,dx;			 
	U8 posx;     
	U32 temp;	   
	float rs;  
	trk_comm_gps_repoter_info *pGPS = NULL;
	if ((!pBuf) || (data_len <= 0))return;
	if(1)//((g_trk_var.g_trk_content_info.gps_info.gps_state== GPS_STATUS_ON_2D_FIX)||(g_trk_var.g_trk_content_info.gps_info.gps_state== GPS_STATUS_ON_3D_FIX))
	{
		p1=(char*)strstr((const char *)pBuf,"$GPRMC");//"$GPRMC",经常有&和GPRMC分开的情况,故只判断GPRMC.
		{
			posx=NMEA_Comma_Pos(p1,1);								//得到UTC时间
			if(posx!=0XFF)
			{
				temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//得到UTC时间,去掉ms
				g_trk_var.g_trk_content_info.gps_info.date_time.hour = temp/10000;
				g_trk_var.g_trk_content_info.gps_info.date_time.minute = (temp/100)%100;
				g_trk_var.g_trk_content_info.gps_info.date_time.second = temp%100;	
				nmea_parse_time(temp, (structUTC *)&pGPS->time);
				//gps_printf("NMEA_GPRMC_Analysis111:%d\r\n",  g_trk_var.g_trk_content_info.gps_info.date_time.hour);
				//gps_printf("NMEA_GPRMC_Analysis111:%d\r\n",  g_trk_var.g_trk_content_info.gps_info.date_time.minute);
				//gps_printf("NMEA_GPRMC_Analysis111:%d\r\n",  g_trk_var.g_trk_content_info.gps_info.date_time.second);
			}	
		}
		
		{
		posx=NMEA_Comma_Pos(p1,3);								//得到纬度
		if(posx!=0XFF)
		{
		temp=NMEA_Str2num(p1+posx,&dx);		
		//gps_printf("NMEA_GPRMC_Analysis222:%d\r\n", temp);
		 g_trk_var.g_trk_content_info.gps_info.latitude = temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
		g_trk_var.g_trk_content_info.gps_info.latitude = g_trk_var.g_trk_content_info.gps_info.latitude*NMEA_Pow(10,6)+(rs*NMEA_Pow(10,6-dx))/60;//转换为° 
		gps_printf("NMEA_GPRMC_Analysis_latitude:%d\r\n", g_trk_var.g_trk_content_info.gps_info.latitude);
		}
		}	
		posx=NMEA_Comma_Pos(p1,5);								//得到经度
		if(posx!=0XFF)
		{												  
		temp=NMEA_Str2num(p1+posx,&dx);		
		//gps_printf("NMEA_GPRMC_Analysis444:%d\r\n", temp);
		g_trk_var.g_trk_content_info.gps_info.longitude = temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
		g_trk_var.g_trk_content_info.gps_info.longitude  = g_trk_var.g_trk_content_info.gps_info.longitude*NMEA_Pow(10,6)+(rs*NMEA_Pow(10,6-dx))/60;//转换为° 
		gps_printf("NMEA_GPRMC_Analysis_longitude:%d\r\n", g_trk_var.g_trk_content_info.gps_info.longitude);
		}
		
		//posx=NMEA_Comma_Pos(p1,6);								//东经还是西经
		//if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);		
		
		//posx=NMEA_Comma_Pos(p1,7);				
		//if(posx!=0XFF)
		//{	
		//g_trk_var.g_trk_content_info.gps_info.speed_km = NMEA_Str2num(p1+posx,&dx);	//速率
	        // gps_printf("NMEA_GPRMC_Analysis444:%d\r\n", g_trk_var.g_trk_content_info.gps_info.speed_km);
		//}
		
		posx=NMEA_Comma_Pos(p1,8);	
		if(posx!=0XFF)
		{
		g_trk_var.g_trk_content_info.gps_info.T_angle =  NMEA_Str2num(p1+posx,&dx);
		//gps_printf("NMEA_GPRMC_Analysis5555:%d\r\n", g_trk_var.g_trk_content_info.gps_info.T_angle);
		}
		posx=NMEA_Comma_Pos(p1,9);								//得到UTC日期
		if(posx!=0XFF)
		{
		temp=NMEA_Str2num(p1+posx,&dx);		 				//得到UTC日期
		g_trk_var.g_trk_content_info.gps_info.date_time.day=temp/10000;
		g_trk_var.g_trk_content_info.gps_info.date_time.month=(temp/100)%100;
		g_trk_var.g_trk_content_info.gps_info.date_time.year=temp%100;	
		nmea_parse_date(temp, (structUTC *)&pGPS->time);
		//gps_printf("NMEA_GPRMC_Analysis666:%d\r\n",temp);
		}
		//if(temp > 0)
		//{
		//int year = getYearOfCurrentTime();
		//printf("ZZZ:%d\r\n", year);
		//if(year < 2020)main_task_post_message(MAIN_MSG_GPS_EVENT, GPS_EVENT_TIME_CALIBRATE, pGPS->time);
		//}
		//main_task_post_message(MAIN_MSG_GPS_EVENT, GPS_EVENT_NEW_DATA, (int)pGPS);
	}
	
}

//分析GPVTG信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPVTG_Analysis(char *pBuf, int data_len)
{
	U8 *p1,dx;			 
	U8 posx;   
	//trk_comm_gps_repoter_info *pGPS = NULL;
	if ((!pBuf) || (data_len <= 0))return;
	p1=(char*)strstr((const char *)pBuf,"$GPVTG");
    	posx=NMEA_Comma_Pos(p1,7);								//得到地面速率
    	if(posx!=0XFF)
    	{
    		g_trk_var.g_trk_content_info.gps_info.speed_km=NMEA_Str2num(p1+posx,&dx);
    		if(dx<3)g_trk_var.g_trk_content_info.gps_info.speed_km*=NMEA_Pow(10,3-dx);	 	 		//确保扩大1000倍
    		//gps_printf("NMEA_GPVTG_Analysis11111:%d\r\n", g_trk_var.g_trk_content_info.gps_info.speed_km);
		//main_task_post_message(MAIN_MSG_GPS_EVENT, GPS_EVENT_NEW_DATA, (int)pGPS);
    	}
}  
//分析GPGGA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGGA_Analysis(char *pBuf, int data_len)
{	
	U8 *p1,dx;			 
	U8 posx; 
	int gps_states;
	//trk_comm_gps_repoter_info *pGPS = NULL;
	if ((!pBuf) || (data_len <= 0))return;
	p1=(char*)strstr((const char *)pBuf,"$GPGGA");
    posx=NMEA_Comma_Pos(p1,6);	
	if(posx!=0XFF) gps_states = NMEA_Str2num(p1+posx,&dx);//得到GPS状态
    //gps_printf("NMEA_GPGGA_Analysis111:%d\r\n", gps_states);
    posx=NMEA_Comma_Pos(p1,7);	//得到用于定位的卫星数
    if(posx!=0XFF) g_trk_var.g_trk_content_info.gps_info.satellite_using_num = NMEA_Str2num(p1+posx,&dx); 
	//gps_printf("NMEA_GPGGA_Analysis222:%d\r\n", g_trk_var.g_trk_content_info.gps_info.satellite_using_num);
    posx=NMEA_Comma_Pos(p1,9);								//得到海拔高度
    if(posx!=0XFF)g_trk_var.g_trk_content_info.gps_info.altitude=NMEA_Str2num(p1+posx,&dx);  
	//main_task_post_message(MAIN_MSG_GPS_EVENT, GPS_EVENT_NEW_DATA, (int)pGPS);
}
//分析GPGSA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGSA_Analysis(char *pBuf, int data_len)
{
	U8 *p1,dx;			 
	U8 posx; 
	//U8 i; 
	//trk_comm_gps_repoter_info *pGPS = NULL;
	if ((!pBuf) || (data_len <= 0))return;
	p1=(char*)strstr((const char *)pBuf,"$GPGSA");
    	posx=NMEA_Comma_Pos(p1,2);								//得到定位类型
    	if(posx!=0XFF) g_trk_var.g_trk_content_info.gps_info.gps_state=NMEA_Str2num(p1+posx,&dx);	
	gps_printf("GSAgps_state:%d\r\n",  g_trk_var.g_trk_content_info.gps_info.gps_state);
	if( g_trk_var.g_trk_content_info.gps_info.gps_state == 1)
	{
		GN_GPS_2D_fix = 0;
		GN_GPS_3D_fix = 0;     						
		g_trk_var.g_trk_content_info.gps_info.altitude = 0;
		g_trk_var.g_trk_content_info.gps_info.speed_km = 0;
		g_trk_var.g_trk_content_info.gps_info.latitude = 0;   
		g_trk_var.g_trk_content_info.gps_info.longitude = 0; 
		g_trk_var.g_trk_content_info.gps_info.gps_state= GPS_STATUS_ON_SEARCHING;
	}
	else if( g_trk_var.g_trk_content_info.gps_info.gps_state == 2)
	{
		if(GN_GPS_2D_fix < 0xfff0)
			GN_GPS_2D_fix++;
		else
			GN_GPS_2D_fix = 0;
		g_trk_var.g_trk_content_info.gps_info.gps_state= GPS_STATUS_ON_2D_FIX;
		//gps_printf("GN_GPS_2D_fix:%d\r\n", GN_GPS_2D_fix);
		//main_task_post_message(MAIN_MSG_GPS_EVENT, GPS_EVENT_LOCATED, 0);
		
	}
	else if( g_trk_var.g_trk_content_info.gps_info.gps_state == 3)
	{
		if(GN_GPS_3D_fix < 0xfff0)
			GN_GPS_3D_fix++;
		else
			GN_GPS_3D_fix = 0;
		 g_trk_var.g_trk_content_info.gps_info.gps_state= GPS_STATUS_ON_3D_FIX;
		// gps_printf("GN_GPS_3D_fix:%d\r\n", GN_GPS_3D_fix);
		 //main_task_post_message(MAIN_MSG_GPS_EVENT, GPS_EVENT_LOCATED, 0);
	}
	else
	{
		GN_GPS_2D_fix = 0;
		GN_GPS_3D_fix = 0;
		g_trk_var.g_trk_content_info.gps_info.altitude = 0;
		g_trk_var.g_trk_content_info.gps_info.speed_km = 0;
		g_trk_var.g_trk_content_info.gps_info.latitude = 0;   
		g_trk_var.g_trk_content_info.gps_info.longitude = 0;
		g_trk_var.g_trk_content_info.gps_info.gps_state = GPS_STATUS_ON_SEARCHING;
	}
	posx=NMEA_Comma_Pos(p1,15);								//得到PDOP位置精度因子
	if(posx!=0XFF) g_trk_var.g_trk_content_info.gps_info.pdop = NMEA_Str2num(p1+posx,&dx);  
	posx=NMEA_Comma_Pos(p1,16);								//得到HDOP位置精度因子
	if(posx!=0XFF)  g_trk_var.g_trk_content_info.gps_info.hdop = NMEA_Str2num(p1+posx,&dx);  
	//gps_printf("GSAhdop:%d\r\n", g_trk_var.g_trk_content_info.gps_info.hdop);
	posx=NMEA_Comma_Pos(p1,17);								//得到VDOP位置精度因子
	if(posx!=0XFF)  g_trk_var.g_trk_content_info.gps_info.vdop = NMEA_Str2num(p1+posx,&dx);  
	//main_task_post_message(MAIN_MSG_GPS_EVENT, GPS_EVENT_NEW_DATA, (int)pGPS);
}

//分析GPGSV信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
//每条GSV语句最多可以显示4颗卫星的信息。其他卫星信息将在下一序列的NMEA0183语句中输出
//卫星号 01-32， 信号强度 0 - 99
void NMEA_GPGSV_Analysis(char *pBuf, int data_len)
{
	U8 *p,*p1,dx;
	U8 len,i,j,slx=0;
	U8 posx;   	 
	p=pBuf;
	p1=(uint8_t*)strstr((const char *)p,"$GPGSV");
   	 if(p1!=NULL)
    	{
		len=p1[7]-'0';								//得到GPGSV的条数
		posx=NMEA_Comma_Pos(p1,3); 					//得到可见卫星总数
		if(posx!=0XFF) g_trk_var.g_trk_content_info.gps_info.satellite_visable_num=NMEA_Str2num(p1+posx,&dx);	
		//gps_printf("NMEA_GPGSV_Analysis num=%d\r\n", g_trk_var.g_trk_content_info.gps_info.satellite_visable_num);
		//if(posx!=0XFF)gpsx->svnum=NMEA_Str2num(p1+posx,&dx);
		for(i=0;i<len;i++)
		{	 
			p1=(U8*)strstr((const char *)p,"$GPGSV");  
			for(j=0;j<4;j++)
			{	  
				posx=NMEA_Comma_Pos(p1,4+j*4);
				if(posx!=0XFF)g_trk_var.g_trk_content_info.gps_info.prn_info[slx].num=NMEA_Str2num(p1+posx,&dx);	//得到卫星编号
				else break; 
				posx=NMEA_Comma_Pos(p1,5+j*4);
				if(posx!=0XFF)g_trk_var.g_trk_content_info.gps_info.prn_info[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//得到卫星仰角 
				else break;
				posx=NMEA_Comma_Pos(p1,6+j*4);
				if(posx!=0XFF)g_trk_var.g_trk_content_info.gps_info.prn_info[slx].azideg=NMEA_Str2num(p1+posx,&dx);//得到卫星方位角
				else break; 
				posx=NMEA_Comma_Pos(p1,7+j*4);
				if(posx!=0XFF)g_trk_var.g_trk_content_info.gps_info.prn_info[slx].sn=NMEA_Str2num(p1+posx,&dx);	//得到卫星信噪比
				else break;
				//gps_printf("NMEA_GPGSV_Analysisg id=%d--------signal:%d\r\n", g_trk_var.g_trk_content_info.gps_info.prn_info[slx].num,g_trk_var.g_trk_content_info.gps_info.prn_info[slx].sn);
				slx++;
			}   
	 		p=p1+1;//切换到下一个GPGSV信息
		}
	}
}

#endif
