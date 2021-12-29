/*************************************************************************
	> File Name: fs.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月27日 星期一 21时01分11秒
 ************************************************************************/
#ifndef		__FS_FS_H
#define	__FS_FS_H
#include "stdint.h"
#include "ide.h"

#define		MAX_FILES_PER_PART	4096		/* 每个分区所支持最大创建的文件数 */
#define 	BITS_PER_SECTOR	4096			/* 每扇区的位数 */
#define		SECTOR_SIZE		512					/* 扇区字节大小 */
#define		BLOCK_SIZE SECTOR_SIZE		/* 块字节大小 */


/* 文件类型 */
enum	file_types
{
	FT_UNKNOWN,			/* 不支持的文件类型 */
	FT_REGULAR,			/* 普通文件 */
	FT_DIRECTORY	  /* 目录 */
};

void filesys_init(void);

#endif 
