/*************************************************************************
	> File Name: ioqueue.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月16日 星期四 14时15分14秒
 ************************************************************************/

#include "ioqueue.h"
#include "../kernel/interrupt.h"
#include "../kernel/global.h"
#include "../kernel/debug.h"



/* 初始化io队列ioq */
void ioqueue_init(struct ioqueue* ioq)
{
    lock_init(&ioq->lock);  /* 初始化io队列的锁 */
    ioq->producer = ioq->consumer = NULL;   /* 生产者消费者置空 */
    ioq->head = ioq->tail = 0;  /* 队列的首尾指针指向缓冲区数组第0个位置 */
}

/* 返回pos在缓冲区中的下一个位置值 */
static int32_t next_pos(int32_t pos)
{
    /* +1后对bufsize求模 */
    return (pos + 1) % bufsize;
}

/* 判断队列是否已满 */
bool ioq_full(struct ioqueue* ioq)
{
    ASSERT(intr_get_status() == INTR_OFF);
    return next_pos(ioq->head) == ioq->tail;
}

/* 判断队列是否为空 */
static bool ioq_empty(struct ioqueue* ioq)
{
    ASSERT(intr_get_status() == INTR_OFF);
    return ioq->head == ioq->tail;
}

/* 使当前生产者或消费者在此缓冲区上等待 */
/* waiter pcb类型的二级指针　传入的实参是线程指针的地址,缓冲区中的成员producer或consumer */
static void ioq_wait(struct task_struct** waiter)
{
    ASSERT(*waiter == NULL && waiter != NULL);
    /* 将当前线程记录在ioq->consumer或ioq->producer */
    *waiter = running_thread();
    /* 将当前线程阻塞 */
    thread_block(TASK_BLOCKED);
}

/* 唤醒waiter */
static void wakeup(struct task_struct** waiter)
{
    ASSERT(*waiter != NULL);
    thread_unblock(*waiter);
    *waiter = NULL;
}

/* 消费者从ioq队列队尾中获取一个字符,我们对缓冲区的操作的数据单位是1字节 */
char ioq_getchar(struct ioqueue*  ioq)
{
    ASSERT(intr_get_status() == INTR_OFF);

    /* 若缓冲区为空，把消费者ioq->consumer阻塞 */
    while(ioq_empty(ioq))
    {
        lock_acquire(&ioq->lock);
 
        /* 除了阻塞自己，还把当前线程记为ioq->consumer,将来生产者往缓冲区装入商品后，就知道要唤醒哪个消费者，即唤醒当前线程 */
        ioq_wait(&ioq->consumer);
 
        /* 醒来后释放锁 */
        lock_release(&ioq->lock);
    }

    /* 从缓冲区中取出 */
    char byte = ioq->buf[ioq->tail];

    /* 把游标移到下一位置 */
    ioq->tail = next_pos(ioq->tail);

    /* 如果不为空，那代表着之前有生产者线程往缓冲区中添加数据但是因为缓冲区满而被阻塞 */
    if(ioq->producer != NULL)
    {
        /* 此时取出一个数据单位，留出了空间，要唤醒阻塞生产者，添加数据 */
        wakeup(&ioq->producer);
    }

    return byte;
}


/* 生产者往 ioq 队列中写入一个字符 byte */
void ioq_putchar(struct ioqueue* ioq,char byte)
{
    ASSERT(intr_get_status() == INTR_OFF);

    /* 若是缓冲区(队列)已经满了，把生产者ioq->producer记为当前线程，方便阻塞后被唤醒 */
    while(ioq_full(ioq))
    {
        lock_acquire(&ioq->lock);
        ioq_wait(&ioq->producer);
        lock_release(&ioq->lock);
    }

    /* 把字节放入缓冲区中 */
    ioq->buf[ioq->head] = byte;

    /* 把写游标移到下一位置 */
    ioq->head = next_pos(ioq->head);

    if(ioq->consumer != NULL)
    {
        wakeup(&ioq->consumer);
    }
}
