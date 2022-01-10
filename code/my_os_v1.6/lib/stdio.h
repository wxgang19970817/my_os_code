
/*************************************************************************
	> File Name: stdio.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月22日 星期三 15时18分49秒
 ************************************************************************/
#ifndef __LIB_STDIO_H
#define __LIB_STDIO_H

#include "stdint.h"

typedef char* va_list;
uint32_t printf(const char* str,...);
uint32_t vsprintf(char* str,const char* format,va_list ap);
uint32_t sprintf(char* buf, const char* format, ...);
#endif

