# 从0到100开发一个C语言编译器（1）- 学习lotabout的系列文章

##### 作者简介 #####
KennyWZhang，前阿里P6工程师、腾讯T3技术专家（自封的，哈哈），编译器开发爱好者（真的，哈哈哈）。

关于编译器开发入门的文章中，到目前为止，网络上我见过的最好文章，莫过于[lotabout大牛](http://lotabout.me)写的“手把手教你构建 C 语言编译器”系列。（原谅我不知道他的真实姓名，姑且称他做lotabout大牛）lotabout大牛的文章写得精炼明了，可以让一个对《编译原理》不太理解的新手开发出一个简单的C语言子集的编译器原型，这说明lotabout大牛不但技术功底好，而且写作能力也是非常不错的。

“手把手教你构建 C 语言编译器”系列共有10个部分：
1. [手把手教你构建 C 语言编译器（0）——前言](http://lotabout.me/2015/write-a-C-interpreter-0/)
1. 手把手教你构建 C 语言编译器（1）——设计
1. 手把手教你构建 C 语言编译器（2）——虚拟机
1. 手把手教你构建 C 语言编译器（3）——词法分析器
1. 手把手教你构建 C 语言编译器（4）——递归下降
1. 手把手教你构建 C 语言编译器（5）——变量定义
1. 手把手教你构建 C 语言编译器（6）——函数定义
1. 手把手教你构建 C 语言编译器（7）——语句
1. 手把手教你构建 C 语言编译器（8）——表达式
1. 手把手教你构建 C 语言编译器（9）——总结

## 编程环境 ##
操作系统：阿里云服务器CentOS 6.5
代码编辑器：vim
编译器：GCC
调试器：GDB

## 第一步 ##
在开发编译器之前，lotabout大牛教我们先来设计虚拟机及其指令集。

### MOV ###
IMM <num>: 将 <num> 放入寄存器 ax 中。
LC: 将对应地址中的字符载入 ax 中，要求 ax 中存放地址。
LI: 将对应地址中的整数载入 ax 中，要求 ax 中存放地址。
SC: 将 ax 中的数据作为字符存放入地址中，要求栈顶存放地址。
SI: 将 ax 中的数据作为整数存放入地址中，要求栈顶存放地址。
```
void eval() {
    int op, *tmp;
    while (1) {
        if (op == IMM)       {ax = *pc++;}                                     // load immediate value to ax
        else if (op == LC)   {ax = *(char *)ax;}                               // load character to ax, address in ax
        else if (op == LI)   {ax = *(int *)ax;}                                // load integer to ax, address in ax
        else if (op == SC)   {ax = *(char *)*sp++ = ax;}                       // save character to address, value in ax, address on stack
        else if (op == SI)   {*(int *)*sp++ = ax;}                             // save integer to address, value in ax, address on stack
    }
    ...
    return 0;
}
```

### PUSH ###
PUSH: 将 ax 的值放入栈中。
```
if (op == PUSH) {*--sp = ax;}                                     // push the value of ax onto the stack
```

### JMP ###
JMP <addr>: 跳转指令，无条件地将当前的PC寄存器设置为指定的<addr>
```
if (op == JMP)  {pc = (int *)*pc;}                                // jump to the address
```

### JZ/JNZ ###
JZ: 结果（ax）为零情况下的跳转
JNZ: 结果（ax）不为零情况下的跳转
```
if (op == JZ)   { // jump if ax is zero
	if (ax == 0 ) {
		pc = pc + 1;
	} else {
		pc = (int *)*pc;
	}
}

if (op == JZ)   { // jump if ax is not zero
	if (ax == 0 ) {
		pc = (int *)*pc;
	} else {
		pc = pc + 1;
	}
}                  
```

### 函数调用 ###
CALL <addr>: 跳转到地址为<addr>的子函数
```
if (op == CALL) {
	*--sp = (int)(pc + 1);
	pc = (int *)*pc;
}
```
调用函数传递参数给被调用函数和被调用函数返回结果给调用函数
1. 由调用者将参数入栈
1. 调用结束时，由调用者将参数出栈
1. 参数逆序入栈

ENT <size>: 保存当前的栈指针，同时在栈上保留一定的空间，用以存放局部变量
```
if (op == ENT)  { // make new stack frame
	*--sp = (int)bp;
	bp = sp;
	sp = sp - *pc++;
}
```
ADJ <size>: 将调用子函数时压入栈中的数据清除
```
if (op == ADJ)  {sp = sp + *pc++;}                                // add esp, <size>
```
LEV: 函数调用返回
```
if (op == LEV)  { // restore call frame and PC
	sp = bp;
	bp = (int *)*sp++;
	pc = (int *)*sp++;
}
```
LEA <offset>: 得到 new_bp + 4
```
if (op == LEA)  {ax = (int)(bp + *pc++);}                         // load address for arguments
```
### 运算符指令 ###
计算后会将栈顶的参数退栈，结果存放在寄存器ax中。
```
else if (op == OR)  ax = *sp++ | ax;
else if (op == XOR) ax = *sp++ ^ ax;
else if (op == AND) ax = *sp++ & ax;
else if (op == EQ)  ax = *sp++ == ax;
else if (op == NE)  ax = *sp++ != ax;
else if (op == LT)  ax = *sp++ < ax;
else if (op == LE)  ax = *sp++ <= ax;
else if (op == GT)  ax = *sp++ >  ax;
else if (op == GE)  ax = *sp++ >= ax;
else if (op == SHL) ax = *sp++ << ax;
else if (op == SHR) ax = *sp++ >> ax;
else if (op == ADD) ax = *sp++ + ax;
else if (op == SUB) ax = *sp++ - ax;
else if (op == MUL) ax = *sp++ * ax;
else if (op == DIV) ax = *sp++ / ax;
else if (op == MOD) ax = *sp++ % ax;
```
### 内置函数 ###
```
else if (op == EXIT) { printf("exit(%d)", *sp); return *sp;}
else if (op == OPEN) { ax = open((char *)sp[1], sp[0]); }
else if (op == CLOS) { ax = close(*sp);}
else if (op == READ) { ax = read(sp[2], (char *)sp[1], *sp); }
else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
else if (op == MALC) { ax = (int)malloc(*sp);}
else if (op == MSET) { ax = (int)memset((char *)sp[2], sp[1], *sp);}
else if (op == MCMP) { ax = memcmp((char *)sp[2], (char *)sp[1], *sp);}
```
这里的原理是，我们的电脑上已经有了这些函数的实现，因此编译编译器时，这些函数的二进制代码就被编译进了我们的编译器，因此在我们的编译器/虚拟机上运行我们提供的这些指令时，这些函数就是可用的。换句话说就是不需要我们自己去实现了

### 测试 ###
```

int poolsize;         // default size of text/data/stack
int line;             // line number
int *text,            // text segment
    *stack;           // stack
int *pc, *sp, ax; // virtual machine registers

// instructions
enum { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
       OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };

int eval() {
    int op;
    while (1) {
        op = *pc++; // get next operation code

        if (op == IMM) { // load immediate value to ax
        	ax = *pc++;
        }  else if (op == PUSH) {
        	*--sp = ax;
        } else if (op == ADD) {
        	ax = *sp++ + ax;
        } else if (op == EXIT) {
        	printf("exit(%d)", *sp);
        	return *sp;
        } else {
            printf("unknown instruction:%d\n", op);
            return -1;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{

	poolsize = 256 * 1024; // arbitrary size

	// allocate memory for virtual machine
    if (!(text = malloc(poolsize))) {
        printf("could not malloc(%d) for text area\n", poolsize);
        return -1;
    }

    if (!(stack = malloc(poolsize))) {
        printf("could not malloc(%d) for stack area\n", poolsize);
        return -1;
    }

    memset(text, 0, poolsize);
    memset(stack, 0, poolsize);
    sp = (int *)((int *)stack + poolsize);
    ax = 0;

    i = 0;
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;
    pc = text;

    eval();
}
```

编译程序 `gcc -m32 compiler0.c`，运行程序：`./a.out`。(在32位的机器上编译时，不需要加-m32参数)输出
```
exit(30)
```

##### 联系方式 #####
* 邮箱：victorypiter@msn.com（欢迎大家发邮件跟我交流）
* QQ群：399128966
* 简书：[Kenny_W_Zhang](http://www.jianshu.com/users/fcfa0c6182e4)