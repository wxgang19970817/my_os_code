/*************************************************************************
	> File Name: timer.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月05日 星期日 19时26分11秒
 ************************************************************************/

#include "timer.h"
#include "../lib/kernel/io.h"
#include "../lib/kernel/print.h"
#include "../kernel/interrupt.h"
#include "../thread/thread.h"
#include "../kernel/debug.h"

#define IRQ0_FREQUENCY       100            /* 时钟中断频率每秒100次 */
#define INPUT_FREQUENCY     1193180
#define COUNTER0_VALUE      INPUT_FREQUENCY / IRQ0_FREQUENCY
#define COUNTER0_PORT       0x40
#define COUNTER0_NO         0
#define COUNTER_MODE       2
#define READ_WRITE_LATCH    3
#define PIT_CONTROL_PORT    0x43

#define mil_seconds_per_intr    (1000 / IRQ0_FREQUENCY)         /* 每多少毫秒发生一次中断,1个中断周期是10毫秒 */

uint32_t ticks;      /* ticks是内核自中断开启以来总共的嘀嗒数 */


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


/* 时钟的中断处理函数 */
static void intr_timer_handler(void)
{
    struct task_struct* cur_thread = running_thread();

    ASSERT(cur_thread->stack_magic == 0x19870916);  /* 检查栈是否溢出 */

    cur_thread->elapsed_ticks++;   /* 记录此线程占用cpu的时间 */

    ticks++;        /* 从内核第一次处理时间中断后开始至今的滴答数，内核态和用户态总共的滴答数 */

    if(cur_thread->ticks == 0)
    {
        /* 如果进程时间片用完，就开始调度新的进程上cpu */
        schedule();
    }
    else
    {
        /* 当前进程时间-1 */
        cur_thread->ticks--;
    }
}




/* 以ticks为单位的sleep,任何时间形式的sleep会转换此ticks形式 */
/* sleep_ticks是中断发生的次数 */
static void ticks_to_sleep(uint32_t sleep_ticks)
{
    uint32_t start_tick = ticks;

    /* 若间隔的ticks数不够便让出cpu */
    while(ticks - start_tick < sleep_ticks)
    {
        thread_yield();
    }
}


/* 以毫秒为单位的sleep　1秒 = 1000毫秒 */
void mtime_sleep(uint32_t m_seconds)
{
    uint32_t sleep_ticks = DIV_ROUND_UP(m_seconds,mil_seconds_per_intr);
    ASSERT(sleep_ticks > 0);
    ticks_to_sleep(sleep_ticks);
}


/* 初始化PIT8253 */
void timer_init()
{
    put_str("timer_init start\n");
    
    /* 设置8253的定时周期，也就是发中断周期 */
    frequency_set(COUNTER0_PORT,COUNTER0_NO,READ_WRITE_LATCH,COUNTER_MODE,COUNTER0_VALUE);

    /* 注册时钟中断 */
    register_handler(0x20,intr_timer_handler);

    put_str("timer_init done\n");
}