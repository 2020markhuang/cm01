#ifndef MY_TEST_H_20200324
#define MY_TEST_H_20200324
#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif	 
#define GPS_BACKUP_MAX_NUMBER 250   
 typedef enum
 {
	 NVDM_SETTING_OFF = 0x00, 				
	 NVDM_SETTING_SAMPLE_TIME = 0x01,						
	 NVDM_SETTING_REPORT_TIME = 0x02, 				
	 NVDM_SETTING_ALL = NVDM_SETTING_SAMPLE_TIME|NVDM_SETTING_REPORT_TIME,
 } tracker_nvdm_type_enum;

void tracker_settings_record_file_init(tracker_nvdm_type_enum rnum);
void tracker_refresh_settings_record(tracker_nvdm_type_enum wnum);
U16 get_number_of_records_in_backup_database(void);
void print_log_for_gps_backup(void);
void gps_back_up_push_fixed(void);
void gps_back_up_push_no_fix(void);
#if defined(__cplusplus)
}
#endif 

#endif //MY_TEST_H_20190903



