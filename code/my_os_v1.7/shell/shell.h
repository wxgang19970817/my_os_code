/*************************************************************************
	> File Name: shell.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2022年01月06日 星期四 13时55分30秒
 ************************************************************************/

#ifndef __KERNEL_SHELL_H
#define __KERNEL_SHELL_H
#include "fs.h"
void print_prompt(void);
void my_shell(void);
extern char final_path[MAX_PATH_LEN];
#endif

