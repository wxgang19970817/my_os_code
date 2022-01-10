TI_GDT equ  0
RPL0  equ   0
SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0

section .data
put_int_buffer dq 0             ;这里指定是16位模式　word是2字节　qword是4字节
                                ;定义8字节缓冲区用于数字到字符的转换

[bits 32]
section .text

;-----------------------   put_str -------------------------------
;输入:栈中参数为打印的字符串
;输出:无
global put_str
put_str:
    push ebx
    push ecx
    xor ecx,ecx
    xor ebx,ebx
    mov ebx,[esp + 12]          ;从栈中得到打印的字符串地址　两个寄存器　+  一个主调函数的返回地址
  .goon:
    mov cl,[ebx]                ;将字符串一个个传入cl            
    cmp cl,0                    ;如果到了字符串的结尾，跳到结束处返回    
    jz .str_over

    push ecx                    ;为 put_char 传递参数
    call put_char
    
    add esp,4                   ;按照c declaration 约定由调用者回收栈空间
    inc ebx                     ;使ebx指向下一个字符
    jmp .goon
  .str_over:
    pop ecx
    pop ebx
    ret
;------------------------   put_char   -----------------------------
;功能描述:把栈中的1个字符写入光标所在处
;-------------------------------------------------------------------   
global put_char
put_char:
   pushad	   ;备份32位寄存器环境,入栈顺序:EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI
   ;需要保证gs中为正确的视频段选择子,为保险起见,每次打印时都为gs赋值
   mov ax, SELECTOR_VIDEO	       ; 不能直接把立即数送入段寄存器
   mov gs, ax

;;;;;;;;;  获取当前光标位置 ;;;;;;;;;
   ;先获得高8位
   mov dx, 0x03d4  ;访问   CRT controller 寄存器组，要先往端口地址位0x3D4的Address Register寄存器写入寄存器索引
   mov al, 0x0e	   ;索引为0Eh的Cursor Location High Register 用来存储光标坐标的高8位
   out dx, al
   mov dx, 0x03d5  ;通过读写数据端口0x3d5来获得或设置光标位置 
   in al, dx	   ;得到了光标位置的高8位
   mov ah, al

   ;再获取低8位
   mov dx, 0x03d4
   mov al, 0x0f
   out dx, al
   mov dx, 0x03d5 
   in al, dx

   ;将光标存入bx
   mov bx, ax	  
   ;下面这行是在栈中获取待打印的字符
   mov ecx, [esp + 36]	      ;pushad压入4×8＝32字节,加上主调函数的返回地址4字节,再往前是参数,故esp+36字节
   cmp cl, 0xd				  ;CR是0x0d,LF是0x0a
   jz .is_carriage_return
   cmp cl, 0xa
   jz .is_line_feed

   cmp cl, 0x8				  ;BS(backspace)的asc码是8
   jz .is_backspace
   jmp .put_other	   
;;;;;;;;;;;;;;;;;;

 .is_backspace:		      
;;;;;;;;;;;;       backspace的一点说明	     ;;;;;;;;;;
; 当为backspace时,本质上只要将光标移向前一个显存位置即可.后面再输入的字符自然会覆盖此处的字符
; 但有可能在键入backspace后并不再键入新的字符,这时在光标已经向前移动到待删除的字符位置,但字符还在原处,
; 这就显得好怪异,所以此处添加了空格或空字符0
   dec bx                         ;光标是字符的坐标，该坐标是一维的线性坐标,第0行坐标0-24，共80行，每个字符占用2字节，低字节是字符的ASCII码，高字节是前、背景色属性
   shl bx,1                       ;这里乘2意味着原来的bx中是字符个数坐标　乘2的话变为了相对坐标
   mov byte [gs:bx], 0x20		  ;将待删除的字节补为0或空格皆可
   inc bx
   mov byte [gs:bx], 0x07         ;前、背景
   shr bx,1                       ;又恢复成个数坐
   jmp .set_cursor
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

 .put_other:
   shl bx, 1				  ; 光标位置是用2字节表示,将光标值乘2,表示对应显存中的偏移字节
   mov [gs:bx], cl			  ; ascii字符本身
   inc bx
   mov byte [gs:bx],0x07		  ; 字符属性
   shr bx, 1				  ; 恢复老的光标值
   inc bx				  ; 下一个光标值
   cmp bx, 2000		   
   jl .set_cursor			  ; 若光标值小于2000,表示未写到显存的最后,则去设置新的光标值
					  ; 若超出屏幕字符数大小(2000)则换行处理
 .is_line_feed:				  ; 是换行符LF(\n)
 .is_carriage_return:			  ; 是回车符CR(\r)
					  ; 如果是CR(\r),只要把光标移到行首就行了。
   xor dx, dx				  ; dx是被除数的高16位,清0.
   mov ax, bx				  ; ax是被除数的低16位.
   mov si, 80				  ; 由于是效仿linux，linux中\n便表示下一行的行首，所以本系统中，
   div si				  ; 把\n和\r都处理为linux中\n的意思，也就是下一行的行首。
   sub bx, dx				  ; 光标值减去除80的余数便是取整
					  ; 以上4行处理\r的代码

 .is_carriage_return_end:		  ; 回车符CR处理结束
   add bx, 80
   cmp bx, 2000
 .is_line_feed_end:			  ; 若是LF(\n),将光标移+80便可。  
   jl .set_cursor

