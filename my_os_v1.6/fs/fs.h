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
#define		BLOCK_SIZE  SECTOR_SIZE		/* 块字节大小 */

#define MAX_PATH_LEN  512			/* 路径最大长度 */


/* 文件类型 */
enum	file_types
{
	FT_UNKNOWN,			/* 不支持的文件类型 */
	FT_REGULAR,			/* 普通文件 */
	FT_DIRECTORY	  /* 目录 */
};


/* 打开文件的选项 */
enum oflags
{
	O_RDONLY,			/* 000b 只读 */
	O_WRONLY,			/* 001b 只写 */
	O_RDWR,				  /* 010b  读写 */
	O_CREAT = 4			/* 100b  创建 */
};

/* 用来记录查找文件过程中已找到的上级路径，也就是查找文件过程中"走过的地方" */
struct path_search_record
{
	char searched_path[MAX_PATH_LEN];		/* 查找过程中的父路径 */
	struct dir* parent_dir;					/* 文件或目录所在的直接父目录 */
	enum file_types file_type;			/* 找到的是普通文件还是目录，找不到将为未知类型(FT_UNKNOWN) */
};

extern struct partition* cur_part;
void filesys_init(void);
int32_t path_depth_cnt(char* pathname);
int32_t sys_open(const char* pathname, uint8_t flags);
int32_t sys_close(int32_t fd);
int32_t sys_write(int32_t fd, const void* buf, uint32_t count);
#endif 
