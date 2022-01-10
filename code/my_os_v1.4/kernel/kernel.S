[bits 32]
%define ERROR_CODE nop     ;若在相关的异常中cpu已经自动压入了错误码，为了保持栈中格式统一，这里不做操作

%define ZERO push 0        ;若在相关的异常中cpu没有压入错误码，为了统一栈中格式，就手工压入一个0

extern idt_table;          ;idt_table是在c中注册的中断处理函数数组


section .data
global intr_entry_table
intr_entry_table:

%macro VECTOR 2
section .text
intr%1entry:             ;每个中断处理程序都要压入中断,所以一个中断类型一个中断处理程序，因为参数%1的不同，会有不同的中断处理程序段
 
    %2                    ;发生中断时的入栈:CPU自动压入SS,ESP,EFLAGS,CS,EIP,(error_code)压入栈中,有的中断会压入错误码，有的不会,这里的%2会被展开为nop 或者　push 0
    
    push ds               ;先压入错误码后要保存进程上下文环境,压入4个16位的段寄存器,虽然是16位但是在32位环境下要占用32位
    push es               ;但是位于其他寄存器和内存中的值如为16位，入栈后就占用2字节
    push fs
    push gs
    pushad                ;再压入8个32位通用寄存器,入栈顺序:EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI


    ;如果是从片进入的中断，除了往从片上发送EOI外，还要往主片上发送EOI
    mov al,0x20
    out 0xa0,al
    out 0x20,al

    push %1               ;压入中断向量号方便调试

    call [idt_table + %1*4] ;调用idt_table中的C版本中断处理函数
    jmp intr_exit

section .data
    dd intr%1entry       ;在数据段保存每个中断服务程序的入口地址

%endmacro


section .text
global intr_exit
intr_exit:
    ;恢复上下文环境    
    add esp,4              ;跳过中断向量号
    popad 
    pop gs
    pop fs
    pop es
    pop ds
    add esp,4              ;跳过error_code
    iretd


VECTOR 0x00,ZERO
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO
VECTOR 0x08,ERROR_CODE
VECTOR 0x09,ZERO
VECTOR 0x0a,ERROR_CODE
VECTOR 0x0b,ERROR_CODE
VECTOR 0x0c,ZERO
VECTOR 0x0d,ERROR_CODE
VECTOR 0x0e,ERROR_CODE
VECTOR 0x0f,ZERO
VECTOR 0x10,ZERO
VECTOR 0x11,ERROR_CODE
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO
VECTOR 0x18,ERROR_CODE
VECTOR 0x19,ZERO
VECTOR 0x1a,ERROR_CODE
VECTOR 0x1b,ERROR_CODE
VECTOR 0x1c,ZERO
VECTOR 0x1d,ERROR_CODE
VECTOR 0x1e,ERROR_CODE
VECTOR 0x1f,ZERO
VECTOR 0x20,ZERO            ;时钟中断对应的入口
VECTOR 0x21,ZERO            ;键盘中断对应的入口
VECTOR 0x22,ZERO            ;级联用的
VECTOR 0x23,ZERO            ;串口2对应的入口
VECTOR 0x24,ZERO            ;串口1对应的入口
VECTOR 0x25,ZERO            ;并口2对应的入口
VECTOR 0x26,ZERO            ;软盘对应的入口
VECTOR 0x27,ZERO            ;并口1对应的入口
VECTOR 0x28,ZERO            ;实时时钟对应的入口
VECTOR 0x29,ZERO            ;重定向
VECTOR 0x2a,ZERO            ;保留
VECTOR 0x2b,ZERO            ;保留
VECTOR 0x2c,ZERO            ;ps/2鼠标
VECTOR 0x2d,ZERO            ;fpu浮点单元异常
VECTOR 0x2e,ZERO            ;硬盘
VECTOR 0x2f,ZERO            ;保留



;;;;;;;;;;;;;;;;;;;; 0x80号中断 ;;;;;;;;;;;;;;;;;;;;;
[bits 32]
extern syscall_table
section .text
global syscall_handler
syscall_handler:
        ;1 保存上下文环境
        push 0                                  ;压入0　充当中断错误码

        push ds
        push es
        push fs
        push gs
        pushad                                 ;32位寄存器全部压入

        push 0x80                           ;压入中断向量号，与其他中断处理程序保持一致

        ;为系统调用子功能准备参数
        push edx                              ;系统调用中第3个参数
        push ecx                              ;系统调用中第2个参数
        push ebx                              ;系统调用中第1个参数

        ;调用子功能处理函数
        call [syscall_table + eax * 4]  ;table中存放的是几个子功能处理函数地址，每个地址是4个字节
        add esp,12                          ;跨过上面的三个参数

        ;将call调用后的返回值存入当前内核栈中eax的位置
        mov [esp + 8*4],eax         ;此时eax中存放的是函数调用的返回值，为了在恢复上下文的时候不被覆盖，将其写入栈中eax的位置
        jmp intr_exit                       ;intr_exit返回，恢复上下文




