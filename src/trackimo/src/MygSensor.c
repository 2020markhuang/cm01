#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "MyCommon.h"
#include "MainTask.h"
#include "MygSensor.h"

#ifdef SUPPORT_MY_GSENSOR
extern BOOL smart_accelerator;
typedef enum
{
    GSENSOR_ON_FROM_XMODE = 0,   
    GSENSOR_ON_FROM_FENCE_SPEED_ALARM,   
    GSENSOR_ON_FROM_MOVING_ALARM, 
    GSENSOR_ON_FROM_TESTMODE,
    GSENSOR_ON_FROM_MAX   
} gsensor_on_from_enum;
 
BOOL gsensor_on[GSENSOR_ON_FROM_MAX] = {FALSE,FALSE,FALSE,FALSE};
BOOL GSENSOR_INT_ON = FALSE;
U16 gsensor_eint_enable_fail_count = 0;
void gSensor_driver_init(void){
	gsensor_printf("gSensor_driver_init\r\n");
	#ifndef WIN32
	extern void bsp_sensor_peripheral_init(void);
	bsp_sensor_peripheral_init();
	#endif				
}
void gSensor_on(void){
	gsensor_printf("gSensor_on\r\n");
	{
		extern void mc3410_eint_enable(void);
		mc3410_eint_enable();
	}				
}
void gSensor_off(void){
	gsensor_printf("gSensor_off\r\n");
	{
		extern void mc3410_eint_disable(void);
		mc3410_eint_disable();
	}				
}

void moving_alarm_turnon_gsensor(void)	
{
	printf("moving_alarm_turnon_gsensor\r\n");
	gsensor_on[GSENSOR_ON_FROM_MOVING_ALARM] = TRUE;
}

void moving_alarm_turnoff_gsensor(void)	
{
	printf("moving_alarm_turnoff_gsensor\r\n");
	gsensor_on[GSENSOR_ON_FROM_MOVING_ALARM] = FALSE;

}
//-------------------------------------------------------
void xmode_turnon_gsensor(void)	
{	 
	printf("xmode_turnon_gsensor\r\n");	 
 	gsensor_on[GSENSOR_ON_FROM_XMODE] = TRUE;
}

void xmode_turnoff_gsensor(void)	
{
	printf("xmode_turnoff_gsensor\r\n"); 
   	gsensor_on[GSENSOR_ON_FROM_XMODE] = FALSE;

}

BOOL Is_smart_accelerator(void)
{
	if(smart_accelerator == TRUE)
	{		
		gsensor_printf("Is_smart_accelerator KAL_TRUE\r\n");
		return TRUE;
	}
	else
	{		
		gsensor_printf("Is_smart_accelerator KAL_FALSE\r\n");
		return FALSE;
	}
}

void fence_speed_alarm_turnoff_gsensor(void)	
{
	printf("fence_alarm_turnoff_gsensor\r\n");
 	gsensor_on[GSENSOR_ON_FROM_FENCE_SPEED_ALARM] = FALSE;
}

void fence_speed_alarm_turnon_gsensor(void)	
{
	printf("fence_alarm_turnon_gsensor\r\n"); 
	gsensor_on[GSENSOR_ON_FROM_FENCE_SPEED_ALARM] = TRUE;
}

void CheckTurnOnOffGsensor(void)
{
	U8 i=0;
	BOOL turnoff = FALSE;
	if(Is_smart_accelerator() == FALSE)
	{
		gSensor_on();
		return;
	}	

	for(i=0;i<GSENSOR_ON_FROM_MAX;i++)
	{
		if(gsensor_on[i] == TRUE)
		{
			turnoff = TRUE;
			gsensor_printf("CheckIfTurnOffGsensor: gsensor_on[%d] True!\r\n",i);	
		}  
	}

	if(turnoff == TRUE)
	{
		gsensor_printf("CheckTurnOnOffGsensor: ON!\r\n");
		gSensor_on();
	}
	else
	{
		gsensor_printf("CheckIfTurnOffGsensor: OFF!\r\n");	
		gSensor_off();
	}	  			
}	

void gSensor_get_data(I16 * pX, I16 * pY , I16 * pZ){

	if(!pX || !pY || !pZ)return;
	#ifndef WIN32
	{
		extern I8 mc3410_read_accel_xyz(I16 *x,I16 *y, I16 *z);
	        mc3410_read_accel_xyz(pX, pY, pZ);
	} 
	#endif		 
	gsensor_printf("gSensor_get_data:%d,%d,%d\r\n", *pX, *pY, *pZ);
}

void ongSensorInt(int type, int para){
	gsensor_printf("ongSensorInt\r\n");
	main_task_post_message(MAIN_MSG_GSENSOR_EVENT, GSENSOR_EVENT_FALL, 0);
}
#endif
