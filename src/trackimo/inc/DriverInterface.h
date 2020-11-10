#ifndef DRIVER_INTERFACE_FHQ_20190722_H
#define DRIVER_INTERFACE_FHQ_20190722_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

void driver_init(void);
void system_entry_sleep(void);
void system_exit_sleep(void);
void main_entry_sleep(void);
void main_4g_exit_sleep(void);
#if defined(__cplusplus)
}
#endif 

#endif	//DRIVER_INTERFACE_FHQ_20190722_H

