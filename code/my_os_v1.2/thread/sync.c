/*************************************************************************
	> File Name: sync.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月15日 星期三 09时42分43秒
 ************************************************************************/

#include "sync.h"
#include "../kernel/global.h"
#include "../kernel/debug.h"
#include "../kernel/interrupt.h"
#include "thread.h"

/* 初始化信号量 */
void sema_init(struct semaphore* psema,uint8_t value)
{
    psema->value = value;           /* 信号量赋初值 */
    list_init(&psema->waiters);     /* 初始化信号量的等待队列 */
}

/* 初始化锁 plock */
void lock_init(struct lock* plock)
{
    plock->holder = NULL;
    plock->holder_repeat_nr = 0;
    sema_init(&plock->semaphore, 1); 
}


/* 信号量down操作 */
void sema_down(struct semaphore* psema)
{
    /* 关中断来保证原子操作 */
    enum intr_status old_status = intr_disable();

    
    /* value为0 代表已经被人持有 */
    while(psema->value == 0)
    {
        

        ASSERT(!elem_find(&psema->waiters,&running_thread()->general_tag));
  
        /* 当前线程不应该在信号量的 waiters队列中 */
        if(elem_find(&psema->waiters,&running_thread()->general_tag))
        {
            PANIC("sema_down:thread blocked has been in waiters_list\n");
        } 
        list_append(&psema->waiters,&running_thread()->general_tag);
        
        thread_block(TASK_BLOCKED);
    }

    /* 若value值为1或被唤醒后，会执行下面的代码，也就是获得了锁 */
    psema->value --;
    ASSERT(psema->value == 0);
    intr_set_status(old_status);
}

/* 信号量up操作 */
void sema_up(struct semaphore* psema)
{
    /* 关中断，保证原子操作 */
    enum intr_status old_status = intr_disable();

    ASSERT(psema->value == 0);

    if(!list_empty(&psema->waiters))
    {
        struct task_struct* thread_blocked = elem2entry(struct task_struct,general_tag,list_pop(&psema->waiters));
        thread_unblock(thread_blocked);
    }
    psema->value++;
    ASSERT(psema->value == 1);
    /* 恢复之前中断的状态 */
    intr_set_status(old_status);
}


/* 获取锁 plock */
void lock_acquire(struct lock* plock)
{
    /* 排除曾经自己已经持有锁但还未将其释放的情况 */
    if(plock->holder != running_thread())
    {
        /* 获取锁或者说获取信号量　即在sema_down如果失败，自己会阻塞自己，不过早晚会成功返回 */
        sema_down(&plock->semaphore); /* 获取信号量，原子操作 */
        plock->holder = running_thread();
        ASSERT(plock->holder_repeat_nr == 0);
        plock->holder_repeat_nr = 1;
    }
    else
    {
        plock->holder_repeat_nr++;
    }
}


/* 释放锁　plock */
void lock_release(struct lock* plock)
{
    ASSERT(plock->holder == running_thread());
    if(plock->holder_repeat_nr > 1)
    {
        plock->holder_repeat_nr--;
        return;
    }
    ASSERT(plock->holder_repeat_nr == 1);

    plock->holder = NULL;
    plock->holder_repeat_nr = 0;
    
    /* 释放锁并不在关中断的情况下进行，锁的持有者置为空必须在释放信号量之前进行 */
    sema_up(&plock->semaphore);         /*　释放信号量 */
}



