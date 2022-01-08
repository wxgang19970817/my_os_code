CC=gcc
BIN=prog_no_arg.bin
Map=prog_no_arg.map
LD =ld
LIB= -I ../lib
CFLAGS = -m32 -Wall $(LIB) -c -fno-builtin -fno-stack-protector -W -Wstrict-prototypes \
      -Wmissing-prototypes -Wsystem-headers
LDFLAGS =-e main -m elf_i386 -Map $(Map)
OBJS = ./prog_no_arg.o ../build/string.o ../build/syscall.o ../build/stdio.o ../build/assert.o  


########## c代码 ########
prog_no_arg.o : prog_no_arg.c ../lib/stdio.h
	$(CC) $(CFLAGS) -o $@ $< 




##### 链接 ######
$(BIN) : $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@
build:$(BIN)

hd:
	dd if=./prog_no_arg.o of=/home/wxgang/bochs/hd.img bs=512 count=10 seek=300 conv=notrunc

clean:
	rm  $(BIN) $(Map) prog_no_arg.o
all: build hd