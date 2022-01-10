/*************************************************************************
	> File Name: sync.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月15日 星期三 09时30分51秒
 ************************************************************************/

#ifndef __THREAD_SYNC_H
#define __THREAD_SYNC_H
#include "../lib/kernel/list.h"
#include "../lib/stdint.h"
#include "thread.h"

/* 信号量结构 */
struct semaphore
{
    uint8_t value;                  /* 信号量要有初值 */
    struct  list waiters;           /* 记录在此信号量上等待(阻塞)的所有线程 */
};

/* 锁结构 */
struct lock
{
    struct task_struct* holder;     /* 锁的持有者 */
    struct semaphore semaphore;     /* 信号量结构中的初值被赋值为1，二元信号量实现锁 */
    uint32_t holder_repeat_nr;      /* 有时候持有了某临界区的锁，未释放前有可能会再次重复调用重复申请此锁的函数，这样内外层函数在释放锁时会对同一个锁释放两次 */
};

void sema_init(struct semaphore* psema, uint8_t value); 
void sema_down(struct semaphore* psema);
void sema_up(struct semaphore* psema);
void lock_init(struct lock* plock);
void lock_acquire(struct lock* plock);
void lock_release(struct lock* plock);

#endif
