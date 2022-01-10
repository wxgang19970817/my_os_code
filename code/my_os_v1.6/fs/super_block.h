/*************************************************************************
	> File Name: super_block.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月27日 星期一 19时55分50秒
 ************************************************************************/

#ifndef __FS_SUPER_BLOCK_H
#define	__FS_SUPER_BLOCK_H

#include "stdint.h"


/* 超级块,为了方便这里的数据块大小和扇区大小一致 */
struct super_block
{
	uint32_t	magic;							/* 操作系统支持多种文件系统，通过此标志来识别文件系统类型 */
	uint32_t	sec_cnt;	 		 		   /* 本分区总共的扇区数，一个分区一个超级块 */
	uint32_t	inode_cnt;			  		/* 本分区中inode数量 */
	uint32_t	part_lba_base;			/* 本分区的起始lba地址 */

	uint32_t	block_bitmap_lba;	/* 块位图本身起始扇区地址 */
	uint32_t	block_bitmap_sects;		/* 扇区位图本身占用的扇区数量是取决于分区的容量大小 */

	uint32_t	inode_bitmap_lba;		/* i结点位图起始扇区lba地址 */
	uint32_t	inode_bitmap_sects;		/* i结点位图占用的扇区数量 */

	uint32_t	inode_table_lba;		/* i结点表起始扇区lba地址 */
	uint32_t 	inode_table_sects;		/* i结点表占用的扇区数量 */

	uint32_t	data_start_lba;					/* 数据区开始的第一个扇区号 */
	uint32_t	root_inode_no;				/* 根目录所在的i结点号 */
	uint32_t	dir_entry_size;					/* 目录项大小 */

	uint8_t		pad[460];							/* 为了凑够一个扇区的大小 */
}__attribute__((packed));


#endif


