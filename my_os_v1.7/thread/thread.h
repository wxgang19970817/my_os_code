/*************************************************************************
	> File Name: thread.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月09日 星期四 20时38分04秒
 ************************************************************************/

#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H

#include "../lib/stdint.h"
#include "../lib/kernel/list.h"
#include "bitmap.h"
#include "../kernel/memory.h"


#define MAX_FILES_OPEN_PER_PROC     8               /* 每个任务可以打开文件数是8 */

/* 自定义通用函数类型，用来在线程函数中作为形参类型 */
typedef void thread_func(void*);
typedef int16_t pid_t;

/* 进程或线程状态 */
enum task_status
{
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
};


/************** 中断栈　intr_stack ************************
 * 此结构用于中断发生时保护程序（线程或进程）的上下文环境
 * 进程或者线程被外部中断或软中断打断时，会按照此结构压入上下文
 * 寄存器，intr_exit中的出栈操作是此结构的逆操作
 * 此栈在线程自己的内核栈中位置固定，在页的最顶端
 *********************************************************/
struct intr_stack
{
    uint32_t vec_no;    /* kernel.S 宏VECTOR中为了调试方便push %1压入的中断号 */
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy; /* popad的时候会忽略不断变化的esp */
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t  gs;
    uint32_t  fs;
    uint32_t  es;
    uint32_t  ds;

    /* 以下由cpu从低特权级进入高特权级时压入的 */
    uint32_t err_code;
    void(*eip)(void);
    uint32_t cs;
    uint32_t eflags;
    void* esp;
    uint32_t ss;
};


/****************** 线程栈　thread_stack ********************
 * 线程自己的栈，用于存储线程中待执行的函数,eip便是该函数的地址
 * 此结构在线程自己的内核栈中的位置不固定
 * 仅用在switch_to时保存线程环境
 * 实际位置取决于实际运行环境
 ************************************************************/
struct thread_stack
{
    /* 根据编译时的ABI(应用程序二进制接口)，ebp,ebx,edi,esi,esp归主调函数所有，在发生切换的时候要入栈保存 */
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    /* 函数第一次执行时,eip指向待调用的函数kernel_thread
     * 其他时候,eip是指向switch_to的返回地址*/
    void (*eip)(thread_func *func,void* func_arg);

    /***** 以下仅供第一次被调度上cpu时使用　******/

    /* 参数 usused_ret 只为占位置充数为返回地址,在函数返回的时候不用这个当返回地址，只为了保证被调函数找参数的时候能找对 */
    void(*unused_retaddr);
    thread_func *function;          /* 由kernel_thread所调用的函数名 */
    void *func_arg;                 /* 由kernel_thread所调用的函数所需的参数 */
};

/* 进程或线程的pcb,程序控制块 */
struct task_struct
{
    uint32_t* self_kstack;          /* 各内核线程都用自己的内核栈 */
    pid_t pid;
    enum task_status status;        /* 类型便是前面的枚举结构enum task_status */
    char name[16];                  /* 记录任务（线程或进程）的名字，最长不超过16个字符 */
    uint8_t priority;               /* 线程优先级 */
    uint8_t ticks;                  /* 每次在处理器上执行的时间嘀嗒数。每次时钟中断都会减１ */

    uint32_t elapsed_ticks;         /* 记录任务从开始到结束的总时钟数 */

    int32_t fd_table[MAX_FILES_OPEN_PER_PROC];          /* 文件描述符数组，该数组的下标就是文件描述符，数组里面存储的是文件结构表的下标 */
    /* general_tag 用于线程在一般队列中的结点 */
    struct list_elem general_tag;

    /* 线程队列 thread_all_list  */
    struct list_elem all_list_tag;

    uint32_t* pgdir;                /* 用于存放进程自己页目录表的虚拟地址，这个地址会加载到cr3，加载时由此虚拟地址转变为物理地址 */

    struct virtual_addr userprog_vaddr;     /* 用户进程的虚拟地址池 */

    struct mem_block_desc u_block_desc[DESC_CNT];   /* 用户进程内存块描述符 */

    uint32_t cwd_inode_nr;          /* 进程所在的工作目录的inode编号 */

    int16_t parent_pid;                    /* 父进程pid */

    /* PCB和0级栈都是在同一个页中，栈位于页的顶端，并向下发展，要保证栈不会把PCB的内容覆盖 */
    uint32_t stack_magic;           /* 栈的边界标记，用于检测栈的溢出，其实就是个魔数，每次都检查是否为初始值 */
};
  
extern struct list thread_ready_list;
extern struct list thread_all_list;

void thread_create(struct task_struct* pthread, thread_func function, void* func_arg);
void init_thread(struct task_struct* pthread, char* name, int prio);
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg);
struct task_struct* running_thread(void);
void schedule(void);
void thread_init(void);
void thread_block(enum task_status stat);
void thread_unblock(struct task_struct* pthread);
void thread_yield(void);
pid_t fork_pid(void);

#endif
