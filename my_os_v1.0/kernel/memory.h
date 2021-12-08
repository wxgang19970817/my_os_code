/*************************************************************************
	> File Name: memory.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月07日 星期二 15时48分13秒
 ************************************************************************/

#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H

#include "../lib/stdint.h"
#include "../lib/kernel/bitmap.h"

/* 虚拟地址池，用于虚拟地址管理 */
struct virtual_addr
{
    struct bitmap vaddr_bitmap;     /* 虚拟地址用到的位图结构 */
    uint32_t vaddr_start;           /* 虚拟地址起始地址 */
};


/* 内存池结构体，实例化之后用于管理内核内存池和用户内存池 */
/* 与在memory.h中定义的虚拟地址池　virtual_addr 相比多了一个成员用来描述有限的物理内存量 */
struct pool
{
    struct bitmap pool_bitmap;      /* 本内存池用到的位图结构 */
    uint32_t phy_addr_start;        /* 本内存池管理的物理内存的起始地址 */
    uint32_t pool_size;             /* 本内存池的字节容量 */
};





extern struct pool kenel_pool,user_pool;
void mem_init(void);


#endif
