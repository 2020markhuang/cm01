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
	��������:�Զ����ͷ��ڴ溯��
	����˵��:ptr	��Ҫ�ͷŵ��ڴ��ָ��
	�� �� ֵ:��
	������Ա:����ǿ
*/

void myFree(void *ptr);
/*
	��������:�Զ�������ڴ溯��
	����˵��:num_bytes	�����ֽ���
	�� �� ֵ:����
	������Ա:����ǿ
*/

void *myMalloc(unsigned int num_bytes);

#if defined(__cplusplus)
}
#endif 

#endif //MY_MEMORY_H



