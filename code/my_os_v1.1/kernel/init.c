/*************************************************************************
	> File Name: init.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月02日 星期四 19时19分44秒
 ************************************************************************/
#include "init.h"
#include "../lib/kernel/print.h"
#include "interrupt.h"
#include "../device/timer.h"
#include "memory.h"
#include "../thread/thread.h"


/* 负责初始化所有模块 */
void init_all()
{
    //put_str("init_all\n");
    idt_init();             /*初始化中断*/
    mem_init();             /* 初始化内存管理系统 */
    thread_init();          /* 初始化线程相关结构 */
    timer_init();           /* 初始化PIT */
}
