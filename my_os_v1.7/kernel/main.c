
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
    
    /******** 写入应用程序 ***********/
    uint32_t file_size = 5343;
    uint32_t sec_cnt = DIV_ROUND_UP(file_size,512);
    struct disk* sda = &channels[0].devices[0];
    void* prog_buf = sys_malloc(file_size);
    ide_read(sda,300,prog_buf,sec_cnt);
    int32_t fd = sys_open("/prog_pipe",O_CREAT | O_RDWR);
    if(fd != 1)
    {
        if(sys_write(fd,prog_buf,file_size) == -1)
        {
            printk("file write error! \n");
            while(1);
        }
    }

    cls_screen();
    console_put_str("[rabbit@localhost /]$");
    thread_exit(running_thread(),true);
    while(1);
    return 0;
}



/* init进程 */
void init(void)
{
    uint32_t ret_pid = fork();
    if(ret_pid)
    {
        /* 父进程 */
        int status;
        int child_pid;
        /* init在此处不停地回收僵尸进程 */
        while(1)
        {
            child_pid = wait(&status);
            printf("i`m init,My pid is 1,i recieve a child");
        }
    }
    else
    {       /* 子进程 */
    	my_shell();
    }
    panic("init: should not be here");
}