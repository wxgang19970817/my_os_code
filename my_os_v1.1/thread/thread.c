/*************************************************************************
	> File Name: thread.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月10日 星期五 09时02分26秒
 ************************************************************************/

#include "thread.h"
#include "../lib/stdint.h"
#include "string.h"
#include "../kernel/global.h"
#include "memory.h"

#define PG_SIZE 4096

/* 由kernel_thread去执行function(func_arg) */
static void kernel_thread(thread_func* function,void* func_arg)
{
    function(func_arg);
}





/* 初始化线程栈thread_stack,将待执行的函数和参数放到thread_stack中相应的位置 */
void thread_create(struct task_struct* pthread,thread_func function,void* func_arg)
{
    /* 先预留中断使用栈的空间，先前初始化过现在再更改 */
    /* 中断栈用来线程进入中断后保存上下文和存放用户进程的初始信息 */
    pthread->self_kstack -= sizeof(struct intr_stack);  /* 原来在pcb的最顶端，现在往下挪预留出中断栈的空间 */
    
    /* 再留出线程栈空间 */
    /* 线程栈用来存储线程中待执行的函数和保存线程环境 */
    pthread->self_kstack -= sizeof(struct thread_stack);

    /* 下面为0级特权线程栈赋值 */
    struct thread_stack* kthread_stack = (struct thread_stack*)pthread->self_kstack;

    kthread_stack->eip = kernel_thread; 
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;
    kthread_stack->ebp = kthread_stack->ebx = kthread_stack->esi = kthread_stack->edi = 0;
}


/* 初始化线程基本信息 */
void init_thread(struct task_struct* pthread,char* name,int prio)
{
    /* 将pcb清零 */
    memset(pthread,0,sizeof(*pthread));
    
    /* 将线程名字写入pcb中的name数组 */
    strcpy(pthread->name,name);

    /* 状态赋值 */
    pthread->status = TASK_RUNNING;

    /* 优先级 */
    pthread->priority = prio;

    /* self_stack 是线程在内核态下使用的栈顶地址 */
    pthread->self_kstack = (uint32_t *)((uint32_t)pthread + PG_SIZE);  /* pcb顶部做栈底 */

    /* 边界检查的魔数会被安排在pbc结构体和栈顶的交界处 */
    pthread->stack_magic = 0x19870916; 
}



/* 创建一优先级为prio的线程，线程名为name，线程所执行的函数是function(func_arg) */
struct task_struct* thread_start(char* name,int prio,thread_func function,void* func_arg)
{
    /* pcb都位于内核空间，包括用户进程的pcb也是在内核空间 */
    struct task_struct* thread = get_kernel_pages(1);       /* 在内核中申请一页作为线程pcb */

    /* 初始化线程，其实就是初始化pcb结构体和安排线程的内核栈 */
    init_thread(thread,name,prio);

    thread_create(thread,function,func_arg);

    /* esp指向线程内核栈底即线程栈顶，根据线程栈的结构， */
    asm volatile("movl %0,%%esp;pop %%ebp;pop %%ebx;pop %%edi;pop %%esi;ret"::"g"(thread->self_kstack):"memory");
    return thread;
}
