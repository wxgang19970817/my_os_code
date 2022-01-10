#include "../lib/kernel/print.h"
#include "init.h"
#include "memory.h"
#include "interrupt.h"
#include "../thread/thread.h"
#include "../device/console.h"

/* 临时为测试添加 */
#include "../device/ioqueue.h"
#include "../device/keyboard.h"

void k_thread_a(void*);
void k_thread_b(void*);


int main(void)
{
    put_str("I am kernel\n");
    init_all();

   thread_start("consumer",31,k_thread_a," A_ ");
   thread_start("consumer",31,k_thread_b," B_ ");

    intr_enable();          /* 开中断，使时钟中断起作用 */
    while(1);
   // {
     //   console_put_str("Main "); 
   // };
    return 0;
}

void k_thread_a(void* arg)
{
    /* 用void*来通用表示参数，被调用的函数知道自己需要什么类型的参数，自己转换再用 */
    char* para = arg;
    while (1) 
    {
        enum intr_status old_status = intr_disable();
        if (!ioq_empty(&kbd_buf))
        {
            console_put_str(arg);
            char byte = ioq_getchar(&kbd_buf);
            console_put_char(byte);
        }
        intr_set_status(old_status);
    }
}

void k_thread_b(void* arg)
{
    char* para = arg;
    while(1)
    {
          enum intr_status old_status = intr_disable();
        if (!ioq_empty(&kbd_buf))
        {
            console_put_str(arg);
            char byte = ioq_getchar(&kbd_buf);
            console_put_char(byte);
        }
        intr_set_status(old_status);
    }
}
