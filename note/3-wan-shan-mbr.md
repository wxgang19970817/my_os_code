---
description: 配套代码:v0.2、v0.3
---

# 3\_完善MBR

### 地址、section、vstart

* **地址**：地址就是数字，描述各种符号在源程序中的位置，它是源代码文件中各符号偏移文件开头的距离
* 编译器的工作就是给各符号（变量名或函数名）编址，其地址就相当于各符号相对于文件开头的偏移量
* 本质上，程序中各种数据结构的访问，就是通过“该数据结构的起始地址＋该数据类型结构所占内存的大小”来实现的，这也就是定义变量要给出变量类型的原因，变量类型规定了变量所占的内存大小
* **section**:编译器提供的关键字，不对程序中的地址产生任何影响，section.节名.start代表获得某一节在文件中的偏移地址
* **vstart**:section用vstart=修饰之后，可以被赋予一个虚拟地址，用来计算在该section内的所以内存引用地址。

### **CPU工作原理**

* CPU大致上可以分为三个部分：
  * 控制单元：cpu的控制中心，大致由指令寄存器IR、指令译码器ID、操作寄存器OC组成
  * 存储单元：cpu内部的L1、L2缓存及寄存器
  * 运算单元：负责算术运算和逻辑运算的逻辑电路，从控制单元那里接收命令的执行单元
* **工作原理**：控制单元要取下一条待运行的指令，该指令的地址在程序计数器即CS:IP中，读取IP寄存器后，将此地址送上地址总线，CPU根据此地址便得到了指令，将其存入指令寄存器中，通过指令译码器解码指令操作码、操作数，如果操作数在内存中，还要将其从内存中取出放入自己的存储单元，之后运算单元开始计算。IP寄存器中的值被加上当前指令的大小，于是ip寄存器又指向了下一条指令的地址

### 实模式体系结构

* 实模式下默认用到的寄存器都是16位宽的
* CPU寄存器大致分为两大类：
  * **内部使用，对程序员不可见**。比如：全局描述符表GDTR、中断描述符表寄存器IDTR、局部描述符表LDTR、任务寄存器TR、控制寄存器CR0\~3、指令寄存器IP、标志寄存器flags、调试寄存器DR0\~7
  * 对程序员可见的寄存器，在进行汇编语言设计时，能够直接操作的就是这些，如：段寄存器和通用寄存器
* 段寄存器：代码段寄存器CS、数据段寄存器DS、栈段寄存器SS、附加段寄存器ES、FS(32位模式下添加)、GS(32)
* 通用寄存器有８个:AX、BX、CX、DX、SI、DI、BP、SP

### 函数调用时的堆栈框架

```
int a = 0;
function(int b,int c)
{
    int d;
}
a++;
```

假设这是在32位下：

1. 调用funtion(1,2);按照c语言规范，参数入栈顺序从右到左:先压入2,再压入1,每个参数在栈中各占4字节
2. 栈中再压入funtion的返回地址，此时栈顶的值是执行a++相关指令的地址
3. push ebp : 将ebp压入栈，栈中备份ebp的值，占用4字节
4. mov ebp,esp : 将esp的值复制到ebp,ebp作为堆栈框架的基址(也叫栈帧指针)，可用于对栈中的局部变量和其他的寻址
5. sub esp,4 : 由于函数中有局部变量d,这里为存放在栈中的局部变量预留空间
6. （之后在此堆栈框架的寻址都以ebp为基址进行寻址）
7. 函数结束后,跳过局部变量的空间：mov esp,ebp
8. 恢复ebp的值：pop ebp
9. 然后是返回指令，根据ABI规定，由主调函数回收栈空间

### CISC和RISC

* 复杂指令集(Complex Instruction Set Computing)为了提高运算速度，减小编译器复杂度，将越来越多的复杂指令添加到指令系统中，尽量一个指令就能干好多事儿。比如上一点中，进入函数调用时，有一条指令"enter"，功能是备份ebp并使ebp更新为esp。因此其电路单元复杂，功耗高
* 精简指令集(Reduced Instruction Set Computing)相对CISC指令更少，虽然效率低，但是可以利用流水技术和超标量技术加以改进，但汇编语言程序需要的内存空间较大，不易设计，但电路单元简单，功耗低。

### 零散知识点

* x86中的x是个变量，指代Intel所有86系列的产品，而8086之所以这么出名，是因为“段基址＋段内偏移地址”的策略首次在8086上出现。
* **地址线位宽和寄存器位宽是没有必然联系的**
* ****