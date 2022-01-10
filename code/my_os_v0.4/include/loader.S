;------------------
%include "boot.inc"
;------------------
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR

jmp loader_start

;构建　gdt 及其内部的描述符
;gdt开始
GDT_BASE:
	dd 0x0000_0000
	dd 0x0000_0000
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

	times 60 dq 0 			;为GDT预留60个４字节
	
	SELECTOR_CODE equ (0x0001 << 3) + TI_GDT + RPL0
	SELECTOR_DATA equ (0x0002 << 3) + TI_GDT + RPL0
	SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0

;GDT指针　前两字节是GDT的限制值　后四个字节是GDT的基地址
	gdt_ptr dw GDT_LIMIT
		dd GDT_BASE

	loadermsg db 'real model,wxg'

    .loader_start:			;.loader_start 和　loader_start 都可以　都是标号　代表地址
;INT 0x10 功能号：0x13 功能：打印字符串
	mov sp,LOADER_BASE_ADDR
	mov bp,loadermsg		;ES:BP 字符串地址
	mov cx,14			;cx 字符串长度
	mov ax,0x1301
	mov bx,0x001f			;
	mov dx,0x1800
	
	int 0x10

;--------------------　准备进入保护模式　----------------
;1 打开A20地址线
	in al,0x92		;寄存器端口8位
	or al,0000_0010B
	out 0x92,al

; 加载　GDT
	lgdt [gdt_ptr]

; 将 cr0 的第0位置为1 
	mov eax,cr0
	or  eax,0x0000_0001　
	mov cr0,eax


	jmp dword SELECTOR_CODE:p_mode_start


[bits:32]
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
























	
	
