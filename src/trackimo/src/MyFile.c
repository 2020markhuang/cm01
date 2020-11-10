#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "TrackerMain.h"
#include "nvdm.h"
#include "MyGPS.h"
#include "MyFile.h"
#include "MyLog.h"
#include "MyDefine.h"
trackergps_backup_data_struct gps_backup[GPS_BACKUP_MAX_NUMBER];//保存500个坐标,共40*500 = 20000byte,20k byte
trackergps_backup_data_struct gps_backup_fixed_history_buffer[GPS_BACKUP_MAX_NUMBER];//根据时间的fix坐标，最多一样500个，20k byte
void gps_back_up_push_fixed(void)
{
  	int i,j;
  	int len= 0;

  	//依次往下压
	for(i=GPS_BACKUP_MAX_NUMBER-1;i>0; i--)
	{
  		 memcpy(&(gps_backup[i]),&(gps_backup[i-1]),sizeof(trackergps_backup_data_struct));
  	}

	  	//最上面的永远是最新的,用255代表0
	memset(&(gps_backup[0]), 0, sizeof(trackergps_backup_data_struct));

  	//最上面的永远是最新的
  	gps_backup[0].type = GPS_FIXED;
  	gps_backup[0].year =   g_trk_var.g_trk_content_info.gps_info.date_time.year;
  	gps_backup[0].month = g_trk_var.g_trk_content_info.gps_info.date_time.month;
  	gps_backup[0].day = g_trk_var.g_trk_content_info.gps_info.date_time.day;
  	gps_backup[0].hour = g_trk_var.g_trk_content_info.gps_info.date_time.hour;
  	gps_backup[0].minute = g_trk_var.g_trk_content_info.gps_info.date_time.minute;
  	gps_backup[0].second = g_trk_var.g_trk_content_info.gps_info.date_time.second;
  	
  	gps_backup[0].longitude = (double)g_trk_var.g_trk_content_info.gps_info.longitude / 1000000;
  	gps_backup[0].latitude = (double)g_trk_var.g_trk_content_info.gps_info.latitude / 1000000;
  	gps_backup[0].altitude = (double)g_trk_var.g_trk_content_info.gps_info.altitude / 10;
  	gps_backup[0].speed = (double)g_trk_var.g_trk_content_info.gps_info.speed_km /1000;
  	gps_backup[0].battery = getBatteryCapacity();
}

//没有fix的时候，有时也要保存
void gps_back_up_push_no_fix(void)
{
  	U16 i = 0;
  	//char log[400];
  	//依次往下压
	for(i=GPS_BACKUP_MAX_NUMBER-1;i>0; i--)
  	{
  		 memcpy(&(gps_backup[i]),&(gps_backup[i-1]),sizeof(trackergps_backup_data_struct));
  	}
  	//最上面的永远是最新的,用255代表0
	memset(&(gps_backup[0]), 0x00, sizeof(trackergps_backup_data_struct));
  	gps_backup[0].type = GPS_NO_FIX;
  	gps_backup[0].year = 255;
  	gps_backup[0].month = 255;
  	gps_backup[0].day = 255;
  	gps_backup[0].hour = 255;
  	gps_backup[0].minute = 255;
  	gps_backup[0].second = 255;
  	gps_backup[0].longitude = 0;
  	gps_backup[0].latitude = 0;
  	gps_backup[0].altitude = 0;
  	gps_backup[0].speed = 0;
  	gps_backup[0].battery = getBatteryCapacity();
}	

U16 get_number_of_records_in_backup_database(void)
{
	U16 number = 0;
	for(number = 0;number<GPS_BACKUP_MAX_NUMBER;number++)
       {
  		if(gps_backup[number].type == GPS_EMPTY)
  			break;	
	}
	main_printf("get_number_of_records_in_backup_database:%d\r\n",number);
	return number;
}

void print_log_for_gps_backup(void)
{
	U16  i=0;
	char log[100];	
	char tmp_log[100];	
	for(i=0;i<10;i++)
	{
		  memset(log,0,100);
		  sprintf(log,"%d %d-%d-%d:%d-%d-%d  lon:%f,lat:%f,alt:%f,speed:%f\r\n ",gps_backup[i].type,gps_backup[i].year,gps_backup[i].month,gps_backup[i].day,
		                                                                 gps_backup[i].hour,gps_backup[i].minute,gps_backup[i].second,
		                                                                 gps_backup[i].longitude,gps_backup[i].latitude,gps_backup[i].altitude,gps_backup[i].speed);
		  main_printf("%d: gps_backup:%s\r\n",i,log);   
	}
	get_number_of_records_in_backup_database();
}  

