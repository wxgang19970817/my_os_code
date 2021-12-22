/*************************************************************************
	> File Name: process.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月18日 星期六 19时33分17秒
 ************************************************************************/
#include "process.h"
#include "global.h"
#include "debug.h"
#include "../kernel/memory.h"
#include "thread.h"    
#include "list.h"    
#include "tss.h"    
#include "interrupt.h"
#include "string.h"
#include "console.h"

extern void intr_exit(void);

/* 构建用户进程初始上下文信息 */
/* 接收的参数表示用户程序的名称，用户程序肯定是从文件系统中加载到内存中的，因此进程名是进程的文件名 */
/* 此函数用来创建用户进程filename的上下文也就是填充用户进程的struct intr_stack */
void start_process(void* filename_)
{
	void* function = filename_;

	/* 此时指向PCB的顶端 */
	struct task_struct* cur = running_thread();

	/* 越过了最顶端的intr_stack 此时指向intr_stack的低端 */
	/* intr_stack的作用是:任务被中断时，用来保存任务的上下文，二是为了给用户进程填充上下文，即寄存器环境 */
	/* thread_stack是在线程创建之初用来保存根据ABI规定的需要保存的寄存器，以及返回地址(待执行的函数)*/
	cur->self_kstack += sizeof(struct thread_stack);

	struct intr_stack* proc_stack = (struct intr_stack*)cur->self_kstack;

	proc_stack->edi = proc_stack->ebp = proc_stack->esi = proc_stack->esp_dummy = 0;

	proc_stack->ebx = proc_stack->edx = proc_stack->ecx = proc_stack->eax = 0;

	proc_stack->gs = 0;					/* 用户态用不上，直接初始为0 */

	proc_stack->ds = proc_stack->es = proc_stack->fs  =  SELECTOR_U_DATA;     /* 用户态代码段 */

	proc_stack->eip = function;				/* 待执行的用户程序地址 */

	proc_stack->cs = SELECTOR_U_CODE;  

	proc_stack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);

	proc_stack->esp = (void*)((uint32_t)get_a_page(PF_USER,USER_STACK3_VADDR) + PG_SIZE);   /* 为用户进程分配3特权级下的栈,0xc0000000 - 0x1000是栈顶 */

	proc_stack->ss = SELECTOR_U_DATA;

	/* 跳转到intr_exit,在那儿一系列的pop和iretd,将proc_stack中的数据载入CPU的寄存器，开始执行向执行的函数进入了特权级3 */
	asm volatile ("movl %0,%%esp;jmp intr_exit" : : "g"(proc_stack) : "memory");
}

/* 激活页表*/
void page_dir_activate(struct task_struct* p_thread)
{
	/* 执行此函数时，当前任务可能是线程，之所以要对线程也重新安装页表，原因是上一次被调度的可能是进程
	如果不恢复页表的话，线程就会使用进程的页表了 */

	/* 若为内核级线程，需要重新填充页表为0x100000,这是内核所使用的页表的物理地址 */
	uint32_t pagedir_phy_addr = 0x100000;
	if(p_thread->pgdir != NULL)
	{
		pagedir_phy_addr = addr_v2p((uint32_t)p_thread->pgdir);
	}

	/* 更新页目录寄存器cr3,使新页表生效 */
	asm volatile ("movl %0,%%cr3": :"r"(pagedir_phy_addr) : "memory");
}

/* 调用页表激活函数，然后更新tss中的esp0为进程的特权级0的栈 */
void process_activate(struct task_struct* p_thread)
{
	ASSERT(p_thread != NULL);
	page_dir_activate(p_thread);

	/* 内核线程特权级本身就是0,处理器进入中断时并不会从tss中获取0特权级栈地址，故不需要更新esp0 */
	if(p_thread->pgdir)
	{
		/* 更新该进程的esp0,用于此进程被中断时保留上下文 */
		update_tss_esp(p_thread);
	}
}

/* 创建页目录表，将当前页表的表示内核空间的pde复制，成功则返回页目录的虚拟地址，否则返回-1 */
uint32_t* create_page_dir(void)
{
	/* 用户进程的页表不能让用户直接访问到，所以在内核空间来申请 */
	uint32_t* page_dir_vaddr = get_kernel_pages(1);

	if(page_dir_vaddr == NULL)
	{
		console_put_str("create_page_dir: get_kernel_page failed!");
		return NULL;
	}

	/*************　把内核的页目录项复制到用户进程使用的页目录表中　***************/
	/* page_dir_vaddr + 0x300*4 是内核页目录的第768项 */
	/* 0xffff000 是用来访问内核页目录表的基地址，1024是要复制的字节量，相当于复制了1024/4=256个页目录项 */
	memcpy((uint32_t*)((uint32_t)page_dir_vaddr + 0x300*4),(uint32_t*)(0xfffff000 + 0x300*4),1024);

	/*************　把用户页目录表中的最后一个页目录项更新为用户进程自己的页目录表的物理地址 ***********/
	uint32_t new_page_dir_phy_addr = addr_v2p((uint32_t)page_dir_vaddr);

	page_dir_vaddr[1023] = new_page_dir_phy_addr | PG_US_U | PG_RW_W | PG_P_1;

	return page_dir_vaddr;
}

/* 用户进程可以在自己的堆中申请、释放内存，所以必须能有一套方法跟踪内存的分配情况 */
/* 因此我们也要利用位图管理实现堆内存管理，其实就是创建用户进程的虚拟地址位图结构体 user_prog->userprog_vaddr */
void create_user_vaddr_bitmap(struct task_struct* user_prog)
{
	/* 位图中所管理的内存空间的起始地址 */
	user_prog->userprog_vaddr.vaddr_start = USER_VADDR_START;

	/* 位图需要的内存页框数 */
	uint32_t bitmap_pg_cnt = DIV_ROUND_UP((0xc0000000 - USER_VADDR_START)/PG_SIZE/8,PG_SIZE);

	/* 返回地址记录在该位图指针中 */
	user_prog->userprog_vaddr.vaddr_bitmap.bits = get_kernel_pages(bitmap_pg_cnt);

	user_prog->userprog_vaddr.vaddr_bitmap.btmp_bytes_len = ((0xc0000000 - USER_VADDR_START)/PG_SIZE/8);

	bitmap_init(&user_prog->userprog_vaddr.vaddr_bitmap);
}

/* 创建用户进程 */
void process_execute(void* filename,char* name)
{
	/* pcb内核的数据结构，由内核来维护进程信息，因此要在内核内存池中申请 */
	struct task_struct* thread = get_kernel_pages(1);

	init_thread(thread,name,default_prio);

	create_user_vaddr_bitmap(thread);

	thread_create(thread,start_process,filename);

	thread->pgdir = create_page_dir();

	enum intr_status old_status = intr_disable();

	ASSERT(!elem_find(&thread_ready_list,&thread->general_tag));
	list_append(&thread_ready_list,&thread->general_tag);

	ASSERT(!elem_find(&thread_all_list,&thread->all_list_tag));
	list_append(&thread_all_list,&thread->all_list_tag);
	intr_set_status(old_status);
}


