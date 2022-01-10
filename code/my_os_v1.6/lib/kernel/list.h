/*************************************************************************
	> File Name: list.h
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月10日 星期五 17时39分42秒
 ************************************************************************/

#ifndef __LIB_KERNEL_LIST_H
#define __LIB_KERNEL_LIST_H


#include "../../kernel/global.h"

#define offset(struct_type,member) (int)(&((struct_type*)0)->member)

/* 作用是将指针elem_ptr转换成struct_type类型的指针，
 * 原理是用elem_ptr的地址减去elem_ptr在结构体struct_type中的偏移量得到地址差再转换成struct_type*/
#define elem2entry(struct_type,struct_member_name,elem_ptr) \
    (struct_type *)((int)elem_ptr - offset(struct_type,struct_member_name))

/*************** 定义链表结点成员结构 ****************
 *链表的功能:链，存储数据，这里结点中不需要数据成员，只要求前驱和后继结点指针*/
struct list_elem
{
    struct list_elem* prev;     /* 前驱结点 */
    struct list_elem* next;     /* 后继结点 */
};

/* 链表结构，用来实现队列 */
struct list
{
    /* head是队首，是固定不变的头入口，不是第一个元素，第1个元素为head.next */
    struct list_elem head;
    /* tail是队尾，同样是固定不变的尾入口 */
    struct list_elem tail;
};

/* 自定义函数类型function,用在list_travelsal中做回调函数 */
typedef bool (function) (struct list_elem*,int arg);


void list_init(struct list* list);
void list_insert_before(struct list_elem* before,struct list_elem* elem);
void list_push(struct list* plist,struct list_elem* elem);
void list_iterate(struct list* plist);
void list_append(struct list* plist,struct list_elem* elem);
void list_remove(struct list_elem* pelem);
struct list_elem* list_pop(struct list* plist);
bool elem_find(struct list* plist,struct list_elem* obj_elem);
bool list_empty(struct list* plist);
struct list_elem* list_traversal(struct list* plist,function func,int arg);
uint32_t list_len(struct list* plist);

#endif
