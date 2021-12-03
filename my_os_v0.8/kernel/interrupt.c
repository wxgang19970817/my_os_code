/*************************************************************************
	> File Name: interrupt.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月01日 星期三 19时09分22秒
 ************************************************************************/

#include "interrupt.h"
#include "global.h"
#include "io.h"
#include "print.h"


/* 可支持的中断个数,共33个 */
#define IDT_DESC_CNT 0x21           

#define PIC_M_CTRL 0x20         /* 主片控制端口 */
#define PIC_M_DATA 0x21         /* 主片数据端口 */
#define PIC_S_CTRL 0xa0         /* 从片控制端口 */
#define PIC_S_DATA 0xa1         /* 从片数据端口 */


/* 中断门描述符结构体 */
struct gate_desc{
    uint16_t func_offset_low_word;
    uint16_t selector;

    /* 双字计数段，是门描述符中的第4字节 */
    uint8_t  dcount;                
    uint8_t  attribute;
    uint16_t func_offset_high_word;
};




/* 静态函数声明，非必须 */
static void make_idt_desc(struct gate_desc *p_gdesc,uint8_t attr,intr_handler function);

/* IDT中断描述符表，本质上就是个中断门描述符数组 */
static struct gate_desc idt[IDT_DESC_CNT];

/* 声明引用定义在kernel.S中的中断处理函数入口数组 */
extern intr_handler intr_entry_table[IDT_DESC_CNT];





/* 初始化可编程中断控制器8259A */
static void pic_init(void)
{
    /* 初始化主片 */
       /* ICW1:边沿触发，级联8259,需要ICW4 */
    outb(PIC_M_CTRL,0x11);
    outb (PIC_M_DATA,0x20);   /* ICW2:起始向量号为0x20,也就是IR[0-7]为0x20~0x27 */

    outb(PIC_M_DATA,0x04);   /* ICW3:IR2接从片 */
    outb(PIC_M_DATA,0x01);   /* ICW4:8086模式,正常EOI */

    /* 初始化从片 */
    outb(PIC_S_CTRL,0x11);   /* ICW1:边沿触发，级联8259,需要ICW4 */
    outb(PIC_S_DATA,0x28);   /* ICW2:起始向量号为0x28,也就是IR[8-15]为0x28~0x2F */

    outb(PIC_S_DATA,0x02);   /* ICW3:设置从片连接到主片的IR2引脚 */
    outb(PIC_S_DATA,0x01);   /* ICW4:8086模式,正常EOI */

    /* 打开主片上IR0,也就是目前只接受时钟产生的中断 */
    outb(PIC_M_DATA,0xfe);
    outb(PIC_S_DATA,0xff);

    put_str("pic_init done\n");
}




/* 创建中断门描述符 */
static void make_idt_desc(struct gate_desc *p_gdesc,uint8_t attr,intr_handler function)
{
    p_gdesc->func_offset_low_word  = (uint32_t)function & 0x0000FFFF;
    p_gdesc->selector              = SELECTOR_K_CODE;
    p_gdesc->dcount                = 0;
    p_gdesc->attribute             = attr;
    p_gdesc->func_offset_high_word = ((uint32_t)function & 0xFFFF0000) >> 16;
}

/* 初始化中断描述符表 */
static void idt_desc_init(void)
{
    int i;
    for(i=0;i<IDT_DESC_CNT;i++)
    {
        make_idt_desc(&idt[i],IDT_DESC_ATTR_DPL0,intr_entry_table[i]);
    }

    put_str("idt_desc_init done\n");
}

/* 完成有关中断的所有初始化工作 */
void idt_init()
{
    put_str("idt_init start\n");

    /* 构造中断描述符表 */
    idt_desc_init();               

    /* 初始化8259A */
    pic_init();
    /* 加载idt 32位基地址　+ 16位段界限*/
    uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt << 16));
    asm volatile("lidt %0"::"m"(idt_operand));
    put_str("idt_init done\n");
}