;屏幕行范围是0~24,滚屏的原理是将屏幕的1~24行搬运到0~23行,再将第24行用空格填充
 .roll_screen:				  ; 若超出屏幕大小，开始滚屏
   cld  
   mov ecx, 960				  ; 一共有2000-80=1920个字符要搬运,共1920*2=3840字节.一次搬4字节,共3840/4=960次 
   mov esi, 0xb80a0			  ; 第1行行首
   mov edi, 0xb8000			  ; 第0行行首
   rep movsd				  

;;;;;;;将最后一行填充为空白
   mov ebx, 3840			  ; 最后一行首字符的第一个字节偏移= 1920 * 2
   mov ecx, 80				  ;一行是80字符(160字节),每次清空1字符(2字节),一行需要移动80次
 .cls:
   mov word [gs:ebx], 0x0720		  ;0x0720是黑底白字的空格键
   add ebx, 2
   loop .cls 
   mov bx,1920				  ;将光标值重置为1920,最后一行的首字符.

 .set_cursor:   
					  ;将光标设为bx值
;;;;;;; 1 先设置高8位 ;;;;;;;;
   mov dx, 0x03d4			  ;索引寄存器
   mov al, 0x0e				  ;用于提供光标位置的高8位
   out dx, al
   mov dx, 0x03d5			  ;通过读写数据端口0x3d5来获得或设置光标位置 
   mov al, bh
   out dx, al

;;;;;;; 2 再设置低8位 ;;;;;;;;;
   mov dx, 0x03d4
   mov al, 0x0f
   out dx, al
   mov dx, 0x03d5 
   mov al, bl
   out dx, al
 .put_char_done: 
   popad
   ret

;-------------------- 将小端字节序的数字变成对应的ASCII后，倒置　--------------------
;输入:栈中参数位待打印的数字
;输出:在屏幕上打印十六进制数字，并不会打印前缀0x
;------------------------------------------------------------------------------------
global put_int
put_int:
    pushad
    mov ebp,esp
    mov eax,[ebp + 4*9]         ;八个寄存器 + 一个主调函数的返回地址
    mov edx,eax                 ;eax获取到参数又给到edx  eax是参数的备份 edx寄存器是每次参与数位转换的寄存器
                                ;每次转换完一个十六进制数后，再由eax为其更新下一个待转换的数字
    mov edi,7                   ;指向缓冲区中最后一个1字节　在此处存储数字最低4位二进制(十六进制中的最低1位)对应的字符
    mov ecx,8                   ;需要处理的十六位进制数字是8个　32位一个数　4位一个十六进制
    mov ebx,put_int_buffer      ;ebx作为缓冲区的基址

  .16based_4bits:
    ;遍历每一位十六进制数字
    and edx,0x0000000F          ;解析十六进制数字的最后一位

    cmp edx,9
    jg .is_A2F          
    add edx,'0'                 ;在0~9内的直接把自身当偏移，加上数字0的ASCII码即得到自身对应的ASCII码
    jmp .store
  .is_A2F:
    sub edx,10                  ;在A~F内的先用自身减去10算出偏移，加上数字A的ASCII码即得到自身对应的ASCII码
    add edx,'A'

  .store:
    mov [ebx + edi],dl          ;将转换好的字符放在缓冲区的最后一个字节
    dec edi                     ;指向缓冲区的前一个字节
    shr eax,4                   ;eax中存放的参数舍去处理完的低四位
    mov edx,eax                 
    loop .16based_4bits

;现在缓冲区中已经全是字符，打印之前把高位连续的字符去掉，比如000123 变为　123
  .ready_to_print:
    inc edi                     ;在把字符全转换完成时,edi已经自减成 -1 (0xffffffff)了，加1使其为0
  .skip_prefix_0:
    cmp edi,8                   ;如果比较到了第9位代表待打印的字符全为0
    je .full0

;找出连续的0字符,edi作为非0的最高位字符的偏移
  .go_on_skip:
    mov cl,[put_int_buffer + edi]
    inc edi
    cmp cl,'0'
    je .skip_prefix_0
    dec edi
    jmp .put_each_num

  .full0:
    mov cl,'0'

  .put_each_num:
    push ecx
    call put_char
    add esp,4
    inc edi
    mov cl,[put_int_buffer + edi]
    cmp edi,8
    jl .put_each_num
    popad
    ret




















   
