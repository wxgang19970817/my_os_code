[bits 32]
section .text
global switch_to
switch_to:
    ;栈中此处是返回地址
    push esi
    push edi
    push ebx
    push ebp
    mov eax,[esp + 20]  ;得到栈中的参数cur,cur=[esp + 20],cur是当前PCB的起始地址，也是task_struct的第一个结构体成员self_kstack
    mov [eax],esp       ;保存栈指针,将当前栈指针保存在self_kstack

    ;--------------------- 以上是备份当前线程的环境，下面是恢复下一个线程的环境 -------------------
    mov eax,[esp + 24]  ;得到栈中的参数 next
    mov esp,[eax]       ;将next对应PCB中的第一个成员self_kstack装入esp，对应的是0特权级栈，里面保存了进程的所有信息，包括3级栈指针

    pop ebp
    pop ebx
    pop edi
    pop esi
    ret                 ;如果next没有执行过，此时esp是pcb中的线程栈self_kstack的指针，里面的成员是ebp ebx edi esi之后就是kernel_thread
                        ;如果next之前有执行过，当时next被中断时，第一层中断处理程序保存了全部的寄存器映像，然后call intr_timer_handler,再到schedule 到switch_to函数,这里返回直接返回到第一层中断处理函数


