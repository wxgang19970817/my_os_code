/*************************************************************************
	> File Name: wait_exit.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2022年01月08日 星期六 15时06分46秒
 ************************************************************************/

#ifndef __USERPROG_WAITEXIT_H
#define __USERPROG_WAITEXIT_H
#include "thread.h"
pid_t sys_wait(int32_t* status);
void sys_exit(int32_t status);
#endif 
