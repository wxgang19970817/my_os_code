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

/* 内存池标记，用于判断用哪个内存池 */
enum pool_flags
{
    PF_KERNEL = 1,              /* 内核内存池 */
    PF_USER = 2                 /* 用户内存池 */
};



/* 内存池结构体，实例化之后用于管理内核内存池和用户内存池 */
/* 与在memory.h中定义的虚拟地址池　virtual_addr 相比多了一个成员用来描述有限的物理内存量 */
struct pool
{
    struct bitmap pool_bitmap;      /* 本内存池用到的位图结构 */
    uint32_t phy_addr_start;        /* 本内存池管理的物理内存的起始地址 */
    uint32_t pool_size;             /* 本内存池的字节容量 */
};




#define PG_P_1 1            /* 页表项或页目录项存在属性位  */
#define PG_P_0 0            /* 页表项或页目录项存在属性位 */
#define PG_RW_R 0           /* R/W 属性位值　读/执行 */
#define PG_RW_W 2           /* R/W 属性位值　读/写/执行 */
#define PG_US_S 0           /* U/S 属性位值，系统级 */
#define PG_US_U 4           /* U/S 属性位值，用户级 */

extern struct pool kenel_pool,user_pool;
void mem_init(void);


#endif
