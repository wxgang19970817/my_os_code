/*************************************************************************
	> File Name: memory.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月07日 星期二 15时54分44秒
 ************************************************************************/

#include "memory.h"
#include "../lib/stdint.h"
#include "../lib/kernel/print.h"
#include "../lib/kernel/bitmap.h"
#include "global.h"
#include "debug.h"
#include "string.h"

#define PG_SIZE 4096




#define PDE_IDEX(addr) ((addr & 0xffc00000) >> 22)  /* 用于返回虚拟地址的高10位 */
#define PTE_IDEX(addr) ((addr & 0x003ff000) >> 12)  /* 用于返回虚拟地址的中10位 */


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


/* 在pf表示的虚拟内存池中申请pg_cnt个虚拟页,成功则返回虚拟页的起始地址，失败则返回NULL */
static void* vaddr_get(enum pool_flags pf,uint32_t pg_cnt)
{
    /* vaddr_start 存储分配的起始虚拟地址　bit_idx_start 存储位图扫描函数的返回值 */
    int vaddr_start = 0,bit_idx_start = -1;
    uint32_t cnt = 0;
    if(pf == PF_KERNEL)
    {
        /* 如果成功扫描到就会返回其起始地址对应下标 */
        bit_idx_start = bitmap_scan(&kernel_pool.pool_bitmap,pg_cnt);
    
        if(bit_idx_start == -1)
        {
            return NULL;
        }
        while(cnt < pg_cnt)
        {
            bitmap_set(&kernel_pool.pool_bitmap,bit_idx_start + cnt++, 1);
        }

        /* 起始虚拟地址等于虚拟内存池的起始地址　+ 起始位索引*页的大小 */
        vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start*PG_SIZE;
    }
    else
    {
        /* 用户内存池，等将来实现用户进程再补充 */
    }
    
    return (void*)vaddr_start;
}

/* 得到虚拟地址vaddr对应的pte指针(即虚拟地址) */
uint32_t* pte_ptr(uint32_t vaddr)
{
    /* 现在输入一个虚拟地址，我们要返回这个地址所在页表项的地址，就需要构造一个新的地址
     * 让处理器按照这个新地址寻址，寻到的物理地址就是这个页表项所在的物理地址*/
    /* 第1023个页表项指向的是页目录表自身 1023=0x3ff 将其移到高10位 变成0xffc00000 */
    /* 用此10位当虚拟地址的高位，可以让处理器取出最后一个页表，也即页目录表 */
    /* 中间10位本来是处理器来寻找页表项的，但是此时我们想要让处理器在页目录表里寻找 */
    /* 将虚拟地址的高10位当成新地址的中间10位，处理器就会在页目录表中找到页目录项 */
    /* 找到页表之后，利用偏移就可以找到页表项的物理地址，用输入地址的中10位（其实就是
     * 页表项的索引）* 4 当做新地址的低12位，因为处理器会直接将低12位当偏移处理 */
    uint32_t* pte = (uint32_t *)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDEX(vaddr) * 4);

    return pte;
}


/* 得到虚拟地址vaddr对应的pde指针(即虚拟地址) */
uint32_t* pde_ptr(uint32_t vaddr)
{
    uint32_t* pde = (uint32_t*)(0xfffff000 + PDE_IDEX(vaddr) * 4);

    return pde;
}


/* 在m_pool指向的物理内存池中分配1个物理页，成功则返回页框的物理地址，失败则返回NULL */
static void* palloc(struct pool* m_pool)
{
    /* 扫描或设置位图要保证原子操作 */
    int bit_idx = bitmap_scan(&m_pool->pool_bitmap, 1);
    if(bit_idx == -1)
    {
        return NULL;
    }
    bitmap_set(&m_pool->pool_bitmap,bit_idx,1);
    uint32_t page_phyaddr = ((bit_idx * PG_SIZE) + m_pool->phy_addr_start);
    return (void*)page_phyaddr;
}


