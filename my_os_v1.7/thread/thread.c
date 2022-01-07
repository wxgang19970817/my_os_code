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
#include "sync.h"
#include "stdio.h"
#include "file.h"
#include "fs.h"



struct task_struct* main_thread;        /* 主线程PCB */
struct task_struct* idle_thread;            /* idle线程 */
struct list thread_ready_list;          /* 就绪队列 */
struct list thread_all_list;            /* 所有任务队列 */
struct lock pid_lock;                       /* 分配pid锁 */
static struct list_elem* thread_tag;    /* 用于保存队列中的线程结点 */

extern void switch_to(struct task_struct* cur,struct task_struct* next);
extern void init(void);

/* 系统空闲时运行的线程 */
/* 定义了函数未使用的话编译器会给出警告　__attrite__(unused)表示忽略此警告　 */
static  void idle(void* arg UNUSED)
{
    while(1)
    {
        thread_block(TASK_BLOCKED);
        /* 执行hlt时必须要保证目前处在开中断的情况下 */
        asm volatile("sti;hlt":::"memory");
    }
}


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


/* 利用锁实现分配pid时的互斥，避免不同任务分配重复的pid */
static pid_t allocate_pid(void)
{
    static pid_t next_pid = 0;
    lock_acquire(&pid_lock);    
    next_pid++;
    lock_release(&pid_lock);
    return next_pid;
}



/* fork进程时，为其分配pid,因为allocate_pid已经是静态的，别的文件无法调用，不想改变函数的定义
  * 故定义fork_pid函数来封装一下 */
pid_t fork_pid(void)
{
    return allocate_pid();
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
    
    /* 给线程分配pid */
    pthread->pid = allocate_pid();

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

    /* 预留标准输入输出 */
    pthread->fd_table[0] = 0;       /* 标准的文件描述符，0是标准输入 */
    pthread->fd_table[1] = 1;       /* 1是标准输出 */
    pthread->fd_table[2] = 2;       /* 2是标准错误 */

    /* 其余全置为-1 */
    uint8_t fd_idx = 3;
    while(fd_idx < MAX_FILES_OPEN_PER_PROC)
    {
        pthread->fd_table[fd_idx] = -1;         /* -1表示该文件描述符可分配 */
        fd_idx++;
    }

    pthread->cwd_inode_nr = 0;          /* 以根目录作为默认工作路径 */

    pthread->parent_pid = -1;                  /* -1表示没有父进程 */ 

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

    /* 如果就绪队列中没有可运行的任务，就唤醒idle */
    if(list_empty(&thread_ready_list))
    {
        thread_unblock(idle_thread);
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



/* 主动让出cpu，换其他线程运行 */
void thread_yield(void)
{
    struct task_struct* cur = running_thread();
    enum intr_status old_status = intr_disable();

    ASSERT(!elem_find(&thread_ready_list,&cur->general_tag));

    list_append(&thread_ready_list,&cur->general_tag);
    cur->status = TASK_READY;

    schedule();
    intr_set_status(old_status);
}



/* 以填充空格的方式对齐输出buf,无论字符串ptr是多少字符，永远输出buf_len长度，如果长度不足buf_len,就以空格填充 */
static void pad_print(char* buf,int32_t buf_len,void* ptr,char format)
{
    memset(buf,0,buf_len);
    uint8_t out_pad_0idx = 0;
    switch(format)
    {
        case 's':
            out_pad_0idx = sprintf(buf,"%s",ptr);
            break;
        case 'd':
            out_pad_0idx = sprintf(buf,"%d",*((int16_t*)ptr));
        case 'x':
            out_pad_0idx = sprintf(buf,"%x",((uint32_t*)ptr));
    }
    while(out_pad_0idx < buf_len)
    {
        /* 以空格填充 */
        buf[out_pad_0idx] = ' ';
        out_pad_0idx++;
    }
    sys_write(stdout_no,buf,buf_len -1);
}





/* elem2thread_info 用于打印任务信息，用于在list_traversal函数中的回调函数，用于针对线程队列的处理 */
static bool elem2thread_info(struct list_elem* pelem,int arg UNUSED)
{
    struct task_struct* pthread = elem2entry(struct task_struct,all_list_tag,pelem);
    char out_pad[16] = {0};

    pad_print(out_pad,16,&pthread->pid,'d');

    if(pthread->parent_pid == -1)
    {
        pad_print(out_pad,16,"NULL",'s');
    }
    else
    {
        pad_print(out_pad,16,&pthread->parent_pid,'d');
    }

    switch(pthread->status)
    {
        case 0:
            pad_print(out_pad,16,"RUNNING",'s');
            break;
        case 1:
            pad_print(out_pad,16,"READY",'s');
            break;
        case 2:
            pad_print(out_pad,16,"BLOCKED",'s');
            break;
        case 3:
            pad_print(out_pad,16,"WAITING",'s');
            break;
        case 4:
            pad_print(out_pad,16,"HANGING",'s');
            break;
        case 5:
            pad_print(out_pad,16,"DIED",'s');
    }
    pad_print(out_pad,16,&pthread->elapsed_ticks,'x');

    memset(out_pad,0,16);

    ASSERT(strlen(pthread->name) < 17);
    memcpy(out_pad,pthread->name,strlen(pthread->name));
    strcat(out_pad,"\n");
    sys_write(stdout_no,out_pad,strlen(out_pad));
    return false;           /* 此处返回false是为了迎合主调函数list_traversal,只有回调函数返回false时才会继续调用此函数 */
}



/* 打印任务列表 */
void sys_ps(void)
{
    char* ps_title = "PID       PPID        STAT           TICKS        COMMAND\n";
    sys_write(stdout_no,ps_title,strlen(ps_title));
    list_traversal(&thread_all_list,elem2thread_info,0);
}



/* 初始化线程环境 */
void thread_init(void)
{
    put_str("thread_init start\n");
    list_init(&thread_all_list);
    list_init(&thread_ready_list);
    lock_init(&pid_lock);

    /* 先创建第一个用户进程:init,以保证pid为1 */
    process_execute(init,"init");

    /* 将当前main函数创建为线程 */
    make_main_thread();

    /* 创建idle线程 */
    idle_thread = thread_start("idle",10,idle,NULL);
    put_str("thread_init done\n");
}

