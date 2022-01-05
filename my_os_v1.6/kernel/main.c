
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

void k_thread_a(void*);
void k_thread_b(void*);
void u_prog_a(void);
void u_prog_b(void);


int main(void)
{
    put_str("I am kernel\n");
    init_all();
 

    printf("try to delete nonempty directory /dir1/subdir1\n");
    struct dir* dir = sys_opendir("/dir1/");
    char* type = NULL;
    struct dir_entry* dir_e = NULL;
    while((dir_e = sys_readdir(dir))) { 
    if (dir_e->f_type == FT_REGULAR) {
	    type = "regular";
      } else {
	 type = "directory";
      }
      printf("      %s   %s\n", type, dir_e->filename);
   }
   printf("try to delete nonempty directory /dir1/subdir1\n");
   if (sys_rmdir("/dir1/subdir1") == -1) 
   {
      printf("sys_rmdir: /dir1/subdir1 delete fail!\n");
   }

   printf("try to delete /dir1/subdir1/file2\n");
   if (sys_rmdir("/dir1/subdir1/file2") == -1) 
   {
      printf("sys_rmdir: /dir1/subdir1/file2 delete fail!\n");
   } 

   if (sys_unlink("/dir1/subdir1/file2") == 0 ) 
   {
      printf("sys_unlink: /dir1/subdir1/file2 delete done\n");
   }
   
   printf("try to delete directory /dir1/subdir1 again\n");
   if (sys_rmdir("/dir1/subdir1") == 0) 
   {
      printf("/dir1/subdir1 delete done!\n");
   }

   printf("/dir1 content after delete /dir1/subdir1:\n");
   sys_rewinddir(dir);
   while((dir_e = sys_readdir(dir))) { 
      if (dir_e->f_type == FT_REGULAR) {
	 type = "regular";
      } else {
	 type = "directory";
      }
      printf("      %s   %s\n", type, dir_e->filename);
   }
    while(1);
    return 0;
}

void k_thread_a(void* arg)
{
    void* addr1 = sys_malloc(256);
    void* addr2 = sys_malloc(255);
    void* addr3 = sys_malloc(254);

    console_put_str(" thread_a malloc addr:0x");
    console_put_int((int)addr1);
    console_put_char(',');
    console_put_int((int)addr2);
    console_put_char(',');
    console_put_int((int)addr3);
    console_put_char('\n');

    int cpu_delay = 100000;
    while(cpu_delay-- > 0);
    
     
    sys_free(addr2);
    sys_free(addr3);
    sys_free(addr1);

    while (1);
}

void k_thread_b(void* arg)
{ 
    void* addr1 = sys_malloc(256);
    void* addr2 = sys_malloc(255);
    void* addr3 = sys_malloc(254);

    console_put_str(" thread_a malloc addr:0x");
    console_put_int((int)addr1);
    console_put_char(',');
    console_put_int((int)addr2);
    console_put_char(',');
    console_put_int((int)addr3);
    console_put_char('\n');

    int cpu_delay = 100000;
    while(cpu_delay-- > 0);
    
     
    sys_free(addr2);
    sys_free(addr3);
    sys_free(addr1);

    while (1);
}


/* 测试用户进程 */
void u_prog_a(void)
{
    void* addr1 = malloc(256);
    void* addr2 = malloc(255);
    void* addr3 = malloc(254);
    printf(" prog_a malloc addr:0x%x,0x%x,0x%x\n",(int)addr1,(int)addr2,(int)addr3);

    int cpu_delay = 100000;
    while(cpu_delay-- > 0);
    free(addr1);
    free(addr2);
    free(addr3);
    while(1);
}

void u_prog_b(void)
{
    void* addr1 = malloc(256);
    void* addr2 = malloc(255);
    void* addr3 = malloc(254);
    printf(" prog_a malloc addr:0x%x,0x%x,0x%x\n",(int)addr1,(int)addr2,(int)addr3);

    int cpu_delay = 100000;
    while(cpu_delay-- > 0);
    free(addr1);
    free(addr2);
    free(addr3);
    while(1);
}