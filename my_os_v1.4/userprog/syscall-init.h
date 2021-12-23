/*************************************************************************
	> File Name: syscall-init.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月21日 星期二 20时08分25秒
 ************************************************************************/
#ifndef __USERPROG_SYSCALLINIT_H
#define __USERPROG_SYSCALLINIT_H

#include "stdint.h"
void syscall_init(void);
uint32_t sys_getpid(void);
uint32_t sys_write(char* str);
#endif

