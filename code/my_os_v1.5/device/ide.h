/*************************************************************************
	> File Name: ide.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月25日 星期六 14时49分27秒
 ************************************************************************/
#ifndef __DEVICE_IDE_H
#define __DEVICE_IDE_H

#include "stdint.h"
#include "sync.h"
#include "list.h"
#include "bitmap.h"

/* 分区表结构 */
struct partition
{
	uint32_t					start_lba;				/* 分区的起始扇区 */
	uint32_t					sec_cnt;				/* 分区的容量扇区数 */
	struct disk* 	  		  my_disk;			    /* 分区所属的硬盘 */
	struct list_elem 	  part_tag;			    /* 本分区将来会汇总到队列中，用此标记 */
	char name[8];						  				  /* 分区名称 */
	struct	super_block* sb;				 /* 本分区超级块 */
	struct	bitmap			block_bitmap;/* 块位图,管理文件系统读取磁盘的多个扇区组成的块 */
	struct	bitmap			inode_bitmap;/* i结点位图 */
	struct	list				   open_inodes;	 /* 本分区打开的i结点队列 */
};

/* 硬件的结构 */
struct disk
{
	char 					name[8];				/* 本硬盘的名称 */
	struct	ide_channel* my_channel;		/* 此硬盘归属于哪个ide通道 */
	uint8_t				  dev_no;								/* 本硬盘是主0,还是从1 */
	struct	partition	prim_parts[4];			   /* 主分区顶多有4个 */
	struct	partition  	logic_parts[8];				/* 逻辑分区数量无限，这里设置上限为8个 */			   			
};

/* ata 通道结构 */
struct ide_channel
{
	char				name[8];						/* 本 ata 通道名称 */
	uint16_t		port_base;					  /* 本通道的起始端口号 */
	uint8_t			 irq_no;							/* 本通道所用的中断号 */
	struct	lock  lock;									/* 通道锁 */
	bool	expecting_intr;						  /* 表示等待硬盘的中断 */
	struct	semaphore	disk_done;	  /* 驱动程序向硬盘发送命令后，在等待硬盘工作期间，用于阻塞、唤醒驱动程序 */
	struct	disk		devices[2];				 /* 一个通道上连接两个硬盘，一主一从 */
};

void intr_hd_handler(uint8_t irq_no);

void ide_init(void);
extern uint8_t channel_cnt;
extern struct ide_channel channels[];
extern struct list partition_list;
void ide_read(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);
void ide_write(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);

#endif
