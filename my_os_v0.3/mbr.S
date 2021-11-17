;主引导程序
;---------------------------------------
%include "boot.inc"
;--------------------------------------
SECTION MBR vstart=0x7c00
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov fs,ax
	mov sp,0x7c00
	mov ax,0xb800				;地址空间B8000-BFFFF 的32k空间用于文本显示适配器
	mov gs,ax	

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

;输出背景色为绿色，前景色红色，并且跳动的字符串“1 MBR”

	mov byte [gs:0x00],'1'		;gs段基址 段跨越前缀 
	mov byte [gs:0x01],0xA4		;A 表示绿色背景色 4表示前景色为红色
					;低字节是ASCII码 高字节是字符属性元信息 在高字节中，低四位是前景色，高四位是背景色

	mov byte [gs:0x02],' '
	mov byte [gs:0x03],0xA4

	mov byte [gs:0x04],'M'
	mov byte [gs:0x05],0xA4

;开始准备读取硬盘
	mov eax,LOADER_START_SECTOR	;起始扇区数
	mov bx,LOADER_BASE_ADDR		;写入的地址
	mov cx,1			;待读入的扇区数
	call rd_disk_m_16		;调用 读硬盘扇区的函数
;转交控制权
	jmp LOADER_BASE_ADDR		;跳转到内核加载器的位置



;读硬盘扇区的函数
rd_disk_m_16:
	mov esi,eax			;备份eax
	mov di,cx			;备份cx
;读写硬盘
;第1步：设置要读取的扇区数
	mov dx,0x1f2			;把端口号写入dx，然后使用 out/in 指令
					;0x1f2 是 sector count 寄存器，用来指定扇区数
	mov al,cl			;硬盘寄存器端口除了data寄存器是16位，都是8位
	out dx,al			;读取的扇区数

	mov eax,esi			;恢复ax

;第2步:将 LBA 地址存入 0x1f3 ~ 0x1f6
	mov dx,0x1f3
	out dx,al			;低8位写到 0x1f3

	mov dx,0x1f4
	mov cl,8
	shr eax,cl			;逻辑右移，低位移到进位标志位CF，最高位补零
	out dx,al			;8-15位 写到 0x1f4

	mov dx,0x1f5
	shr eax,cl
	out dx,al			;16-13 写到 0x1f5

	shr eax,cl
	and al,0x0f			;LBA28 用28位描述一个扇区的地址 28-8-8-8=4
	or  al,0xe0			;device寄存器的4-7位是 1110  代表LBA模式，读的是通道中的主盘
	mov dx,0x1f6			;0x1f6 是 device寄存器
	out dx,al

;第3步	往该通道的 command 寄存器写入操作命令
	mov dx,0x1f7
	mov al,0x20			;读扇区的命令码 是 0x20
	out dx,al			;往 commend 寄存器里写入 命令后，硬盘就立即开始执行

;第4步 检测硬盘状态
    .not_ready:				;以此为轮询查找入口
	nop				;空操作，增加延迟
	in al,dx			;dx 中的值仍是 0x1f7 ,在读操作时为 status 寄存器
	and al,0x88			;取出 第3位：为1表示硬盘已准备好传输数据，取出 第7位：为1表示硬盘忙
	cmp al,0x08			;隐含减法的比较，不修改任何操作数
	jnz .not_ready			;不等于0，跳转回轮询入口

;第5步 跳出轮询代表数据已准备好，开始进行传输
	mov ax,di			;在 54行 将cx写入 di,而cx中存放的是读取的扇区数
					;每个扇区512字节，每次读入两个字节（data寄存器是16位）
	mov dx,256			;每个扇区需要读入的次数 是 256
	mul dx				;mul 默认寄存器是ax  结果高位默认在dx存放，低位在ax存放
					;div 默认寄存器是ax  结果余数默认在dx存放，商  在ax存放
	mov cx,ax			;此时ax存放的值 1*256

	mov dx,0x1f0			;端口0x1f0 是data寄存器
	
    .go_on_read:			;读磁盘
	in ax,dx
	mov [bx],ax			;bx是待写入的地址
	add bx,2			;地址增长
					;但是读入的寄存器不用更改地址 只要不断读就行

	loop .go_on_read 
	ret

	times 510-($-$$) db 0
	db 0x55,0xaa

