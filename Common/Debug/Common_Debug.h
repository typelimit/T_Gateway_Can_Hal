#ifndef __COMMON_DEBUG_H__
#define __COMMON_DEBUG_H__
#include "usart.h"
#include "string.h"
#include "stdio.h"

void Common_Debug_Init(void);

#define DEBUG

#ifdef DEBUG
//debug_println("hello %s","lisi")  => [main.c:21] hello lisi
//__FILE__: 获取文件名
//__LINE__: 获取行号
//... : 可变参数[参数个数不固定]
//__VA_ARGS: 获取可变参数,要求参数最少1个
//##__VA_ARGS: 代表传入的可变参数是最少0个
//debug_println("hello %s","lisi") 此时 format="hello %s"     ...代表参数 "lisi"
//printf("[%s:%d]" format,__FILE__,__LINE__,##__VA_ARGS) => "[%s:%d]  " + format做字符串拼接 => "[%s:%d]  hello %s"
//User\main.c
//User\Driver\Systick\Driver_Systick.c
//

//strrchr: 从字符串中右边找字符,找到则返回指向该字符的指针,没找到则返回NULL
#define FILE_NAME strrchr(__FILE__,'\\') ? strrchr(__FILE__,'\\') + 1 : __FILE__

#define FILE_NAME2 strrchr(FILE_NAME,'/') ? strrchr(FILE_NAME,'/') + 1 : FILE_NAME

#define debug_println(format,...) printf("[%20s:%d]  " format "\r\n" ,FILE_NAME2,__LINE__,##__VA_ARGS__)
#else
#define debug_println(format,...) 
#endif

#endif
