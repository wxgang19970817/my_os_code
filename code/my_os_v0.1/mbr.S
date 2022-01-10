;主引导程序
;---------------------------------------
SECTION MBR vstart=0x7c00
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov fs,ax
	mov sp,0x7c00

;清屏利用 0x06 号功能，上卷全部行，则可清屏。		;这里的 0x10 中断就是 BIOS 在执行时，在0x000~0x3FF 处建立的中断向量表IVT中的一项
;INT 0x10 功能号：0x06

	mov ax,0x600
	mov bx,0x700
	mov cx,0	;左上角:(0,0)
	mov dx,0x184f   ;右下角:(80,25)		VGA文本模式 一行80个字符，共25行
	
	int 0x10	

;获取当前光标位置，在光标位置打印字符
	mov ah,3	;3号子功能
	mov bh,0	;待获取光标的页号
		
	int 0x10	;输出：ch=光标开始行 ,cl=光标结束行， dh=行， dl=列

;打印字符串
	mov ax,message
	mov bp,ax	;es:bp 为串首地址，在程序开头初始化为该段基地址

			;该功能号使用的是存储在DH,DL中的行列号

	mov cx,5	;字符串长度
	mov ax,0x1301   ;al 设置写字符方式

	mov bx,0x2

	int 0x10


	jmp $		;程序悬停


	message db "1 MBR"
	times 510-($-$$) db 0
	db 0x55,0xaa

