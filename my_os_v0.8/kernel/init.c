/*************************************************************************
	> File Name: init.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月02日 星期四 19时19分44秒
 ************************************************************************/
#include "init.h"
#include "print.h"
#include "interrupt.h"

/* 负责初始化所有模块 */
void init_all()
{
    put_str("init_all\n");
    idt_init();
}
