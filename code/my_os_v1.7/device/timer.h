/*************************************************************************
	> File Name: timer.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月05日 星期日 20时06分38秒
 ************************************************************************/
#ifndef __DEVICE_TIME_H
#define __DEVICE_TIME_H

#include "../lib/stdint.h"

void timer_init(void);
void mtime_sleep(uint32_t m_seconds);

#endif
