/*************************************************************************
	> File Name: console.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月15日 星期三 11时06分10秒
 ************************************************************************/
#ifndef __DEVICE_CONSOLE_H
#define __DEVICE_CONSOLE_H

#include "../lib/stdint.h"

void console_init(void);
void console_acquire(void);
void console_release(void);
void console_put_str(char* str);
void console_put_char(uint8_t char_asci);
void console_put_int(uint32_t num);

#endif

