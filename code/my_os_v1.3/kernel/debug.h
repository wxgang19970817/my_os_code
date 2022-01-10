/*************************************************************************
	> File Name: debug.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月06日 星期一 19时33分54秒
 ************************************************************************/

#ifndef __KERNEL_DEBUG_H
#define __KERNEL_DEBUG_H

void panic_spin(char *filename,int line,const char* func,const char* condition);

/******************* VA_ARGS **************************
 * __VA_ARGS__ 预处理器所支持的专用标示符,代表所有与省略号相对应的参数,至少一个但可以为空
 * "..." 参数个数可变的宏　*/
#define PANIC(...) panic_spin(__FILE__,__LINE__,__func__,__VA_ARGS__)


#ifdef NDEBUG
    #define ASSERT(CONDITION) ((void)0)
#else
    #define ASSERT(CONDITION) \
    if(CONDITION){}else{      \
        /* 符号#让编译器将宏的参数转化为字符串字面量,如 var = 0 变为　"var = 0" */\
     /* 传给panic_spin函数的第4个参数__VA_ARGS,实际类型为字符串指针 */\
        PANIC(#CONDITION); \
    }


#endif /* NDEBUG */    
#endif /* __KERNEL_DEBUG_H */
