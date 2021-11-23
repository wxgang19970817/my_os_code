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

;创建页目录及页表并初始化页内存位图
	call setup_page
	
;将描述符表地址及偏移量写入内存gdt_ptr，一会儿用新地址重新加载
	sgdt [gdt_ptr]

;将gdt描述符中视频段描述符中的段地址+0xc0000000
	mov ebx,[gdt_ptr + 2]			;GDT_BASE
	or dword [ebx + 0x18 + 4],0xc0000000	;第三个段描述符，每个描述符8个字节
	
;将gdt的基地址加上0xc0000000使其成为内核所在的高地址
	add dword [gdt_ptr + 2],0xc0000000

	add esp,0xc0000000			;将栈指针也映射到内核地址

;把页目录地址赋给cr3
	mov eax,PAGE_DIR_TABLE_POS
	mov cr3,eax

;打开cr0的PG 第31位
	mov eax,cr0
	or eax,0x80000000
	mov cr0,eax

;开启分页后，用gdt新的地址重新加载
	lgdt [gdt_ptr]

;视频段段基地址已经更新
        mov byte [gs:160],'V'

        jmp $
	




;------------------  创建页目录及页表　--------------------------
setup_page:
;先清空页目录项要用到的内存
	mov ecx,4096
	mov esi,0
    .clear_page_dir:
	mov byte [PAGE_DIR_TABLE_POS + esi],0
	inc esi
	loop .clear_page_dir

;开始创建页目录项(PDE)
    .create_pde:
	mov eax,PAGE_DIR_TABLE_POS	;0x10100
	add eax,0x1000			;此时 eax 为第一个页表的位置及属性 0x101000 
	mov ebx,eax			;此处 ebx 为第一个页表，为 .create_pte 做准备，ebx为基址
	
;下面将第一个页表的位置和属性同时写入第0个和第0xc00个页目录项
;这里两个页目录项指向同一个页表的原因非常重要：
;1.在完全加载内核之前，一直运行的是储存在物理地址0-1MB范围内的loader，而开启分页之后程序中的地址就变成了虚拟地址，为了能顺利找到程序，所以我们需要将物理地址的0-1MB和虚拟地址的0-1MB映射起来
;2.我们写的程序是属于操作系统的内核部分的，而在虚拟地址空间内，操作系统是占高1G地址空间的，所以要把内核实际的物理地址空间与虚拟地址空间对应起来
	or eax,PG_US_U | PG_RW_W | PG_P
	mov [PAGE_DIR_TABLE_POS + 0x0],eax  	;eax=0x1111 写入页目录项0 地址是0x101000,页表项0-bff是用户空间
	mov [PAGE_DIR_TABLE_POS + 0xc00],eax  	;eax=0x1111 写入第768个页目录项c00 地址是0x10d00,c00-fff是内核空间  
	
	sub eax,0x1000 				;0x100000 页目录表地址
	mov [PAGE_DIR_TABLE_POS + 4092],eax 	;页目录表地址写入最后一个页目录项
	
;创建页表项PTE
	mov ecx,256			;1M低端内存/每页4K = 256
	mov esi,0
	mov edx,PG_US_U | PG_RW_W | PG_P ;属性为7,US=1,RW=1,P=1 普通用户　可读写　存在于内存
    .create_pte:
	mov [ebx+esi*4],edx		;物理地址0对应第一个页表项　edx中是实际物理页地址
	add edx,4096			;下一个页
	inc esi
	loop .create_pte

;创建内核其他页表的PDE
	mov eax,PAGE_DIR_TABLE_POS
	add eax,0x2000			;第二个页表的地址
	or eax,PG_US_U | PG_RW_W | PG_P
	mov ebx,PAGE_DIR_TABLE_POS
	mov ecx,254
	mov esi,769			;从769个页表往上表示的都是内核的物理地址空间了
    .create_kernal_pde:
	mov [ebx + esi*4],eax		;第二个页表的地址写入对应内核空间的页目录项
	inc esi
	add eax,0x1000			;下一个页表日入下一个页表项
  	loop .create_kernal_pde
	
	ret
	







	
	
