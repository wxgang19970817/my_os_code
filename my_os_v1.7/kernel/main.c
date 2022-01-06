
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

void init(void);


int main(void)
{
    put_str("I am kernel\n");
    init_all();
 
   /**** 测试代码*********/
   /**********************/
    while(1);
    return 0;
}



/* init进程 */
void init(void)
{
    uint32_t ret_pid = fork();
    if(ret_pid)
    {
        printf("i am father,my pid is %d,child pid is %d\n",getpid(),ret_pid);
    }
    else
    {
        printf("i am child,my pid is %d,ret pid is %d\n",getpid(),ret_pid);
    }
    while(1);
}