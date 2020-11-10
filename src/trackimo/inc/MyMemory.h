#ifndef MY_MEMORY_H
#define MY_MEMORY_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#ifdef WIN32
void *pvPortMalloc(unsigned int num_bytes);
void vPortFree(void *ptr);

#endif
/*
	功能作用:自定义释放内存函数
	参数说明:ptr	需要释放的内存的指针
	返 回 值:无
	开发人员:付华强
*/

void myFree(void *ptr);
/*
	功能作用:自定义分配内存函数
	参数说明:num_bytes	分配字节数
	返 回 值:如题
	开发人员:付华强
*/

void *myMalloc(unsigned int num_bytes);

#if defined(__cplusplus)
}
#endif 

#endif //MY_MEMORY_H



