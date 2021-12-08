/*************************************************************************
	> File Name: memory.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月07日 星期二 15时54分44秒
 ************************************************************************/

#include "memory.h"
#include "../lib/stdint.h"
#include "../lib/kernel/print.h"


#define PG_SIZE 4096

/********************************* 位图地址　******************************
 * 0xc009f000 是现在的内核栈顶地址,0xc0009e000 是内核主线程的pcb
 * 用4页的内存做位图，可以管理512MB的内存大小，粒度为4K
 * 0xc0009e000 - 0x4000 = 0xc0009a00 是位图的起始地址
 * ************************************************************************/
#define MEM_BITMAP_BASE 0xc009a000

/* 0xc0000000 是内核的虚拟地址的起始　0xc0000000~0xc00fffff 已经映射到了物理
 * 地址的低1MB空间内　这里 0xc0010000 作为内核堆空间起始的虚拟地址　　
 * 注：物理地址的0x10000~0x101fff是在loader.S中定义好的页目录及页表，不能映射*/
#define  K_HEAP_START 0xc0100000




/* 物理内存池结构体实例化 */
struct pool kernel_pool,user_pool;  
/* 给内核分配虚拟地址 */
struct virtual_addr kernel_vaddr;     

/* 初始化内存池 */
static void mem_pool_init(uint32_t all_mem)
{
    put_str(" mem_pool_init start\n");

    /* 1页的目录表 + 第0和第768个页目录项指向同一个页表 + 第769~1022个页目录项共指向254个页表，共256个页表 */
    uint32_t page_table_size = PG_SIZE * 256;

    /* 记录已经使用的内存字节数　包括页表大小和低端0x100000字节的内存 */
    uint32_t used_mem = page_table_size + 0x100000;

    /* 总内存　all_mem */
    uint32_t free_mem = all_mem - used_mem;

    /* 如果总内存不是4k的倍数，不足4k的余数不作考虑 */
    uint16_t all_free_pages = free_mem / PG_SIZE;

    /* 存储分配给内核的空闲物理页 */
    uint16_t kernel_free_pages = all_free_pages / 2;

    /* 给内核分配完了之后剩余的空闲物理页分配给用户 */
    uint16_t user_free_pages = all_free_pages - kernel_free_pages;

    /* 记录位图的长度，余数不作处理，坏处是会丢失部分内存，好处是不用做越界检查，因为位图表示的内存少于实际物理内存 */
    /* Kernel BitMap 的长度，位图中的一位表示一页，以字节为单位 */
    uint32_t kbm_length = kernel_free_pages / 8;

    /* User BitMap 的长度 */
    uint32_t ubm_length = user_free_pages / 8;

    /* Kernel Pool start,内核内存池的起始地址 */
    uint32_t kp_start = used_mem;

    /* User Pool start,用户内存池的起始地址 */
    uint32_t up_start = kp_start + kernel_free_pages * PG_SIZE;

    kernel_pool.phy_addr_start = kp_start;
    user_pool.phy_addr_start = up_start;

    kernel_pool.pool_size = kernel_free_pages * PG_SIZE;
    user_pool.pool_size = user_free_pages * PG_SIZE;

    kernel_pool.pool_bitmap.btmp_bytes_len = kbm_length;
    user_pool.pool_bitmap.btmp_bytes_len = ubm_length;


    /* 内核使用的最高地址是0xc009f000,0xc009e000 是内核主线程的pcb,0xc0009a000是位图基地址 */
    kernel_pool.pool_bitmap.bits = (void *)MEM_BITMAP_BASE;

    /* 用户内存池的位图紧跟在内核内存池位图之后 */
    user_pool.pool_bitmap.bits = (void *)(MEM_BITMAP_BASE + kbm_length);

    /**************************** 输出内存池信息 ****************************** */
    put_str("  kernel_pool_bitmap_start:");
    put_int((int)kernel_pool.pool_bitmap.bits);
    put_str("  kernel_pool_phy_addr_start:");
    put_int(kernel_pool.phy_addr_start);
    put_str("\n");
    
    put_str("user_pool_bitmap_start:");
    put_int((int)user_pool.pool_bitmap.bits);
    put_str(" user_pool_phy_addr_start:");
    put_int(user_pool.phy_addr_start);
    put_str("\n");

    /* 将位图置0 */
    bitmap_init(&kernel_pool.pool_bitmap);
    bitmap_init(&user_pool.pool_bitmap);

    /* 初始化内核虚拟地址的位图，按实际物理内存大小生成数组 */
    /* 用于维护内核堆的虚拟地址，所以要和内核内存池大小一致 */
    kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_length;

    /* 位图的数组指向一块未使用的内存，目前定位在内核内存池和用户内存池之外 */
    kernel_vaddr.vaddr_bitmap.bits = (void *)(MEM_BITMAP_BASE + kbm_length + ubm_length);

    kernel_vaddr.vaddr_start = K_HEAP_START;

    bitmap_init(&kernel_vaddr.vaddr_bitmap);

    put_str("  mem_pool_init done \n");
}





/* 内存管理部分初始化入口 */
void mem_init()
{
    put_str("mem_init start\n");
    uint32_t mem_bytes_total = (*(uint32_t *)(0xb00));
    mem_pool_init(mem_bytes_total);
    put_str("mem_init done\n");
}


