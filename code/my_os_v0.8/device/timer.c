/*************************************************************************
	> File Name: timer.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月05日 星期日 19时26分11秒
 ************************************************************************/

#include "timer.h"
#include "../lib/kernel/io.h"
#include "../lib/kernel/print.h"

#define IRQ_FREQUENCY       100
#define INPUT_FREQUENCY     1193180
#define COUNTER0_VALUE      INPUT_FREQUENCY / IRQ_FREQUENCY
#define COUNTER0_PORT       0X40
#define COUNTER0_NO         0
#define COUNTER0_MODE       2
#define READ_WRITE_LATCH    3
#define PIT_CONTROL_PORT    0x43


/* 把操作的计数器 counter_no、读写锁属性rwl、计数器模式、counter_mode写入模式控制寄存器并赋予初始值counter_value */
static void frequency_set(uint8_t counter_port,uint8_t counter_no,uint8_t rwl,uint8_t counter_mode,uint16_t counter_value)
{
    /* 往控制字寄存器端口 0x43  */
    outb(PIT_CONTROL_PORT,(uint8_t)(counter_no << 6 | rwl << 4 | counter_mode <<1));

    /* 先写入counter_value 的低8位 */
    outb(counter_port,(uint8_t)counter_value);

    /* 再写入counter_value 的高8位 */
    outb(counter_port,(uint8_t)counter_value >> 8);
}


/* 初始化PIT8253 */
void timer_init()
{
    put_str("timer_init start\n");
    
    /* 设置8253的定时周期，也就是发中断周期 */
    frequency_set(COUNTER0_PORT,COUNTER0_NO,READ_WRITE_LATCH,COUNTER0_MODE,COUNTER0_VALUE);

    put_str("timer_init done\n");
}
