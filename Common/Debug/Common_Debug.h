#ifndef __COMMON_DEBUG_H__
#define __COMMON_DEBUG_H__
#include "usart.h"
#include "string.h"
#include "stdio.h"

void Common_Debug_Init(void);

#define DEBUG

#ifdef DEBUG
//debug_println("hello %s","lisi")  => [main.c:21] hello lisi
//__FILE__: ��ȡ�ļ���
//__LINE__: ��ȡ�к�
//... : �ɱ����[�����������̶�]
//__VA_ARGS: ��ȡ�ɱ����,Ҫ���������1��
//##__VA_ARGS: ������Ŀɱ����������0��
//debug_println("hello %s","lisi") ��ʱ format="hello %s"     ...������� "lisi"
//printf("[%s:%d]" format,__FILE__,__LINE__,##__VA_ARGS) => "[%s:%d]  " + format���ַ���ƴ�� => "[%s:%d]  hello %s"
//User\main.c
//User\Driver\Systick\Driver_Systick.c
//

//strrchr: ���ַ������ұ����ַ�,�ҵ��򷵻�ָ����ַ���ָ��,û�ҵ��򷵻�NULL
#define FILE_NAME strrchr(__FILE__,'\\') ? strrchr(__FILE__,'\\') + 1 : __FILE__

#define FILE_NAME2 strrchr(FILE_NAME,'/') ? strrchr(FILE_NAME,'/') + 1 : FILE_NAME

#define debug_println(format,...) printf("[%20s:%d]  " format "\r\n" ,FILE_NAME2,__LINE__,##__VA_ARGS__)
#else
#define debug_println(format,...) 
#endif

#endif
