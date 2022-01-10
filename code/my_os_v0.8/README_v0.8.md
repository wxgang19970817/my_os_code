# v0.8

## 文件结构

## 学习心得

### 中断分类

### 中断描述符表

### 中断处理过程

### 中断发生时的压栈

## 调试经验

1. gcc编译.c文件时用到的选项　-c 只编译　-m32 32位格式  -fno-stack-protector  栈空间保护，避免链接时出现栈溢出错误

2. 在编译汇编文件时，有时候会出现:

   ```c++
   error: comma, colon, decorator or end of line expected after operand
   ```

   此时如果排查了没有常见的语法错误，可以考虑是注释太长换行而导致的问题.

3. ld链接器生成32位文件的选项是　-m elf_i386，指定虚拟入口地址选项　-Ttext，指定入口符号　-e

4. 查看elf文件的命令：xxd，readelf

5. 在bochs调试中，使用print-stack打印出来的栈:上面是顶，新数据，下面是栈底旧数据

6. 在bochs调试中，eflags中的各个位大写代表为1