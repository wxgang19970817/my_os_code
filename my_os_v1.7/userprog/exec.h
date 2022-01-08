/*************************************************************************
	> File Name: exec.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2022年01月07日 星期五 13时48分36秒
 ************************************************************************/

#ifndef __USERPROG_EXEC_H
#define __USERPROG_EXEC_H
#include "stdint.h"
int32_t sys_execv(const char* path, const char*  argv[]);
#endif