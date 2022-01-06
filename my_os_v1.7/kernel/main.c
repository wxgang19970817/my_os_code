
#include "../lib/kernel/print.h"
#include "init.h"
#include "interrupt.h"
#include "../thread/thread.h"
#include "../device/console.h"
#include "../userprog/process.h"
#include "../userprog/syscall-init.h"
#include "syscall.h"
#include "stdio.h"
#include "memory.h"
#include "dir.h"
#include "fs.h"
#include "assert.h"
#include "shell.h"


void init(void);


int main(void)
{
    put_str("I am kernel\n");
    init_all();
    
    cls_screen();
    console_put_str("[rabbit@localhost /]$");
    while(1);
    return 0;
}



/* init进程 */
void init(void)
{
    uint32_t ret_pid = fork();
    if(ret_pid)
    {
        while(1);
    }
    else
    {
    	my_shell();
    }
    panic("init: should not be here");
}