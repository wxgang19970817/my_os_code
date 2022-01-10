PRJ=prog_pipe
AS=nasm
CC=gcc
BIN=$(PRJ).bin
Map=$(PRJ).map
LD =ld
LIB= -I ../lib -I ../lib/kernel/ -I ../lib/user/ -I ../kernel/ -I ../device/ -I ../thread/ -I \
      ../userprog/ -I ../fs/ -I ../shell/
CFLAGS = -m32 -Wall $(LIB) -c -fno-builtin -fno-stack-protector -W -Wstrict-prototypes \
      -Wmissing-prototypes -Wsystem-headers
LDFLAGS =-e main -m elf_i386 -Map $(Map)
OBJS = ./$(PRJ).o ../build/string.o ../build/syscall.o ../build/stdio.o ../build/assert.o  start.o
C_LIB=simple_crt.a
########## 汇编代码 #########
start.o : start.S
	$(AS) -f elf $< -o $@

########## c代码 ########
$(PRJ).o : $(PRJ).c 
	$(CC) $(CFLAGS) -o $@ $< 

####### C 运行库 #####
$(C_LIB) : $(OBJS)
	ar rcs $(C_LIB) $(OBJS)
##### 链接 ######
$(BIN) : $(C_LIB)
	$(LD) $(LDFLAGS) $^ -o $@
build:$(BIN)

hd:
	dd if=./$(BIN) of=/home/wxgang/bochs/hd.img bs=512 count=10 seek=300 conv=notrunc

clean:
	rm  $(BIN) $(Map) $(PRJ).o 
all: build hd