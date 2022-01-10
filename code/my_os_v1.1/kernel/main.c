#include "../lib/kernel/print.h"
#include "init.h"
#include "memory.h"
#include "../thread/thread.h"


void k_thread_a(void*);
void k_thread_b(void*);


int main(void)
{
    put_str("I am kernel\n");
    init_all();

    thread_start("k_thread_a",8,k_thread_a,"argS ");
    thread_start("k_thread_b",11,k_thread_b,"argB ");

    intr_enable();          /* 开中断，使时钟中断起作用 */
    while(1)
    {
        put_str("Main ");
    };
    return 0;
}

void k_thread_a(void* arg)
{
    /* 用void*来通用表示参数，被调用的函数知道自己需要什么类型的参数，自己转换再用 */
    char* para = arg;
    while (1) 
    {
        put_str(para);
    }
}

void k_thread_b(void* arg)
{
    char* para = arg;
    while(1)
    {
        put_str(para);
    }
}
