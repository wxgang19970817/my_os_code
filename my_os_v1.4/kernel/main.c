
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
 
    thread_start("k_thread_a", 31, k_thread_a, "I am thread_a");
    thread_start("k_thread_b", 31, k_thread_b, "I am thread_b ");
    while(1);
    return 0;
}

void k_thread_a(void* arg)
{
    /* 用void*来通用表示参数，被调用的函数知道自己需要什么类型的参数，自己转换再用 */
    char* para = arg;

    void* addr1;
    void* addr2;
    void* addr3;
    void* addr4;
    void* addr5;
    void* addr6;
    void* addr7;

    console_put_str(" thread_a start\n");

    int max = 1000;

    while(max-- > 0)
    {
        int size = 128;
        addr1 = sys_malloc(size);
        size *= 2;
        addr2 = sys_malloc(size);
        size *= 2;
        addr3 = sys_malloc(size);
        sys_free(addr1);
        addr4 = sys_malloc(size);
        size *= 2;size *= 2;size *= 2;size *= 2;
         size *= 2; size *= 2; size *= 2;
        addr5 = sys_malloc(size);
        addr6 = sys_malloc(size);
        sys_free(addr5);
        size *= 2;
        addr7 = sys_malloc(size);
        sys_free(addr6);
        sys_free(addr7);
        sys_free(addr2);
        sys_free(addr3);
        sys_free(addr4);
    }

    console_put_str(" thread_a end \n");
    while (1);
}

void k_thread_b(void* arg)
{ 
    /* 用void*来通用表示参数，被调用的函数知道自己需要什么类型的参数，自己转换再用 */
    char* para = arg;

    void* addr1;
    void* addr2;
    void* addr3;
    void* addr4;
    void* addr5;
    void* addr6;
    void* addr7;
    void* addr8;
    void* addr9;

    console_put_str(" thread_b start\n");

    int max = 1000;

    while(max-- > 0)
    {
        int size = 9;
        addr1 = sys_malloc(size);
        size *= 2;
        addr2 = sys_malloc(size);
        size *= 2;
        sys_free(addr2);
        addr3 = sys_malloc(size);
        sys_free(addr1);
        addr4 = sys_malloc(size);
        addr5 = sys_malloc(size);
        addr6 = sys_malloc(size);
        sys_free(addr5);
        size *= 2;
        addr7 = sys_malloc(size);
        sys_free(addr6);
        sys_free(addr7);
        sys_free(addr3);
        sys_free(addr4);

        size *= 2;size *= 2;size *= 2;
        addr1 = sys_malloc(size);
        addr2 = sys_malloc(size);
        addr3 = sys_malloc(size);
        addr4 = sys_malloc(size);
        addr5 = sys_malloc(size);
        addr6 = sys_malloc(size);
        addr7 = sys_malloc(size);
        addr8 = sys_malloc(size);
        addr9 = sys_malloc(size);
        sys_free(addr1);
        sys_free(addr2);
        sys_free(addr3);
        sys_free(addr4);
        sys_free(addr5);
        sys_free(addr6);
        sys_free(addr7);
        sys_free(addr8);
        sys_free(addr9);
    }

    console_put_str(" thread_a end \n");
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