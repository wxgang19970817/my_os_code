BULID_DIR = ./build
ENTRY_POINT = 0xc0001500
AS = nasm
CC = gcc
LD = ld
LIB = -I lib -I lib/kernel -I kernel -I device -I thread
ASFLAGS = -f elf
CFLAGS = -Wall $(LIB) -c -m32 -fno-builtin -fno-stack-protector -W -Wstrict-prototypes -Wmissing-prototypes
LDFLAGS = -Ttext $(ENTRY_POINT) -e main -m elf_i386 -Map $(BULID_DIR)/kernel.map
OBJS = $(BULID_DIR)/main.o $(BULID_DIR)/init.o $(BULID_DIR)/interrupt.o \
	   $(BULID_DIR)/timer.o $(BULID_DIR)/kernel.o $(BULID_DIR)/print.o \
	   $(BULID_DIR)/debug.o $(BULID_DIR)/memory.o $(BULID_DIR)/bitmap.o $(BULID_DIR)/string.o $(BULID_DIR)/thread.o \
	   $(BULID_DIR)/list.o $(BULID_DIR)/switch.o

###############    c代码编译　###############
$(BULID_DIR)/main.o : kernel/main.c lib/kernel/print.h lib/stdint.h kernel/init.h
	$(CC) $(CFLAGS) -o $@ $< 

$(BULID_DIR)/init.o : kernel/init.c lib/kernel/print.h lib/stdint.h kernel/init.h \
						kernel/interrupt.h device/timer.h
	$(CC) $(CFLAGS) -o $@ $<

$(BULID_DIR)/interrupt.o : kernel/interrupt.c kernel/interrupt.h lib/stdint.h kernel/global.h \
						lib/kernel/io.h lib/kernel/print.h
	$(CC) $(CFLAGS) -o $@ $<

$(BULID_DIR)/timer.o : device/timer.c device/timer.h lib/stdint.h lib/kernel/io.h lib/kernel/print.h
	$(CC) $(CFLAGS) -o $@ $<

$(BULID_DIR)/debug.o : kernel/debug.c kernel/debug.h lib/kernel/print.h lib/stdint.h kernel/interrupt.h
	$(CC) $(CFLAGS) -o $@ $<

$(BULID_DIR)/string.o : lib/string.c lib/string.h lib/stdint.h kernel/global.h kernel/debug.h
	$(CC) $(CFLAGS) -o $@ $<

$(BULID_DIR)/bitmap.o : lib/kernel/bitmap.c lib/kernel/bitmap.h kernel/global.h lib/stdint.h lib/string.h \
						lib/kernel/print.h kernel/interrupt.h kernel/debug.h
	$(CC) $(CFLAGS) -o $@ $<

$(BULID_DIR)/memory.o : kernel/memory.c kernel/memory.h lib/stdint.h lib/kernel/bitmap.h kernel/global.h \
						kernel/debug.h lib/kernel/print.h lib/kernel/io.h kernel/interrupt.h lib/string.h
	$(CC) $(CFLAGS) -o $@ $<

$(BULID_DIR)/thread.o : thread/thread.c thread/thread.h lib/stdint.h lib/string.h kernel/global.h kernel/memory.h
	$(CC) $(CFLAGS) -o $@ $< 

$(BULID_DIR)/list.o	: lib/kernel/list.c lib/kernel/list.h kernel/global.h lib/stdint.h kernel/interrupt.h
	$(CC) $(CFLAGS) -o $@ $<

###############		汇编代码编译　#################
$(BULID_DIR)/kernel.o : kernel/kernel.S
	$(AS) $(ASFLAGS) $< -o $@

$(BULID_DIR)/print.o : lib/kernel/print.S
	$(AS) $(ASFLAGS) $< -o $@

$(BULID_DIR)/switch.o : thread/switch.S
	$(AS) $(ASFLAGS) $< -o $@

###############  	 链接所有目标文件　###############
$(BULID_DIR)/kernel.bin : $(OBJS)
	$(LD) $(LDFLAGS)  $^  -o $@ 


.PHONY : mk_dir hd clean all

mk_dir:
	if [[ ! -d $(BULID_DIR) ]];then mk_dir $(BULID_DIR);fi

hd:
	dd if=$(BULID_DIR)/kernel.bin of=/home/wxgang/bochs/hd.img bs=512 count=200 seek=9  conv=notrunc

clean:
	cd $(BULID_DIR) && rm -f ./*

build: $(BULID_DIR)/kernel.bin

all:mk_dir build hd




