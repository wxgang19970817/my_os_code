/*************************************************************************
	> File Name: interrupt.c
	> Author: wxgang
	> Mail: wxgang19970817@163.com 
	> Created Time: 2021年12月01日 星期三 19时09分22秒
 ************************************************************************/

#include "interrupt.h"
#include "global.h"
#include "../lib/kernel/io.h"
#include "../lib/kernel/print.h"
#include "../lib/stdint.h"


/* 可支持的中断个数 */
#define IDT_DESC_CNT 0x30           




#define PIC_M_CTRL 0x20         /* 主片控制端口 */
#define PIC_M_DATA 0x21         /* 主片数据端口 */
#define PIC_S_CTRL 0xa0         /* 从片控制端口 */
#define PIC_S_DATA 0xa1         /* 从片数据端口 */


/* eflags 寄存器　中断标志位 IF 第9位 */
#define EFLAGS_IF 0x00000200

#define GET_EFLAGS(EFLAG_VAR) asm volatile("pushfl;popl %0":"=g"(EFLAG_VAR))


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

/* 用于保存异常的名字 */
char* intr_name[IDT_DESC_CNT];

/* 定义中断处理程序数组　在kernel.S中定义的interXXentry只是中断处理程序的入口，最终调用的是idt_table中的处理程序 */
intr_handler idt_table[IDT_DESC_CNT];

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
   // outb(PIC_M_DATA,0xfe);
   // outb(PIC_S_DATA,0xff);

    /* 打开键盘中断和时钟中断，其他全部关闭 */
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

/* 通用的中断处理函数，一般用在异常出现时的处理 */
static void general_intr_hander(uint8_t vec_nr)
{
    if(vec_nr == 0x27 || vec_nr == 0x2f)
    {
        /* IRQ7(并口1)和IRQ15(保留)会产生伪中断，无法通过IMR寄存器屏蔽 */
        return;
    }
    
    /* 将光标置为0，从屏幕左上角清出一片打印异常信息的区域，方便阅读 */
    set_cursor(0);
    int cursor_pos = 0;
    while(cursor_pos < 320)
    {
        put_char(' ');
        cursor_pos++;
    }

    set_cursor(0);          /* 重置光标为屏幕左上角 */
    put_str("!!!!! exception_message begin !!!!!!!\n");
    set_cursor(88);         /* 从第2行第8个字符开始打印 */
    put_str(intr_name[vec_nr]);
    if(vec_nr == 14)
    {
        /* 若为pagefault,将缺失的地址打印出来并悬停 */
        int page_fault_vaddr = 0;

        asm("movl %%cr2,%0":"=r"(page_fault_vaddr));    /* cr2存放造成page_fault_vaddr的地址 */
        put_str("\npage fault addr is");put_int(page_fault_vaddr);
    }
        put_str("\n!!!!!!  excetion message end  !!!!!!!");

        /* 处理器进入中断后会自动把标志寄存器eflags中的IF位置0 */
        /* 能进入中断处理程序就表示已经处在关中断情况下 */
        /* 不会出现调度进程的情况。故下面的死循环不会再被中断 */
        while(1);      

}




/* 完成一般的中断处理函数注册及异常名称注册 */
static void exception_init(void)
{
    int i;
    for(i=0;i<IDT_DESC_CNT;i++)
    {
        /* idt_table 数组中的函数是在进入中断后根据中断向量号调用的，在kernel.S中的call[idt_table + %1*4] */
        /* 先将general_intr_hander函数指向idt_table中的所有元素 */
        idt_table[i] = general_intr_hander;
        /* 先统一赋值为 unknown */
        intr_name[i] = "unknown";
    }

    /* 异常名字赋值 */
    intr_name[0] = "#DE Divid Error";
    intr_name[1] = "#DB Debug Exception";
    intr_name[2] = "NMI Interrupt";
    intr_name[3] = "#BP Breakpoint Exception";
    intr_name[4] = "#OF Overflow Exception";
    intr_name[5] = "#BR BOUND Range Exceeded Exception";
    intr_name[6] = "#UD Invalid Opcode Exception";
    intr_name[7] = "#NM Device Not Available Exception";
    intr_name[8] = "#DF Double Fault Exception";
    intr_name[9] = "Coprocessor Segment Overrun";
    intr_name[10] = "#TS Invalid TSS Exception";
    intr_name[11] = "#NP Segment Not Present";
    intr_name[12] = "#SS Stack Fault Exception";
    intr_name[13] = "#GP General Protection Exception";
    intr_name[14] = "#PF Page-Fault Exception";
    /* intr_name[15] Intel保留项　*/
    intr_name[16] = "#MF x87 FPU Floating-Point Error";
    intr_name[17] = "#AC Alignment Check Exception";
    intr_name[18] = "#MC Machine-Check Exception";
    intr_name[19] = "#XF SIMD Floating-Point Exception";
}






/* 完成有关中断的所有初始化工作 */
void idt_init()
{
    put_str("idt_init start\n");

    /* 构造中断描述符表 */
    idt_desc_init();               
    
    /* 异常名初始化并注册通常的中断处理函数 */
    exception_init();

    /* 初始化8259A */
    pic_init();
    /* 加载idt 32位基地址　+ 16位段界限*/
    uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt << 16));
    asm volatile("lidt %0"::"m"(idt_operand));
    put_str("idt_init done\n");
}


/* 获取当前中断状态 */
enum intr_status intr_get_status()
{
    uint32_t eflags=0;
    GET_EFLAGS(eflags);
    return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}


/* 开中断，并返回开中断前的状态 */
enum intr_status intr_enable()
{
    enum intr_status old_status;
    if(INTR_ON == intr_get_status())
    {
        old_status = INTR_ON;
        return old_status;
    }
    else
    {
        old_status = INTR_OFF;
        asm volatile ("sti");       /* 开中断　sti 将状态寄存器的 IF位置 */       
        return old_status;
    }
}


/* 关中断，并返回关中断之前的状态 */
enum intr_status intr_disable()
{
    enum intr_status old_status;
    if(INTR_ON == intr_get_status())
    {
        old_status = INTR_ON;
        asm volatile ("cli":::"memory");
        return old_status;
    }
    else 
    {
        old_status = INTR_OFF;
        return old_status;
    }
}

/* 将中断状态设置为status */
enum intr_status intr_set_status(enum intr_status status)
{
    return status & INTR_ON ? intr_enable() : intr_disable();
}


/* 在中断处理程序数组第vector_no个元素中注册安装中断处理程序function */
void register_handler(uint8_t vector_no,intr_handler function)
{
    /* idt_table数组中的函数是在进入中断后根据中断向量号调用的 */
    idt_table[vector_no] = function;
}
