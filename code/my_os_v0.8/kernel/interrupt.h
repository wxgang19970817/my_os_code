/*************************************************************************
	> File Name: interrupt.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月02日 星期四 09时46分22秒
 ************************************************************************/

#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H
#include "stdint.h"

typedef void* intr_handler;

void idt_init(void);






#endif