/* 页表中添加虚拟地址_vaddr与物理地址_page_phyaddr的映射 */
/* 本质上就是在页表中添加此虚拟地址对应的页表项pte,并将该地址所属物理页的物理地址写入到这个页表项中 */
static void page_table_add(void* _vaddr,void* _page_phyaddr)
{
    uint32_t vaddr = (uint32_t)_vaddr,page_phyaddr = (uint32_t) _page_phyaddr;
    uint32_t * pde = pde_ptr(vaddr);
    uint32_t * pte = pte_ptr(vaddr);


    /* 先在页目录内判断目录项的p位(32位的第0位)，若为1，则表示该表已存在 */
    if(*pde & 0x00000001)
    {
        ASSERT(!(*pte & 0x00000001));

        /* 一般来说，只要是需要我们创建页表，pte就应该不会存在，多判断一下 */
        if(!(*pte & 0x00000001))
        {
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);  /* 高20位的地址和低三位的属性－用户级、可写、存在 */
        }
        else
        {
            /* 应该不会执行到这儿，因为前面的ASSERT会先执行 */
            PANIC("pte repeat");
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);  /* 高20位的地址和低三位的属性－用户级、可写、存在 */
        }
    }
    else /* 表示页目录项即页表不存在，所以要创建页表再创建页表项 */
    {   
        /* 页表用到页框一律从内核空间分配 */
        uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);

        *pde = (pde_phyaddr | PG_US_S | PG_RW_W | PG_P_1);

        /* 将用到的页表清0,要清零需要的是该页表的虚拟地址，目前只有物理地址 */
        /* pte是指向vaddr对应页表项物理地址的虚拟地址，也就是说这个pte的低12位是用来在页表中检索该页表项的，这个页表正好就是我们要清零的页表 */
        memset((int)pte & 0xfffff000, 0,PG_SIZE);

        ASSERT(!(*pte & 0x00000001));

        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}

/* 分配pg_cnt个页空间，成功则返回起始虚拟地址，失败时返回NULL */
void* malloc_page(enum pool_flags pf,uint32_t pg_cnt)
{
    ASSERT(pg_cnt > 0 && pg_cnt < 3840);

    /************************ malloc_page 原理　*******************************
     1 通过 vaddr_get 在虚拟内存池中申请虚拟地址
     2 通过　palloc 在物理内存中申请物理页
     3 通过　page_table_add 将以上得到的虚拟地址和物理地址在页表中完成映射
     **************************************************************************/
    void* vaddr_start = vaddr_get(pf,pg_cnt);
    if(vaddr_start == NULL)
    {
        return NULL;
    }

    uint32_t vaddr = (uint32_t)vaddr_start,cnt = pg_cnt;

    struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;

    /* 因为虚拟地址是连续的，但物理地址可以是不连续的，所以逐个做映射 */
    while(cnt-- > 0)
    {
        /* 每次只申请一个物理页 */
        void* page_phyaddr = palloc(mem_pool);
        if(page_phyaddr == NULL)
        {
            /* 失败时要将曾经已申请的虚拟地址和物理页全部回滚,内存回收的时候补充代码 */

            return NULL;
        }

        /* 将在虚拟内存池中申请的虚拟地址和用户或内核内存池中申请的物理页　在页表中建立映射关系 */
        page_table_add((void*)vaddr,page_phyaddr);

        /* 在虚拟内存池中申请的虚拟地址是连续的 */
        vaddr += PG_SIZE;
    }

    /* 返回申请到的虚拟地址的起始 */
    return vaddr_start;
}


/* 从内核物理内存池中申请pg_cut页内存，成功则返回其虚拟地址，失败则返回NULL */
void* get_kernel_pages(uint32_t pg_cnt)
{
    void* vaddr = malloc_page(PF_KERNEL,pg_cnt);
    if(vaddr != NULL)
    {
        /* 如果分配的页框不为空，将页框清0后返回 */
        memset(vaddr,0,pg_cnt * PG_SIZE);
    }

    return vaddr;
}

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
   // put_str("mem_init start\n");
    uint32_t mem_bytes_total = (*(uint32_t *)(0xb00));
    mem_pool_init(mem_bytes_total);
   // put_str("mem_init done\n");
}


