/*************************************************************************
	> File Name: syscall-init.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月21日 星期二 20时08分15秒
 ************************************************************************/

#include "syscall-init.h"
#include "../lib/user/syscall.h"
#include "stdint.h"
#include "print.h"
#include "thread.h"
#include "thread.h"
#include "console.h"
#include "string.h"


#define syscall_nr		32
typedef void* syscall;
syscall  syscall_table[syscall_nr];

/* 返回当前任务的pid */
uint32_t sys_getpid(void)
{
	return running_thread()->pid;
}

/* 初始化系统调用 */
void syscall_init(void)
{
	put_str("syscall_init start\n");
	syscall_table[SYS_GETPID] = sys_getpid;
	put_str("syscall_init done\n");
}