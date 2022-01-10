/*************************************************************************
	> File Name: console.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月15日 星期三 11时02分38秒
 ************************************************************************/

#include "console.h"
#include "../lib/kernel/print.h"
#include "../lib/stdint.h"
#include "../thread/sync.h"
#include "../thread/thread.h"

/* 所有的终端操作都是围绕这个锁展开的，必须是全局唯一的，因此是静态static */
static struct lock console_lock;            /* 控制台锁 */


/* 初始化终端 */
void console_init()
{
    lock_init(&console_lock);
}

/* 获取终端 */
void console_acquire()
{
    lock_acquire(&console_lock);
}


/* 释放终端 */
void console_release()
{
    lock_release(&console_lock);
}

/* 终端中输出字符串 */
void console_put_str(char* str)
{
    console_acquire();
    put_str(str);
    console_release();
}

/* 终端中输出字符 */
void console_put_char(uint8_t char_asci)
{
    console_acquire();
    put_char(char_asci);
    console_release();
}

/* 终端中输出十六进制整数 */
void console_put_int(uint32_t num)
{
    console_acquire();
    put_int(num);
    console_release();
}
