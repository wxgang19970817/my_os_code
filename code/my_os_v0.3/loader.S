%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR

;输出背景色绿色，前景色红色，并且跳动的字符串“WXG”
	mov byte [gs:0x00],'W'
	mov byte [gs:0x01],0xA4

	mov byte [gs:0x02],'X'
	mov byte [gs:0x03],0xA4

	mov byte [gs:0x04],'G'
	mov byte [gs:0x05],0xA4

	jmp $

