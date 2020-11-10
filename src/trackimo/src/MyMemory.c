#include <stddef.h>
#include <stdlib.h>

#include "MyMemory.h"

#ifdef WIN32
void *pvPortMalloc(unsigned int num_bytes){
	return malloc(num_bytes);
}
void vPortFree(void *ptr){
	free(ptr);
}
#endif

void *myMalloc(unsigned int num_bytes){
#ifdef WIN32
	return malloc(num_bytes);
#else
	extern void *pvPortMalloc(unsigned int num_bytes);
	return pvPortMalloc(num_bytes);
#endif
}
void myFree(void *ptr){
#ifdef WIN32
	free(ptr);
#else
	extern void vPortFree( void *ptr );
	return vPortFree(ptr);
#endif
}
