;------------------
%include "boot.inc"
;------------------
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR


;构建　gdt 及其内部的描述符
;gdt开始
GDT_BASE:
	dd 0x00000000
	dd 0x00000000
CODE_DESC:
	dd 0x0000FFFF
	dd DESC_CODE_HIGH4
DATA_STACK_DESC:
	dd 0x0000FFFF
	dd DESC_DATA_HIGH4
VIDEO_DESC:
	dd 0x80000007 			;limit=(0xbffff-0xb8000)/4k = 0x7
	dd DESC_VIDEO_HIGH4
;gdt结束

	GDT_SIZE equ $ - GDT_BASE	;利用　equ 伪指令定义数据常量　不会占用汇编地址也不会产生占用任何内存单元　在预处理阶段被替换　类似于宏
	GDT_LIMIT equ GDT_SIZE - 1

        SELECTOR_CODE equ (0x0001 << 3) + TI_GDT + RPL0
        SELECTOR_DATA equ (0x0002 << 3) + TI_GDT + RPL0
        SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0

	times 60 dq 0 			;为GDT预留60个8字节
	
	total_mem_bytes dd 0 		;total_mem_bytes 用于保存内存容量　以字节为单位
					;当前偏移量：4*8+60*8 = 512 即　0x200 ,loader_bin的加载地址是　0x900,故此处的地址是　0xb00
	
					
	gdt_ptr dw GDT_LIMIT		;GDT指针　前两字节是GDT的限制值　后四个字节是GDT的基地址
		dd GDT_BASE

	ards_buf times 244 db 0		

	ards_nr dw 0			;用于记录ARDS结构体数量
					;total_mem_bytes 4 + gdt_ptr 6 + ards_buf 244 + ards_nr 2 = 256


loader_start:			
;INT 0x15 功能号：0000E820h 功能：获取内存布局

	xor ebx,ebx			;初次使用，要清0
	mov edx,0x534d4150		;edx只赋值一次，固定签名标记，循环体中不会改变
	mov di,ards_buf			;ards结构缓冲区
		
    .e820_mem_get_loop:
	mov eax,0x0000e820		;执行int 0x15后，eax的值变为0x534d4150,所以每次都要在执行 int 前更新子功能号

	mov ecx,20			;ARDS地址范围描述符结构大小是 20 字节，用来指示BIOS写入的字节数

    	int 0x15

	jc .e820_failed_so_try_e801	;若 cf 位为1,则有错误发生，尝试 0xe801 子功能

	add di,cx			;使di增加20字节指向缓冲区中新的ARDS结构位置

	inc word [ards_nr]		;记录ARDS数量，inc dec 比　add sub 快，占用空间小，不影响CF进位标志位

	cmp ebx,0			;ebx 为0,Cf 为0，代表是最后一个ards

	jnz .e820_mem_get_loop

	
;在所有ards结构中找出(base_add_low + length_low)的最大值，即内存的容量
	
	mov cx,[ards_nr]
	mov ebx,ards_buf
	xor edx,edx

    .find_max_mem_area:
	mov eax,[ebx]
	add eax,[ebx + 8]		;base_add_low + length_low

	add ebx,20			;指向下一个ARDS结构

	cmp edx,eax			;冒泡排序，找出最大，edx寄存器始终是最大的内存容量

	jge .next_ards			;great equal 

	mov edx,eax

    .next_ards:
	loop .find_max_mem_area
	jmp .mem_get_ok
	
;--------- int 15h 子功能　E801h ------------------------
    .e820_failed_so_try_e801:
	mov ax,0xe801
	int 0x15
	jc .e801_failed_so_try88 	

;先算出低15MB的内存
;ax和cx中是以KB为单位，先转化为以 byte 为单位
	mov cx,0x400
	mul cx
	shl edx,16			;逻辑左移　最高位进入CF,低位补0
	and eax,0x0000FFFF
	or edx,eax
	add edx,0x100000
	mov esi,edx
;再将16MB以上的内存转换为64kb为单位的内存数量
	xor eax,eax
	mov ax,bx
	mov ecx,0x10000
	mul ecx

	add esi,eax
	mov edx,esi
	jmp .mem_get_ok

;---------- int 15h 子功能　0x88 ------------------------
     .e801_failed_so_try88:
	mov ah,0x88
	int 0x15
	jc .error_hlt
	and eax,0x0000FFFF

	mov cx,0x400
	mul cx
	shl edx,16
	or edx,eax
	add edx,0x100000

    .mem_get_ok:

	mov [total_mem_bytes],edx	;将内存换为 byte 单位后存入　total_mem_bytes处


;--------------------　准备进入保护模式　----------------
;1 打开A20地址线
        in al,0x92              ;寄存器端口8位
        or al,0000_0010B
        out 0x92,al

; 加载　GDT
        lgdt [gdt_ptr]

; 将 cr0 的第0位置为1 
        mov eax,cr0
        or  eax,0x00000001
        mov cr0,eax


        jmp dword SELECTOR_CODE:.p_mode_start	;jmp的两个作用　1.段描述符缓冲器的值还是实模式下的值，进入保护模式需要更新。而往相应段寄存器里加载选择子，就会导致段描述符更新
						;在进入32位模式的之前，送上流水线的都是16位的指令，CPU遇到jmp只能将之前预测送上流水线的指令清空，从而保证都是32位的指令

    .error_hlt:
	hlt    			;挂起暂停　只有RESET(复位),NMI(非屏蔽中断请求),INTR(可屏蔽的外部中断请求)可以使其退出暂停状态	


[bits 32]
.p_mode_start:
        mov ax,SELECTOR_DATA
        mov ds,ax
        mov es,ax
        mov ss,ax
        mov esp,LOADER_STACK_TOP
        mov ax,SELECTOR_VIDEO
        mov gs,ax






        mov byte [gs:160],'P'

        jmp $
	














	
	
