
#include "../lib/kernel/print.h"
#include "init.h"
#include "memory.h"
#include "interrupt.h"
#include "../thread/thread.h"
#include "../device/console.h"
#include "../userprog/process.h"
#include "../userprog/syscall-init.h"
#include "syscall.h"
#include "stdio.h"

void k_thread_a(void*);
void k_thread_b(void*);
void u_prog_a(void);
void u_prog_b(void);


int main(void)
{
    put_str("I am kernel\n");
    init_all();

    // process_execute(u_prog_a,"user_prog_a");
    // process_execute(u_prog_b,"user_prog_b");

    intr_enable();          /* 开中断，使时钟中断起作用 */
 
    thread_start("k_thread_a", 31, k_thread_a, "argA ");
    thread_start("k_thread_b", 31, k_thread_b, "argB ");
    while(1);
    return 0;
}

void k_thread_a(void* arg)
{
    /* 用void*来通用表示参数，被调用的函数知道自己需要什么类型的参数，自己转换再用 */
    char* para = arg;
    void* addr = sys_malloc(33);
    console_put_str(" i am thread_a,sys_malloc(33), addr is 0x");
    console_put_int((int)addr);
    console_put_char('\n');
    while (1);
}

void k_thread_b(void* arg)
{
    char* para = arg;
     void* addr = sys_malloc(63);
    console_put_str(" i am thread_b,sys_malloc(63), addr is 0x");
    console_put_int((int)addr);
  
    console_put_char('\n');
    while (1);
}


/* 测试用户进程 */
void u_prog_a(void)
{
    char* name = "prog_a";
    printf("I am %s,my_pid:%d%c",name,getpid(),'\n');
    while(1);
}

void u_prog_b(void)
{
    char* name = "prog_b";
    printf(" I am %s, my pid:%d%c", name, getpid(), '\n');
    while(1);
}