void nvram_test(void)
{
	main_printf("nvram_test\r\n");
	U8 buf[11];
	U32 buf_size;
	nvdm_status_t nvdm_status;
	buf_size = sizeof(buf);
	//nvdm_status = nvdm_read_data_item("2511", "userid", (U8 *)buf, &buf_size);
	if (  nvdm_write_data_item("2511", "userid", NVDM_DATA_ITEM_TYPE_STRING, (U8 *)"1111111111", 11) != NVDM_STATUS_OK)
	{
	//nvdm_write_data_item("2511", "userid", NVDM_DATA_ITEM_TYPE_STRING, (uint8_t *)"0000000000", 11);
	printf("write nvdm userid fail \r\n");
	} else
	{
	nvdm_status = nvdm_read_data_item("2511", "userid", (U8 *)buf, &buf_size);
	if(nvdm_status != NVDM_STATUS_OK )	
	{
			printf("read nvdm userid fail \r\n");
	}
	else
	{
			printf("userid = %s\r\n", buf);
	}
	}

}

/**
	读取settings记录,如果记录有更新则更新nvram
**/
void tracker_settings_record_file_init(tracker_nvdm_type_enum rnum )
{
	U32 size;
	main_printf("tracker_settings_record_file_init:%x\r\n",rnum);
	if(rnum == NVDM_SETTING_SAMPLE_TIME)
		{
			size = sizeof(g_trk_var.g_trk_settings.sample_time);
			if (nvdm_read_data_item("device", "sample_time", (U8 *)&g_trk_var.g_trk_settings.sample_time, &size) != NVDM_STATUS_OK) {
		            main_printf("device read sample time error\r\n");
		        }
		}
	else if(rnum == NVDM_SETTING_REPORT_TIME)
		{
			size = sizeof(g_trk_var.g_trk_settings.report_time);
			if (nvdm_read_data_item("device", "report_time", (U8 *)&g_trk_var.g_trk_settings.report_time,  &size) != NVDM_STATUS_OK) {
		            main_printf("device read report time error\r\n");
		        }
			
		}
	else if(rnum == NVDM_SETTING_ALL)
		{
			size = sizeof(g_trk_var.g_trk_settings.sample_time);
			if (nvdm_read_data_item("device", "sample_time", (U8 *)&g_trk_var.g_trk_settings.sample_time, &size) != NVDM_STATUS_OK) {
		            main_printf("device read sample time error\r\n");
	        	}	
			size = sizeof(g_trk_var.g_trk_settings.report_time);
			if (nvdm_read_data_item("device", "report_time", (U8 *)&g_trk_var.g_trk_settings.report_time,  &size) != NVDM_STATUS_OK) {
		            main_printf("device read report time error\r\n");
		        }
			 main_printf("sample_time= %d,report_time=%d\r\n",g_trk_var.g_trk_settings.sample_time,g_trk_var.g_trk_settings.report_time);
		}
}

/**
	更新settings记录,如果记录有更新则更新nvram
**/
void tracker_refresh_settings_record(tracker_nvdm_type_enum wnum)
{
	U32 size;
	main_printf("tracker_refresh_settings_record:%x\r\n",wnum);
	if(wnum == NVDM_SETTING_SAMPLE_TIME)
		{
			size = sizeof(g_trk_var.g_trk_settings.sample_time);
			 if (nvdm_write_data_item("device", "sample_time", NVDM_DATA_ITEM_TYPE_RAW_DATA, (U8 *)&g_trk_var.g_trk_settings.sample_time, size) != NVDM_STATUS_OK) 
			 {
		         	main_printf("device write sample time error\r\n");
		         }
		}
	else if(wnum == NVDM_SETTING_REPORT_TIME)
		{
			size = sizeof(g_trk_var.g_trk_settings.report_time);
			 if (nvdm_write_data_item("device", "report_time", NVDM_DATA_ITEM_TYPE_RAW_DATA, (U8 *)&g_trk_var.g_trk_settings.report_time, size) != NVDM_STATUS_OK) 
			 {
		          	main_printf("device write report time error\r\n");
		         }
		}
	else if(wnum ==NVDM_SETTING_ALL )
		{
			size = sizeof(g_trk_var.g_trk_settings.sample_time);
			 if (nvdm_write_data_item("device", "sample_time", NVDM_DATA_ITEM_TYPE_RAW_DATA, (U8 *)&g_trk_var.g_trk_settings.sample_time, size) != NVDM_STATUS_OK) 
			 {
		         	main_printf("device write sample time error\r\n");
		         }
			 
			size = sizeof(g_trk_var.g_trk_settings.report_time);
			 if (nvdm_write_data_item("device", "report_time", NVDM_DATA_ITEM_TYPE_RAW_DATA, (U8 *)&g_trk_var.g_trk_settings.report_time, size) != NVDM_STATUS_OK) 
			 {
		          	main_printf("device write report time error\r\n");
		         }
		}
}



