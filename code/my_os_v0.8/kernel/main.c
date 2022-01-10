#include "print.h"
#include "init.h"

void main(void)
{
    put_str("I am kernel\n");
    init_all();
    /* 为了演示中断处理，这里临时开中断 */
    asm volatile("sti"); 
    while(1);
}
