;-------------------- loader and kernel ---------------
LOADER_START_SECTOR equ 0x2

LOADER_BASE_ADDR equ 0x900

PAGE_DIR_TABLE_POS equ 0x100000  ;页目录表的位置放在物理地址 0x10000

;-------------------- gdt 描述符属性　--------------
DESC_G_4K 		equ	1000_0000_0000_0000_0000_0000B   ;第23位 G位  为1　段界限的粒度是4KB

DESC_D_32      	 	equ	100_0000_0000_0000_0000_0000B    ;第22位 D位(对代码段)　指令中有效地址及操作数是32位

DESC_L          	equ	00_0000_0000_0000_0000_0000B     ;第21位

DESC_AVL  		equ	0_0000_0000_0000_0000_0000B      ;第20位

DESC_LIMIT_CODE2	equ     1111_0000_0000_0000_0000B	 ;第16-19位　代码段段界限值的16-19位
DESC_LIMIT_DATA2	equ 	DESC_LIMIT_CODE2
DESC_LIMIT_VIDEO2	equ     0000_0000_0000_0000_0000B	 ;第16-19位　段界限值的16-19位 原书此处有错误

DESC_P			equ     1000_0000_0000_0000B		 ;第15位　段存在于内存中

DESC_DPL_0		equ     000_0000_0000_0000B		 ;第13-14位　DPL 描述符特权级　0级
DESC_DPL_1		equ     010_0000_0000_0000B		 ;第13-14位　DPL 描述符特权级　1级
DESC_DPL_2		equ     100_0000_0000_0000B		 ;第13-14位　DPL 描述符特权级　2级
DESC_DPL_3		equ     110_0000_0000_0000B		 ;第13-14位　DPL 描述符特权级　3级

DESC_S_CODE		equ	1_0000_0000_0000B		 ;第12位　S位　1_当前描述符描述的是系统段 0_数据段（代码段也属于此类）
DESC_S_DATA		equ	1_0000_0000_0000B		 ;第12位　S位　1_当前描述符描述的是系统段 0_数据段（代码段也属于此类）
DESC_S_SYS		equ	0_0000_0000_0000B		 ;第12位　S位　1_当前描述符描述的是系统段 0_数据段（代码段也属于此类）

DESC_TYPE_CODE		equ 	1000_0000_0000B			 ;第8-11位　type位　可执行　非一致性　不可读　已访问位清0
DESC_TYPE_DATA		equ 	0010_0000_0000B			 ;第8-11位　type位　不可执行　向下扩展　可读写　已访问位清0


DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P + DESC_DPL_0 +\
		    DESC_S_CODE + DESC_TYPE_CODE + 0x00 
DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + DESC_P + DESC_DPL_0 +\
		    DESC_S_DATA + DESC_TYPE_DATA + 0x00 
DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + DESC_DPL_0 +\
DESC_S_DATA + DESC_TYPE_DATA + 0x0b 

;------------------------------选择子属性-------------
TI_GDT equ 000B
TI_LDT equ 100B

RPL0 equ 00B	;请求特权级
RPL1 equ 01B	;请求特权级
RPL2 equ 10B	;请求特权级
RPL3 equ 11B	;请求特权级

;------------------------- 页表相关属性　---------------
PG_P equ 1b
PG_RW_R equ 00b
PG_RW_W equ 10b
PG_US_S equ 000b
PG_US_U equ 100b

