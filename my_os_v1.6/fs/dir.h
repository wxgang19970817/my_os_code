/*************************************************************************
	> File Name: dir.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月27日 星期一 20时44分23秒
 ************************************************************************/
#ifndef __FS_DIR_H
#define	__FS_DIR_H

#include	"stdint.h"
#include	"inode.h"
#include	"ide.h"
#include	"global.h"

#define MAX_FILE_NAME_LEN	16		/* 最大文件名长度 */

/* 目录结构，不在磁盘上存在，只用于与目录相关的操作，在内存中创建的，用完就释放了，不会回写到磁盘 */
struct	dir
{
	struct	inode*	inode;
	uint32_t	dir_pos;			/* 记录在目录内的偏移 */
	uint8_t		dir_buf[512];		/* 目录的数据缓存 */
};

/* 目录项结构 */
struct	dir_entry
{
	char filename[MAX_FILE_NAME_LEN];			/* 普通文件或目录名称 */
	uint32_t	i_no;							/* 普通文件或目录对应的inode编号 */
	enum file_types	f_type;				/* 文件类型 */
};

#endif 

