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
#include "../kernel/memory.h"
#include "../kernel/interrupt.h"
#include "../kernel/debug.h"
#include "../userprog/process.h"

#define PG_SIZE 4096

struct task_struct* main_thread;        /* 主线程PCB */
struct list thread_ready_list;          /* 就绪队列 */
struct list thread_all_list;            /* 所有任务队列 */
static struct list_elem* thread_tag;    /* 用于保存队列中的线程结点 */

extern void switch_to(struct task_struct* cur,struct task_struct* next);


/* 获取当前线程pcb指针 */
struct task_struct* running_thread()
{
    uint32_t esp;
    asm("mov %%esp,%0":"=g"(esp));

    /* 各个线程的0特权级栈都在自己的pcb中， 取esp整数部分，即pcb的起始地址 */
    return (struct task_struct*)(esp & 0xfffff000);
}


/* 由kernel_thread去执行function(func_arg) */
static void kernel_thread(thread_func* function,void* func_arg)
{
    /* 执行function前要开中断，避免后面的时钟中断被屏蔽，而无法调度其他线程 */
    intr_enable();
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

    if(pthread == main_thread)
    {
        /* 由于把main函数也封装成一个线程，并且它一直是运行的 */
        /* 状态赋值 */
        pthread->status = TASK_RUNNING;
    }
    else 
    {
        pthread->status = TASK_READY;
    }
    /* 优先级 */
    pthread->priority = prio;

    /* 时间片 */
    pthread->ticks = prio;

    /* 执行时间 */
    pthread->elapsed_ticks = 0;

    /* 线程没有自己的虚拟地址空间 */
    pthread->pgdir = NULL;

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

    /* 在加入就绪队列之前确保不在队列中 */
    ASSERT(!elem_find(&thread_ready_list,&thread->general_tag));

    /* 加入就绪队列 */
    list_append(&thread_ready_list, &thread->general_tag);

    /* 确保不在全部队列中 */
    ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));

    /* 加入全部线程队列 */
    list_append(&thread_all_list, &thread->all_list_tag);

    return thread;

}

/* 将kernel中的main函数完善为主线程 */
static void make_main_thread(void)
{
    /* main线程早已运行，但是还没有PCB，在loader.S中的mov esp,0xc009f000
     * 就是为其预留pcb的，因此pcb地址为0xc009e000*/
    main_thread = running_thread();
    init_thread(main_thread,"main",31);

    /* main函数是当前线程，当前线程不在thread_ready_list中，所以只将其加在thread_all_list中 */
    /* 就绪队列只存储准备运行的线程，全部队列包括就绪的、阻塞的、正在执行的等等 */
    ASSERT(!elem_find(&thread_all_list,&main_thread->all_list_tag));
    list_append(&thread_all_list,&main_thread->all_list_tag);
    
}



/* 实现任务调度 */
void schedule()
{
    ASSERT(intr_get_status() == INTR_OFF);

    struct task_struct* cur = running_thread();

    if(cur->status == TASK_RUNNING)
    {
        /* 若此线程只是cpu时间片到了，将其加入到就绪队列尾 */
        ASSERT(!elem_find(&thread_ready_list,&cur->general_tag));
        list_append(&thread_ready_list,&cur->general_tag);
        cur->ticks = cur->priority;
        cur->status = TASK_READY;
    }
    else
    {
        /* 若此线程需要某事件发生后才能继续上cpu运行，不需要将其加入队列，因为当前线程不在就绪队列 */
    }

    /* 避免无线程可调度的情况，暂时用此方法保证，后续会更改 */
    ASSERT(!list_empty(&thread_ready_list));

    thread_tag = NULL;  /* thread_tag是全局变量，使用前习惯性清空 */
    
    /* 将就绪队列中的第一个就绪线程的PCB中的general_tag或all_list_tag弹出；准备调度上cpu */
    thread_tag = list_pop(&thread_ready_list);

    struct task_struct* next = elem2entry(struct task_struct, general_tag, thread_tag);

    next->status = TASK_RUNNING;

    /* 激活任务页表等 */
    process_activate(next);

    switch_to(cur,next);
}


/* 当前线程将自己阻塞，标志其状态位stat */
void thread_block(enum task_status stat)
{
    /* stat 取值为TASK_BLOCKDE,TASK_WAITING,TASK_HANGING,阻塞、等待、挂起这三种状态下不会被调度 */
    ASSERT((stat == TASK_BLOCKED) || (stat == TASK_HANGING) || (stat == TASK_WAITING));

    enum intr_status old_status = intr_disable();

    struct task_struct* cur_thread = running_thread();

    cur_thread->status = stat;   /* 将当前线程的状态设为函数输入参数的状态 */

    schedule();                 /* 开始调度，状态不符合，会被换下处理器 */

    /* 待当前线程被解除阻塞后才继续运行下面的intr_set_status */
    intr_set_status(old_status);
}



/* 将线程pthread解除阻塞 */
void thread_unblock(struct task_struct* pthread)
{
    enum intr_status old_status = intr_disable();

    ASSERT((pthread->status == TASK_BLOCKED) || (pthread->status == TASK_HANGING) || (pthread->status == TASK_WAITING));

    if(pthread->status != TASK_READY)
    {
        ASSERT(!(elem_find(&thread_ready_list,&pthread->general_tag)));
        if(elem_find(&thread_ready_list,&pthread->general_tag))
        {
            PANIC("thread_unblock:blocked thread in ready_list\n");
        }

        list_push(&thread_ready_list,&pthread->general_tag);  /* 放到队列的最前面，使其尽快得到调度 */

        pthread->status = TASK_READY;
    }

    intr_set_status(old_status);
}



/* 初始化线程环境 */
void thread_init(void)
{
    put_str("thread_init start\n");
    list_init(&thread_all_list);
    list_init(&thread_ready_list);
    
    /* 将当前main函数创建为线程 */
    make_main_thread();
    put_str("thread_init done\n");
}

