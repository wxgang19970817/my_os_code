/*************************************************************************
	> File Name: tss.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月17日 星期五 20时34分48秒
 ************************************************************************/
#ifndef __USERPROG_TSS_H
#define __USERPROG_TSS_H

#include "../thread/thread.h"

void updata_tss_esp(struct task_struct* pthread);
void tss_init(void);



#endif
