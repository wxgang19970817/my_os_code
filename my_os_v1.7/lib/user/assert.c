/*************************************************************************
	> File Name: assert.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2022年01月06日 星期四 17时21分16秒
 ************************************************************************/

#include "assert.h"
#include "stdio.h"

void user_spin(char* filename,int line,const char* func,const char* condition)
{
	printf("\n\n\n\nfilename %s \nline %d\nfunction %s\ncondition %s\n",filename,line,func,condition);
	while(1);
}