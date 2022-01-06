/*************************************************************************
	> File Name: syscall.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月21日 星期二 19时22分17秒
 ************************************************************************/
#ifndef __LIB_USER_SYSCALL_H
#define __LIB_USER_SYSCALL_H

#include "stdint.h"


/* 用来存放系统子功能号 */
enum SYSCALL_NR
{
	SYS_GETPID,
	SYS_WRITE,
	SYS_MALLOC,
	SYS_FREE,
	SYS_FORK
};

uint32_t getpid(void);
uint32_t write(int32_t fd, const void* buf, uint32_t count);
void* malloc(uint32_t size);
void free(void* ptr);
int16_t fork(void);

#endif

