
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
/**********************************������*********************************************/
char NMEA_Comma_Pos(char *buf,char cx)
{	 		    
	char *p=buf;
	while(cx)
	{		 
		if(*buf=='*'||*buf<' '||*buf>'z')
		return 0XFF;//����'*'���߷Ƿ��ַ�,�򲻴��ڵ�cx������
		if(*buf==',')
		{
		  cx--;
		}
		buf++;
	}
	return buf-p;	 
}
//m^n����
//����ֵ:m^n�η�.
int NMEA_Pow(char m,char n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}
//strת��Ϊ����,��','����'*'����
//buf:���ִ洢��
//dx:С����λ��,���ظ����ú���
//����ֵ:ת�������ֵ
int NMEA_Str2num(char *buf,char*dx)
{
	char *p=buf;
	int ires=0,fres=0;
	char ilen=0,flen=0,i;
	char mask=0;
	int res;
	while(1) //�õ�������С���ĳ���
	{
		if(*p=='-'){mask|=0X02;p++;}//�Ǹ���
		if(*p==','||(*p=='*'))break;//����������
		if(*p=='.'){mask|=0X01;p++;}//����С������
		else if(*p>'9'||(*p<'0'))	//�зǷ��ַ�
		{	
			ilen=0;
			flen=0;
			break;
		}	
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;	//ȥ������
	for(i=0;i<ilen;i++)	//�õ�������������
	{  
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen>5)flen=5;	//���ȡ5λС��
	*dx=flen;	 		//С����λ��
	for(i=0;i<flen;i++)	//�õ�С����������
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
		p1=(char*)strstr((const char *)pBuf,"$GPRMC");//"$GPRMC",������&��GPRMC�ֿ������,��ֻ�ж�GPRMC.
		{
			posx=NMEA_Comma_Pos(p1,1);								//�õ�UTCʱ��
			if(posx!=0XFF)
			{
				temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//�õ�UTCʱ��,ȥ��ms
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
		posx=NMEA_Comma_Pos(p1,3);								//�õ�γ��
		if(posx!=0XFF)
		{
		temp=NMEA_Str2num(p1+posx,&dx);		
		//gps_printf("NMEA_GPRMC_Analysis222:%d\r\n", temp);
		 g_trk_var.g_trk_content_info.gps_info.latitude = temp/NMEA_Pow(10,dx+2);	//�õ���
		rs=temp%NMEA_Pow(10,dx+2);				//�õ�'		 
		g_trk_var.g_trk_content_info.gps_info.latitude = g_trk_var.g_trk_content_info.gps_info.latitude*NMEA_Pow(10,6)+(rs*NMEA_Pow(10,6-dx))/60;//ת��Ϊ�� 
		gps_printf("NMEA_GPRMC_Analysis_latitude:%d\r\n", g_trk_var.g_trk_content_info.gps_info.latitude);
		}
		}	
		posx=NMEA_Comma_Pos(p1,5);								//�õ�����
		if(posx!=0XFF)
		{												  
		temp=NMEA_Str2num(p1+posx,&dx);		
		//gps_printf("NMEA_GPRMC_Analysis444:%d\r\n", temp);
		g_trk_var.g_trk_content_info.gps_info.longitude = temp/NMEA_Pow(10,dx+2);	//�õ���
		rs=temp%NMEA_Pow(10,dx+2);				//�õ�'		 
		g_trk_var.g_trk_content_info.gps_info.longitude  = g_trk_var.g_trk_content_info.gps_info.longitude*NMEA_Pow(10,6)+(rs*NMEA_Pow(10,6-dx))/60;//ת��Ϊ�� 
		gps_printf("NMEA_GPRMC_Analysis_longitude:%d\r\n", g_trk_var.g_trk_content_info.gps_info.longitude);
		}
		
		//posx=NMEA_Comma_Pos(p1,6);								//������������
		//if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);		
		
		//posx=NMEA_Comma_Pos(p1,7);				
		//if(posx!=0XFF)
		//{	
		//g_trk_var.g_trk_content_info.gps_info.speed_km = NMEA_Str2num(p1+posx,&dx);	//����
	        // gps_printf("NMEA_GPRMC_Analysis444:%d\r\n", g_trk_var.g_trk_content_info.gps_info.speed_km);
		//}
		
		posx=NMEA_Comma_Pos(p1,8);	
		if(posx!=0XFF)
		{
		g_trk_var.g_trk_content_info.gps_info.T_angle =  NMEA_Str2num(p1+posx,&dx);
		//gps_printf("NMEA_GPRMC_Analysis5555:%d\r\n", g_trk_var.g_trk_content_info.gps_info.T_angle);
		}
		posx=NMEA_Comma_Pos(p1,9);								//�õ�UTC����
		if(posx!=0XFF)
		{
		temp=NMEA_Str2num(p1+posx,&dx);		 				//�õ�UTC����
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

//����GPVTG��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GPVTG_Analysis(char *pBuf, int data_len)
{
	U8 *p1,dx;			 
	U8 posx;   
	//trk_comm_gps_repoter_info *pGPS = NULL;
	if ((!pBuf) || (data_len <= 0))return;
	p1=(char*)strstr((const char *)pBuf,"$GPVTG");
    	posx=NMEA_Comma_Pos(p1,7);								//�õ���������
    	if(posx!=0XFF)
    	{
    		g_trk_var.g_trk_content_info.gps_info.speed_km=NMEA_Str2num(p1+posx,&dx);
    		if(dx<3)g_trk_var.g_trk_content_info.gps_info.speed_km*=NMEA_Pow(10,3-dx);	 	 		//ȷ������1000��
    		//gps_printf("NMEA_GPVTG_Analysis11111:%d\r\n", g_trk_var.g_trk_content_info.gps_info.speed_km);
		//main_task_post_message(MAIN_MSG_GPS_EVENT, GPS_EVENT_NEW_DATA, (int)pGPS);
    	}
}  
//����GPGGA��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GPGGA_Analysis(char *pBuf, int data_len)
{	
	U8 *p1,dx;			 
	U8 posx; 
	int gps_states;
	//trk_comm_gps_repoter_info *pGPS = NULL;
	if ((!pBuf) || (data_len <= 0))return;
	p1=(char*)strstr((const char *)pBuf,"$GPGGA");
    posx=NMEA_Comma_Pos(p1,6);	
	if(posx!=0XFF) gps_states = NMEA_Str2num(p1+posx,&dx);//�õ�GPS״̬
    //gps_printf("NMEA_GPGGA_Analysis111:%d\r\n", gps_states);
    posx=NMEA_Comma_Pos(p1,7);	//�õ����ڶ�λ��������
    if(posx!=0XFF) g_trk_var.g_trk_content_info.gps_info.satellite_using_num = NMEA_Str2num(p1+posx,&dx); 
	//gps_printf("NMEA_GPGGA_Analysis222:%d\r\n", g_trk_var.g_trk_content_info.gps_info.satellite_using_num);
    posx=NMEA_Comma_Pos(p1,9);								//�õ����θ߶�
    if(posx!=0XFF)g_trk_var.g_trk_content_info.gps_info.altitude=NMEA_Str2num(p1+posx,&dx);  
	//main_task_post_message(MAIN_MSG_GPS_EVENT, GPS_EVENT_NEW_DATA, (int)pGPS);
}
//����GPGSA��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GPGSA_Analysis(char *pBuf, int data_len)
{
	U8 *p1,dx;			 
	U8 posx; 
	//U8 i; 
	//trk_comm_gps_repoter_info *pGPS = NULL;
	if ((!pBuf) || (data_len <= 0))return;
	p1=(char*)strstr((const char *)pBuf,"$GPGSA");
    	posx=NMEA_Comma_Pos(p1,2);								//�õ���λ����
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
	posx=NMEA_Comma_Pos(p1,15);								//�õ�PDOPλ�þ�������
	if(posx!=0XFF) g_trk_var.g_trk_content_info.gps_info.pdop = NMEA_Str2num(p1+posx,&dx);  
	posx=NMEA_Comma_Pos(p1,16);								//�õ�HDOPλ�þ�������
	if(posx!=0XFF)  g_trk_var.g_trk_content_info.gps_info.hdop = NMEA_Str2num(p1+posx,&dx);  
	//gps_printf("GSAhdop:%d\r\n", g_trk_var.g_trk_content_info.gps_info.hdop);
	posx=NMEA_Comma_Pos(p1,17);								//�õ�VDOPλ�þ�������
	if(posx!=0XFF)  g_trk_var.g_trk_content_info.gps_info.vdop = NMEA_Str2num(p1+posx,&dx);  
	//main_task_post_message(MAIN_MSG_GPS_EVENT, GPS_EVENT_NEW_DATA, (int)pGPS);
}

//����GPGSV��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
//ÿ��GSV�����������ʾ4�����ǵ���Ϣ������������Ϣ������һ���е�NMEA0183��������
//���Ǻ� 01-32�� �ź�ǿ�� 0 - 99
void NMEA_GPGSV_Analysis(char *pBuf, int data_len)
{
	U8 *p,*p1,dx;
	U8 len,i,j,slx=0;
	U8 posx;   	 
	p=pBuf;
	p1=(uint8_t*)strstr((const char *)p,"$GPGSV");
   	 if(p1!=NULL)
    	{
		len=p1[7]-'0';								//�õ�GPGSV������
		posx=NMEA_Comma_Pos(p1,3); 					//�õ��ɼ���������
		if(posx!=0XFF) g_trk_var.g_trk_content_info.gps_info.satellite_visable_num=NMEA_Str2num(p1+posx,&dx);	
		//gps_printf("NMEA_GPGSV_Analysis num=%d\r\n", g_trk_var.g_trk_content_info.gps_info.satellite_visable_num);
		//if(posx!=0XFF)gpsx->svnum=NMEA_Str2num(p1+posx,&dx);
		for(i=0;i<len;i++)
		{	 
			p1=(U8*)strstr((const char *)p,"$GPGSV");  
			for(j=0;j<4;j++)
			{	  
				posx=NMEA_Comma_Pos(p1,4+j*4);
				if(posx!=0XFF)g_trk_var.g_trk_content_info.gps_info.prn_info[slx].num=NMEA_Str2num(p1+posx,&dx);	//�õ����Ǳ��
				else break; 
				posx=NMEA_Comma_Pos(p1,5+j*4);
				if(posx!=0XFF)g_trk_var.g_trk_content_info.gps_info.prn_info[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//�õ��������� 
				else break;
				posx=NMEA_Comma_Pos(p1,6+j*4);
				if(posx!=0XFF)g_trk_var.g_trk_content_info.gps_info.prn_info[slx].azideg=NMEA_Str2num(p1+posx,&dx);//�õ����Ƿ�λ��
				else break; 
				posx=NMEA_Comma_Pos(p1,7+j*4);
				if(posx!=0XFF)g_trk_var.g_trk_content_info.gps_info.prn_info[slx].sn=NMEA_Str2num(p1+posx,&dx);	//�õ����������
				else break;
				//gps_printf("NMEA_GPGSV_Analysisg id=%d--------signal:%d\r\n", g_trk_var.g_trk_content_info.gps_info.prn_info[slx].num,g_trk_var.g_trk_content_info.gps_info.prn_info[slx].sn);
				slx++;
			}   
	 		p=p1+1;//�л�����һ��GPGSV��Ϣ
		}
	}
}

#endif
