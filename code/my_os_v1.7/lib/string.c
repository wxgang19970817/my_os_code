/*************************************************************************
	> File Name: string.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月07日 星期二 09时17分15秒
 ************************************************************************/

#include "string.h"
#include "assert.h"

/* 将 dst_ 起始的 size 个字节置为 value  */
void memset(void* dst_,uint8_t value,uint32_t size)
{
    assert(dst_ != NULL);
    uint8_t* dst = (uint8_t *)dst_;
    while(size-- > 0)
        *dst++ = value;
}


/* 将 src_ 起始的size个字节复制到dst_ */
void memcpy(void* dst_,const void* src_,uint32_t size)
{
    assert(dst_ != NULL && src_ != NULL);
    uint8_t* dst = dst_;
    const uint8_t* src = src_;
    while(size-- > 0)
        *dst++ = *src++;
}

/* 连续比较以地址 a_ 和地址 b_ 开头的size个字节，若相等返回0，若a_大于b_,返回+1,否则就返回-1 */
int memcmp(const void* a_,const void* b_,uint32_t size)
{
    const char* a = a_;
    const char* b = b_;

    assert(a != NULL || b != NULL);

    while(size-- > 0)
    {
        if(*a != *b ? 1 : -1)
        {
            return *a > *b ? 1 : -1;
        }
        a++;
        b++;
    }
    return 0;
}


/* 将字符串从 src_ 复制到　dst_ */
char* strcpy(char* dst_,const char* src_)
{
    assert(dst_ != NULL && src_ != NULL);
    char * r = dst_;                                 /* 用来返回目的字符串起始地址 */
    while((*dst_ ++ = *src_ ++));
    return r;
}


/* 返回字符串长度 */
uint32_t strlen(const char* str)
{
    assert(str != NULL);
    const char* p = str;
    while(*p++);                    /* 字符串结尾 */
    return(p - str - 1);
}


/* 比较两个字符串，若a_中的字符大于b_中字符返回1,相等时返回0,否则返回-1 */
int8_t strcmp(const char* a,const char* b)
{
    assert(a != NULL && b != NULL);
    while( *a != 0 && *a == *b )
    {
        a++;
        b++;
    }
    return *a < *b ? -1 : *a > *b ;
}

/*　从左到右查找字符串 str 中首次出现字符 ch 的地址 */
char* strchr(const char* str,const uint8_t ch)
{
    assert(str != NULL);
    while(*str != 0)
    {
        if(*str == ch)
        {
            return (char*)str;          /* 需要强制转换成和返回值类型一样，否则编译器会报const属性丢失 */
        }
        str++;
    }
    return NULL;
}


/* 从后往前查找字符串 str 中首次出现字符 ch 的地址 */
char* strrchr(const char* str,const uint8_t ch)
{
    assert(str != NULL);
    const char* last_char = NULL;

    /* 从头到尾遍历一次，若存在ch字符，last_char总是该字符最后一次出现在串中的地址(不是下标，是地址) */
    while(*str != 0)
    {
        if(*str == ch)
        {
            last_char = str;
        }
        str++;          /* 注意比较和前面的区别 */
    }
    return  (char*)last_char;
}


/* 将字符串src_ 拼接到dst_后，返回拼接的地址 */
char* strcat(char* dst_,const char* src_)
{
    assert(dst_ != NULL && src_ != NULL);
    char* str = dst_;
    while(*str++);
    --str;
    while((*str++ = *src_++));       /* 正好当*str被赋值为0后，表达式不成立 */
    return dst_;
}

/* 在字符串str中查找字符ch出现的次数 */
uint32_t strchrs(const char* str,uint8_t ch)
{
    assert(str != NULL);
    uint32_t ch_cnt = 0;
    const char* p = str;
    while(*p != 0)
    {
        if(*p == ch)
        {
            ch_cnt++;
        }
        p++;
    }
    return ch_cnt;
}
