# 从0到100开发一个C语言编译器（1）- 学习lotabout的系列文章

###### 作者简介
KennyWZhang，前阿里P6工程师、腾讯T3技术专家（自封的，哈哈），编译器开发爱好者（真的，哈哈哈）。

## 正文
关于编译器开发入门的文章中，到目前为止，网络上我见过的最好文章，莫过于[lotabout大牛](http://lotabout.me)写的“手把手教你构建 C 语言编译器”系列。（原谅我不知道他的真实姓名，姑且称他做lotabout大牛）lotabout大牛的文章写得精炼明了，可以让一个对《编译原理》不太理解的新手开发出一个简单的C语言子集的编译器原型，这说明lotabout大牛不但技术功底好，而且写作能力也是非常不错的。

“手把手教你构建 C 语言编译器”系列共有10个部分：  
1. [手把手教你构建 C 语言编译器（0）——前言](http://lotabout.me/2015/write-a-C-interpreter-0/)  
2. 手把手教你构建 C 语言编译器（1）——设计  
3. 手把手教你构建 C 语言编译器（2）——虚拟机  
4. 手把手教你构建 C 语言编译器（3）——词法分析器  
5. 手把手教你构建 C 语言编译器（4）——递归下降  
6. 手把手教你构建 C 语言编译器（5）——变量定义  
7. 手把手教你构建 C 语言编译器（6）——函数定义  
8. 手把手教你构建 C 语言编译器（7）——语句  
9. 手把手教你构建 C 语言编译器（8）——表达式  
10. 手把手教你构建 C 语言编译器（9）——总结  

## 编程环境

操作系统：阿里云服务器CentOS 6.5  
代码编辑器：vim  
编译器：GCC  
调试器：GDB  

## 第一步
在开发编译器之前，lotabout大牛教我们先来设计虚拟机及其指令集。

### MOV
IMM <num>: 将 <num> 放入寄存器 ax 中。  
LC: 将对应地址中的字符载入 ax 中，要求 ax 中存放地址。  
LI: 将对应地址中的整数载入 ax 中，要求 ax 中存放地址。  
SC: 将 ax 中的数据作为字符存放入地址中，要求栈顶存放地址。  
SI: 将 ax 中的数据作为整数存放入地址中，要求栈顶存放地址。  

```C
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

### PUSH
PUSH: 将 ax 的值放入栈中。

```C
if (op == PUSH) {*--sp = ax;}                                     // push the value of ax onto the stack
```

### JMP
JMP <addr>: 跳转指令，无条件地将当前的PC寄存器设置为指定的<addr>

```C
if (op == JMP)  {pc = (int *)*pc;}                                // jump to the address
```

### JZ/JNZ
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

### 函数调用
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

### 运算符指令
计算后会将栈顶的参数退栈，结果存放在寄存器ax中。

```C
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

### 内置函数

```C
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

### 测试虚拟机
```C
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

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
> exit(30)

## 词法分析器和语法分析器

词法分析器，用于将字符串转化成内部的表示结构。  
语法分析器，将词法分析得到的标记流（token）生成一棵语法树。

1. `next()`: 用于词法分析，获取下一个标记token，它将自动忽略空白字符，包括空格和回车。
2. `program()`: 语法分析的入口，分析整个 C 语言程序。

### 测试语法分析器的框架

```C
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token;            // current token
char *src, *old_src;  // pointer to source code string;
int poolsize;         // default size of text/data/stack

void next() {
    token = *src++;
    return;
}

void program() {
    next();                  // get next token
    while (token > 0) {
        printf("token is: %c\n", token);
        next();
    }
}

int main(int argc, char *argv[]) {
  int i, fd;

  argv++;

  poolsize = 256 * 1024; // arbitrary size

  if ((fd = open(*argv, 0)) < 0) {
    printf("could not open(%s)\n", *argv);
    return -1;
  }

  if (!(src = malloc(poolsize))) {
    printf("could not malloc(%d) for source area\n", poolsize);
    return -1;
  }

  // read the source file
  if ((i = read(fd, src, poolsize-1)) <= 0) {
    printf("read() returned %d\n", i);
    return -1;
  }

  src[i] = -1; // add EOF character
  close(fd);

  program();
}
```

编译程序 `gcc -m32 compiler1.c`，运行程序：`./a.out compiler1.c`。输出：

```
token is: #
token is: i
token is: n
token is: c
token is: l
...
```
这里的next()函数，把一个字符当做了一个token，事实上这是不对的。每次调用next()函数，应该从未分析过的字符流中分拆出来一个的token，比如未分析过的字符流是“100 + 200”这9个字符，第一次调用next()函数时，应该分拆出“单词”100，第二次调用next()函数时，应该分拆出“单词”+，第三次调用next()函数时，应该分拆出“单词”200。那么，next()函数的主体应该是如下这个样子：

```
void next() {
    while (token = *src) {
        ++src;
        // parse token here
    }
    return;
}
```

### C语言中的token

```
// tokens and classes (operators last and in precedence order)
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};
```

#### 空格
忽略空格

```
if (token == ' ') {
  // do nothing
}
```

#### 数字
暂时只支持十进制

```
if (token >= '1' && token <= '9') {
    while (*src >= '0' && *src <= '9') {
      src++;
    }
  token = Num;
  return;
}
```

#### 加号
```
if (token == '+') {
  token = Add;
  return;
}
```

#### 测试
编译程序 `gcc -m32 compiler2.c`，运行程序：`./a.out onlyNumAndPlus.c`。输出：
> token is: 128
> token is: 157
> token is: 128

128是枚举类型Num对应的数字，157是枚举类型Add对应的数字。  
Congratulations!!!目前的词法分析器成功识别了十进制的数字Token和加号Token。

#### 支持更多的token
#### 数字
上面识别数字的逻辑只考虑了十进制的数字，事实上，C语言中的数字还有十六进制和八进制。  
1. 十六进制：以0x开头，比如0x7a；
2. 八进制：以0开头，比如05，0237；

```
if (token >= '0' && token <= '9') {
    // parse number, three kinds: dec(123) hex(0x123) oct(017)
    token_val = token - '0';
    if (token_val > 0) {
        // dec, starts with [1-9]
        while (*src >= '0' && *src <= '9') {
            token_val = token_val * 10 + (*src - '0');
            src++;
        }
    } else {
        // starts with number 0
        if (*src == 'x' || *src == 'X') {
            // hex
            ++src; // ignore 'x' or 'X'
            token = *src;
            while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F')) {
                token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
                src++;
                token = *src;
            }
        } else {
            // oct
            while (*src >= '0' && *src <= '7') {
                token_val = token_val * 8 + (*src - '0');
                src++;
            }
        }
    }
    token = Num;
    return;
}
```

#### 换行符

```
if (token == '\n') {
    ++line;
}
```

#### 宏定义
我们设计的编译器并不打算支持宏定义，所以直接跳过它们。

```
if (token == '#') {
    // skip macro, because we will not support it
    while (*src != 0 && *src != '\n') { // 0 is EOF
        src++;
    }
}
```

#### 标识符与符号表
标识符（identifier）可以理解为变量名。对于语法分析而言，我们并不关心一个变量具体叫什么名字，而只关心这个变量名代表的唯一标识。例如 int a; 定义了变量 a，而之后的语句 a = 10，我们需要知道这两个 a 指向的是同一个变量。

基于这个理由，词法分析器会把扫描到的标识符全都保存到一张表中，遇到新的标识符就去查这张表，如果标识符已经存在，就返回它的唯一标识。

那么我们怎么表示标识符呢？如下
```
struct identifier {
    int token;
    int hash;
    char * name;
    int class;
    int type;
    int value;
    int Bclass;
    int Btype;
    int Bvalue;
}
```
这里解释一下具体的含义：

    token：该标识符返回的标记，理论上所有的变量返回的标记都应该是 Id，但实际上由于我们还将在符号表中加入关键字如 if, while 等，它们都有对应的标记。
    hash：顾名思义，就是这个标识符的哈希值，用于标识符的快速比较。
    name：存放标识符本身的字符串。
    class：该标识符的类别，如数字，全局变量或局部变量等。
    type：标识符的类型，即如果它是个变量，变量是 int 型、char 型还是指针型。
    value：存放这个标识符的值，如标识符是函数，刚存放函数的地址。
    BXXXX：C 语言中标识符可以是全局的也可以是局部的，当局部标识符的名字与全局标识符相同时，用作保存全局标识符的信息。

由上可以看出，我们实现的词法分析器与传统意义上的词法分析器不太相同。传统意义上的符号表只需要知道标识符的唯一标识即可，而我们还存放了一些只有语法分析器才会得到的信息，如 type 。

由于我们的目标是能自举，而我们定义的语法不支持 struct，故而使用下列方式
Symbol table:
----+-----+----+----+----+-----+-----+-----+------+------+----
 .. |token|hash|name|type|class|value|btype|bclass|bvalue| ..
----+-----+----+----+----+-----+-----+-----+------+------+----
    |<---       one single identifier                --->|

即用一个整型数组来保存相关的ID信息。每个ID占用数组中的9个空间，分析标识符的相关代码如下：
```
int token_val;                // value of current token (mainly for number)
int *current_id,              // current parsed ID
    *symbols;                 // symbol table
// fields of identifier
enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};
void next() {
        ...
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {
            // parse identifier
            last_pos = src - 1;
            hash = token;
            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) {
                hash = hash * 147 + *src;
                src++;
            }
            // look for existing identifier, linear search
            current_id = symbols;
            while (current_id[Token]) {
                if (current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos)) {
                    //found one, return
                    token = current_id[Token];
                    return;
                }
                current_id = current_id + IdSize;
            }
            // store new ID
            current_id[Name] = (int)last_pos;
            current_id[Hash] = hash;
            token = current_id[Token] = Id;
            return;
        }
        ...
}
```
查找已有标识符的方法是线性查找符号表。

#### 字符串
在分析时，如果分析到字符串，我们需要将它存放到前一篇文章中说的`data`段中。然后返回它在`data`段中的地址。另一个特殊的地方是我们需要支持转义符。例如用`\n`表示换行符。由于本编译器的目的是达到自己编译自己，所以代码中并没有支持除`\n`的转义符，如`\t`,`\r` 等，但仍支持`\a`表示字符a的语法，如`\"`表示"。

在分析时，我们将同时分析单个字符如 'a' 和字符串如 "a string"。若得到的是单个字符，我们以 Num 的形式返回。相关代码如下：
```
if (token == '"' || token == '\'') {
    // parse string literal, currently, the only supported escape character is '\n', store the string literal into data.
    last_pos = data;
    while (*src != 0 && *src != token) {
        token_val = *src++;
        if (token_val == '\\') {
            // escape character
            token_val = *src++;
            if (token_val == 'n') {
                token_val = '\n';
            }
        }
        if (token == '"') {
            *data++ = token_val;
        }
    }
    src++;
    // if it is a single character, return Num token
    if (token == '"') {
        token_val = (int)last_pos;
    } else {
        token = Num;
    }
    return;
}
```
#### 注释
在C语言中，只支持`//`类型的注释，并不支持`/* comments */`类型的注释。
```
if (token == '/') {
    if (*src == '/') {
        // skip comments
        while (*src != 0 && *src != '\n') {
            ++src;
        }
    } else {
        // divide operator
        token = Div;
        return;
    }
}
```

#### 关键字与内置函数
虽然上面写完了词法分析器，但还有一个问题需要考虑，那就是“关键字”，例如 if, while, return 等。它们不能被作为普通的标识符，因为有特殊的含义。

一般有两种处理方法：

1. 词法分析器中直接解析这些关键字。
2. 在语法分析前将关键字提前加入符号表。

这里我们就采用**第二种方法**，将它们加入符号表，并提前为它们赋予必要的信息（还记得前面说的标识符Token字段吗？）。这样当源代码中出现关键字时，它们会被解析成标识符，但由于符号表中已经有了相关的信息，我们就能知道它们是特殊的关键字。

内置函数的行为也和关键字类似，不同的只是赋值的信息。在main函数中进行初始化如下：
```
// types of variable/function
enum { CHAR, INT, PTR };
int *idmain;

void main() {
    ...
    src = "char else enum if int return sizeof while open read close printf malloc memset memcmp exit void main";

     // add keywords to symbol table
    i = Char;
    while (i <= While) {
        next();
        current_id[Token] = i++;
    }

    // add library to symbol table
    i = OPEN;
    while (i <= EXIT) {
        next();
        current_id[Class] = Sys;
        current_id[Type] = INT;
        current_id[Value] = i++;
    }

    next(); current_id[Token] = Char; // handle void type
    next(); idmain = current_id; // keep track of main
    ...
}
```

#### 其它
其它的Token的解析就相对比较容易，直接贴上代码：
```
if (token == '=') {
    // parse '==' and '='
    if (*src == '=') {
        src++;
        token = Eq;
    } else {
        token = Assign;
    }
    return;
}
else if (token == '+') {
    // parse '+' and '++'
    // ignore '+='
    if (*src == '+') {
        src++;
        token = Inc;
    } else {
        token = Add;
    }
    return;
}
else if (token == '-') {
    // parse '-' and '--'
    // ignore '-='
    if (*src == '-') {
        src++;
        token = Dec;
    } else {
        token = Sub;
    }
    return;
}
else if (token == '!') {
    // parse '!='
    if (*src == '=') {
        src++;
        token = Ne;
    }
    return;
}
else if (token == '<') {
    // parse '<=', '<<' or '<'
    if (*src == '=') {
        src++;
        token = Le;
    } else if (*src == '<') {
        src++;
        token = Shl;
    } else {
        token = Lt;
    }
    return;
}
else if (token == '>') {
    // parse '>=', '>>' or '>'
    if (*src == '=') {
        src++;
        token = Ge;
    } else if (*src == '>') {
        src++;
        token = Shr;
    } else {
        token = Gt;
    }
    return;
}
else if (token == '|') {
    // parse '|' or '||'
    if (*src == '|') {
        src++;
        token = Lor;
    } else {
        token = Or;
    }
    return;
}
else if (token == '&') {
    // parse '&' and '&&'
    if (*src == '&') {
        src++;
        token = Lan;
    } else {
        token = And;
    }
    return;
}
else if (token == '^') {
    token = Xor;
    return;
}
else if (token == '%') {
    token = Mod;
    return;
}
else if (token == '*') {
    // ignore '*='
    // ignore pointer指针
    token = Mul;
    return;
}
else if (token == '[') {
    token = Brak;
    return;
}
else if (token == '?') {
    token = Cond;
    return;
}
else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
    // directly return the character as token;
    return;
}
```

#### 测试
编译程序 `gcc -m32 compiler3.c`，运行程序：`./a.out compiler3.c`。（自己编译自己，“自举”）输出：
```
token is: 138
token is: 133
token is: 59
token is: 138
token is: 133
token is: 59
token is: 134
token is: 159
token is: 133
token is: 59
...
```
Congratulations again!!!改进后的词法分析器已经成功识别了C语言中的绝大部分的Token。

#### 自顶向下语法分析--递归下降法

一上来就看lotabout大大牛的文章“手把手教你构建 C 语言编译器（4）- 递归下降”，是比较难看懂的。  
为了看懂那篇文章，现在我们先做一个实验：**递归下降法判断算术表达式的正确性**  
[链接](http://wenku.baidu.com/view/889db11bf242336c1fb95e1b.html)

实验对应的源代码如下：

```C
#include<stdio.h>
#include<string.h>

//全局变量
char exps[30];
char gra[30];
char prod[30] = "";
char chExp = '#';
int expSize=0,graSize=0,step=0;

//函数声明
int E();
int T();
int G();
int S();
int F();

//功能函数实现
/*打印文法*/
void printGrammar()
{
    printf("\t\t\t 递归下降分析程序构造\n\t\t（作者：董正荣,邮箱：chinadongzr@163.com）\n");
    printf("-----------------------------------------------------------\n");
    printf("\t\t\t （1）E->TG\n");
    printf("\t\t\t （2）G->+TG|－TG|ε\n");
    printf("\t\t\t （3）T->FS\n");
    printf("\t\t\t （4）S->*FS|/FS|ε\n");
    printf("\t\t\t （5）F->(E)|i\n");
    printf("-----------------------------------------------------------\n");
}

void GetExp()
{
    printf("请输入表达式：（以#结束）\t");
    gets(exps);//获得输入表达式
    expSize=strlen(exps);
    chExp=exps[0];
    printf("----------------------------------------------------------\n");
    //puts(exps);//显示
}

void printHead()
{
    printf("步骤：\t 语法栈：\t\t输入串：\t产生式：\n");
}

void printStep()
{
    printf("%d\t%-20s %10s\t \t%-15s\n",step,gra,exps,prod);
    strcpy(prod,"");
    step++;
    if(chExp=='#'&&gra[graSize-1]=='#')
    {
        printf("\n表达式分析成功！\n");
    }
}

//语法栈入栈，匹配的语法产生式顺序入栈
void pushGraStack(char* ch)
{
    for(int i=0;i<strlen(ch);i++)
    {
        gra[graSize]=ch[strlen(ch)-1-i];
        graSize++;
    }
}

//语法栈出栈，返回字符ch
char popGraStack()
{
    char ch;
    ch=gra[graSize-1];
    gra[graSize-1]='\0';
    graSize--;
    return ch;
}

//表达式出栈，匹配字符ch
void nextChar()
{
    for(int i=0;i<expSize-1;i++)
    {
        exps[i]=exps[i+1];
    }
    exps[expSize-1]='\0';
    expSize--;
    chExp=exps[0];
    printf("当前chExp是%c\n",chExp);
}

//初始化语法栈
void InitGra()
{
    gra[graSize]='#';
    graSize++;
    gra[graSize]='E';
    graSize++;
    printStep();
}

//错误打印
void printError()
{
    printf("\n表达式不匹配！\n");
}

//主程序
int main()
{
    printGrammar();//输出文法
    GetExp();//获取输入表达式
    printHead();//打印题头
    InitGra();//初始化语法栈
    E();
    printf("Recursive Down Analyse App!\n");

    return 0;
}

int E()
{
    popGraStack();
    char graE[]="TG";
    pushGraStack(graE);
    strcpy(prod,"E-->TG");
    printStep();
    T();
    G();
    return 1;
}

int T()
{
    popGraStack();
    char graT[]="FS";
    pushGraStack(graT);
    strcpy(prod,"T-->FS");
    printStep();
    F();
    S();
    return 1;
}

int G()
{
    if(chExp=='+'||chExp=='-')
    {
        popGraStack();
        char graG[]={chExp,'T','G','\0'};
        pushGraStack(graG);
        strcpy(prod,"G-->");
        strcat(prod,graG);
        printStep();
        popGraStack();
        nextChar();
        strcpy(prod,"匹配");
        printStep();
        T();
        G();
        return 1;
    }
    else
    {
        strcpy(prod,"G-->ε");
        printStep();
        popGraStack();
        strcpy(prod,"匹配");
        printStep();
        return 1;
    }
}

int F()
{
    if(chExp=='(')
    {
        popGraStack();
        char graF[]="(E)";
        pushGraStack(graF);
        strcpy(prod,"F-->(E)");
        printStep();
        popGraStack();
        nextChar();
        strcpy(prod,"匹配");
        printStep();
        E();
        if(chExp==')')
        {
            popGraStack();
            nextChar();
            strcpy(prod,"匹配");
            printStep();
            return 1;
        }
        else
        {
            printError();
            return 0;
        }
    }
    else if(chExp=='i')
    {
        strcpy(prod,"F-->i");
        printStep();
        popGraStack();
        nextChar();
        strcpy(prod,"匹配");
        printStep();
        return 1;
    }
    else
    {
        printError();
        return 0;
    }
}

int S()
{
    if(chExp=='*'||chExp=='/')
    {
        popGraStack();
        char graS[]={chExp,'F','S','\0'};
        pushGraStack(graS);
        strcpy(prod,"S-->");
        strcat(prod,graS);
        printStep();
        popGraStack();
        nextChar();
        strcpy(prod,"匹配");
        printStep();
        F();
        S();
        return 1;
    }
    else
    {
        strcpy(prod,"S-->ε");
        printStep();
        popGraStack();
        strcpy(prod,"匹配");
        printStep();
        return 1;
    }
}
```
编译`gcc RDA.c`，然后运行`./a.out`，输入`i+i*(i-i/i)#`，输出：

```
...
表达式分析成功！
```

理解了这个实验之后，我们再来看lotabout大牛的文章，现在应该就可以看懂了。^_^
* [手把手教你构建 C 语言编译器（4）——递归下降](http://lotabout.me/2015/write-a-C-interpreter-4/)<br>

编译`gcc compiler4.c`，然后运行`./a.out`，输入`2+3*(5-4/2)`，输出：
> 11

再次恭喜我们自己!!!我们已经懂得了如何写一个简单的语法分析器，离写出一个完整的C语言子集的编译器，仅一步之遥了。  
下一步，我们会完善我们的语法分析器，以支持更多的c语言中的“表达式”或者“语句”。

#### 解析变量的定义

EBNF表示
```
program ::= {global_declaration}+
global_declaration ::= enum_decl | variable_decl | function_decl
enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'] '}'
variable_decl ::= type {'*'} id { ',' {'*'} id } ';'
function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'
parameter_decl ::= type {'*'} id {',' type {'*'} id}
body_decl ::= {variable_decl}, {statement}
statement ::= non_empty_statement | empty_statement
non_empty_statement ::= if_statement | while_statement | '{' statement '}'
                     | 'return' expression | expression ';'
if_statement ::= 'if' '(' expression ')' statement ['else' non_empty_statement]
while_statement ::= 'while' '(' expression ')' non_empty_statement
```
这一节重点介绍如何解析定义枚举型变量`enum_decl`和普通变量`variable_decl`的语句，下一节将介绍如何解析定义函数的语句。

##### program()

```C
void program() {
    // get next token
    next();
    while (token > 0) {
        global_declaration();
    }
}
```

##### global_declaration()

```C
int basetype;    // the type of a declaration, make it global for convenience
int expr_type;   // the type of an expression
void global_declaration() {
    // global_declaration ::= enum_decl | variable_decl | function_decl
    //
    // enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'} '}'
    //
    // variable_decl ::= type {'*'} id { ',' {'*'} id } ';'
    //
    // function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'
    int type; // tmp, actual type for variable
    int i; // tmp
    basetype = INT;
    // parse enum, this should be treated alone.
    if (token == Enum) {
        // enum [id] { a = 10, b = 20, ... }
        match(Enum);
        if (token != '{') {
            match(Id); // skip the [id] part
        }
        if (token == '{') {
            // parse the assign part
            match('{');
            enum_declaration();
            match('}');
        }
        match(';');
        return;
    }
    // parse type information
    if (token == Int) {
        match(Int);
    }
    else if (token == Char) {
        match(Char);
        basetype = CHAR;
    }
    // parse the comma seperated variable declaration.
    while (token != ';' && token != '}') {
        type = basetype;
        // parse pointer type, note that there may exist `int ****x;`
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }
        if (token != Id) {
            // invalid declaration
            printf("%d: bad global declaration\n", line);
            exit(-1);
        }
        if (current_id[Class]) {
            // identifier exists
            printf("%d: duplicate global declaration\n", line);
            exit(-1);
        }
        match(Id);
        current_id[Type] = type;
        if (token == '(') {
            current_id[Class] = Fun;
            current_id[Value] = (int)(text + 1); // the memory address of function
            // function_declaration();
        } else {
            // variable declaration
            current_id[Class] = Glo; // global variable
            current_id[Value] = (int)data; // assign memory address
            data = data + sizeof(int);
        }
        if (token == ',') {
            match(',');
        }
    }
    next();
}
```

##### enum_declaration()

```C
void enum_declaration() {
    // parse enum [id] { a = 1, b = 3, ...}
    int i;
    i = 0;
    while (token != '}') {
        if (token != Id) {
            printf("%d: bad enum identifier %d\n", line, token);
            exit(-1);
        }
        next();
        if (token == Assign) {
            // like {a=10}
            next();
            if (token != Num) {
                printf("%d: bad enum initializer\n", line);
                exit(-1);
            }
            i = token_val;
            next();
        }
        current_id[Class] = Num;
        current_id[Type] = INT;
        current_id[Value] = i++;
        if (token == ',') {
            next();
        }
    }
}
```

##### match()

```C
void match(int tk) {
    if (token == tk) {
        next();
    } else {
        printf("%d: expected token: %d\n", line, tk);
        exit(-1);
    }
}
```

编译`gcc compiler5.c`，由于我们还没有实现`function_declaration()`函数，程序暂时无法测试。

>

#### 解析函数的定义

这里最为重要的一点是，无论是函数的参数（如 param_a）还是函数的局部变量（如 local_1）都是存放在计算机的 栈 上的。因此，与存放在 数据段 中的全局变量不同，在函数内访问它们是通过 new_bp 指针和对应的位移量进行的。因此，在解析的过程中，我们需要知道参数的个数，各个参数的位移量。

##### function_declaration()

```C
void function_declaration() {
    // type func_name (...) {...}
    //               | this part
    match('(');
    function_parameter();
    match(')');
    match('{');
    function_body();
    //match('}');                 //  ①
    // ②
    // unwind local variable declarations for all local variables.
    current_id = symbols;
    while (current_id[Token]) {
        if (current_id[Class] == Loc) {
            current_id[Class] = current_id[BClass];
            current_id[Type]  = current_id[BType];
            current_id[Value] = current_id[BValue];
        }
        current_id = current_id + IdSize;
    }
}
```

##### function_parameter()

```C
int index_of_bp; // index of bp pointer on stack
void function_parameter() {
    int type;
    int params;
    params = 0;
    while (token != ')') {
        // ①
        // int name, ...
        type = INT;
        if (token == Int) {
            match(Int);
        } else if (token == Char) {
            type = CHAR;
            match(Char);
        }
        // pointer type
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }
        // parameter name
        if (token != Id) {
            printf("%d: bad parameter declaration\n", line);
            exit(-1);
        }
        if (current_id[Class] == Loc) {
            printf("%d: duplicate parameter declaration\n", line);
            exit(-1);
        }
        match(Id);
        //②
        // store the local variable
        current_id[BClass] = current_id[Class]; current_id[Class]  = Loc;
        current_id[BType]  = current_id[Type];  current_id[Type]   = type;
        current_id[BValue] = current_id[Value]; current_id[Value]  = params++;   // index of current parameter
        if (token == ',') {
            match(',');
        }
    }
    // ③
    index_of_bp = params+1;
}
```

##### function_body()

```C
void function_body() {
    // type func_name (...) {...}
    //                   -->|   |<--
    // ... {
    // 1. local declarations
    // 2. statements
    // }
    int pos_local; // position of local variables on the stack.
    int type;
    pos_local = index_of_bp;
    // ①
    while (token == Int || token == Char) {
        // local variable declaration, just like global ones.
        basetype = (token == Int) ? INT : CHAR;
        match(token);
        while (token != ';') {
            type = basetype;
            while (token == Mul) {
                match(Mul);
                type = type + PTR;
            }
            if (token != Id) {
                // invalid declaration
                printf("%d: bad local declaration\n", line);
                exit(-1);
            }
            if (current_id[Class] == Loc) {
                // identifier exists
                printf("%d: duplicate local declaration\n", line);
                exit(-1);
            }
            match(Id);
            // store the local variable
            current_id[BClass] = current_id[Class]; current_id[Class]  = Loc;
            current_id[BType]  = current_id[Type];  current_id[Type]   = type;
            current_id[BValue] = current_id[Value]; current_id[Value]  = ++pos_local;   // index of current parameter
            if (token == ',') {
                match(',');
            }
        }
        match(';');
    }
    // ②
    // save the stack size for local variables
    *++text = ENT;
    *++text = pos_local - index_of_bp;
    // statements
    while (token != '}') {
        statement();
    }
    // emit code for leaving the sub function
    *++text = LEV;
}
```

编译`gcc compiler6.c`，由于我们还没有实现`statement()`函数，程序暂时无法测试。

>

#### 解析语句的定义

C语言区分“语句”（statement）和“表达式”（expression）两个概念。简单地说，可以认为语句就是表达式加上末尾的分号。

在我们的编译器中共识别 6 种语句：

- if (...) <statement> [else <statement>]
- while (...) <statement>
- { <statement> }
- return xxx;
- <empty statement>;
- expression; (expression end with semicolon)

##### `IF`语句的解析

翻译成汇编，伪代码：
```
if (...) <statement> [else <statement>]
  if (<cond>)                   <cond>
                                JZ a
    <true_statement>   ===>     <true_statement>
  else:                         JMP b
a:                           a:
    <false_statement>           <false_statement>
b:
```

对应的解析逻辑：
```C
if (token == If) {
    match(If);
    match('(');
    expression(Assign);  // parse condition
    match(')');
    *++text = JZ;
    b = ++text;
    statement();         // parse statement
    if (token == Else) { // parse else
        match(Else);
        // emit code for JMP B
        *b = (int)(text + 3);
        *++text = JMP;
        b = ++text;
        statement();
    }
    *b = (int)(text + 1);
}
```

##### `while`语句的解析

```C
if (token == While) {
    match(While);
    a = text + 1;
    match('(');
    expression(Assign);
    match(')');
    *++text = JZ;
    b = ++text;
    statement();
    *++text = JMP;
    *++text = (int)a;
    *b = (int)(text + 1);
}
```

##### `Return`语句的解析

```C
if (token == Return) {
    // return [expression];
    match(Return);
    if (token != ';') {
        expression(Assign);
    }
    match(';');
    // emit code for return
    *++text = LEV;
}
```

##### 其它语句

其它语句并不直接生成汇编代码，代码如下：

```C
if (token == '{') {
    // { <statement> ... }
    match('{');
    while (token != '}') {
        statement();
    }
    match('}');
}
else if (token == ';') {
    // empty statement
    match(';');
}
else {
    // a = b; or function_call();
    expression(Assign);
    match(';');
}
```

##### expression()

```C
void expression(int level) {
    // expressions have various format.
    // but majorly can be divided into two parts: unit and operator
    // for example `(char) *a[10] = (int *) func(b > 0 ? 10 : 20);
    // `a[10]` is an unit while `*` is an operator.
    // `func(...)` in total is an unit.
    // so we should first parse those unit and unary operators
    // and then the binary ones
    //
    // also the expression can be in the following types:
    //
    // 1. unit_unary ::= unit | unit unary_op | unary_op unit
    // 2. expr ::= unit_unary (bin_op unit_unary ...)

    // unit_unary()
    int *id;
    int tmp;
    int *addr;
    {
        if (!token) {
            printf("%d: unexpected token EOF of expression\n", line);
            exit(-1);
        }
        if (token == Num) {
            match(Num);

            // emit code
            *++text = IMM;
            *++text = token_val;
            expr_type = INT;
        }
        else if (token == '"') {
            // continous string "abc" "abc"


            // emit code
            *++text = IMM;
            *++text = token_val;

            match('"');
            // store the rest strings
            while (token == '"') {
                match('"');
            }

            // append the end of string character '\0', all the data are default
            // to 0, so just move data one position forward.
            data = (char *)(((int)data + sizeof(int)) & (-sizeof(int)));
            expr_type = PTR;
        }
        else if (token == Sizeof) {
            // sizeof is actually an unary operator
            // now only `sizeof(int)`, `sizeof(char)` and `sizeof(*...)` are
            // supported.
            match(Sizeof);
            match('(');
            expr_type = INT;

            if (token == Int) {
                match(Int);
            } else if (token == Char) {
                match(Char);
                expr_type = CHAR;
            }

            while (token == Mul) {
                match(Mul);
                expr_type = expr_type + PTR;
            }

            match(')');

            // emit code
            *++text = IMM;
            *++text = (expr_type == CHAR) ? sizeof(char) : sizeof(int);

            expr_type = INT;
        }
        else if (token == Id) {
            // there are several type when occurs to Id
            // but this is unit, so it can only be
            // 1. function call
            // 2. Enum variable
            // 3. global/local variable
            match(Id);

            id = current_id;

            if (token == '(') {
                // function call
                match('(');

                // pass in arguments
                tmp = 0; // number of arguments
                while (token != ')') {
                    expression(Assign);
                    *++text = PUSH;
                    tmp ++;

                    if (token == ',') {
                        match(',');
                    }

                }
                match(')');

                // emit code
                if (id[Class] == Sys) {
                    // system functions
                    *++text = id[Value];
                }
                else if (id[Class] == Fun) {
                    // function call
                    *++text = CALL;
                    *++text = id[Value];
                }
                else {
                    printf("%d: bad function call\n", line);
                    exit(-1);
                }

                // clean the stack for arguments
                if (tmp > 0) {
                    *++text = ADJ;
                    *++text = tmp;
                }
                expr_type = id[Type];
            }
            else if (id[Class] == Num) {
                // enum variable
                *++text = IMM;
                *++text = id[Value];
                expr_type = INT;
            }
            else {
                // variable
                if (id[Class] == Loc) {
                    *++text = LEA;
                    *++text = index_of_bp - id[Value];
                }
                else if (id[Class] == Glo) {
                    *++text = IMM;
                    *++text = id[Value];
                }
                else {
                    printf("%d: undefined variable\n", line);
                    exit(-1);
                }

                // emit code, default behaviour is to load the value of the
                // address which is stored in `ax`
                expr_type = id[Type];
                *++text = (expr_type == Char) ? LC : LI;
            }
        }
        else if (token == '(') {
            // cast or parenthesis
            match('(');
            if (token == Int || token == Char) {
                tmp = (token == Char) ? CHAR : INT; // cast type
                match(token);
                while (token == Mul) {
                    match(Mul);
                    tmp = tmp + PTR;
                }

                match(')');

                expression(Inc); // cast has precedence as Inc(++)

                expr_type  = tmp;
            } else {
                // normal parenthesis
                expression(Assign);
                match(')');
            }
        }
        else if (token == Mul) {
            // dereference *<addr>
            match(Mul);
            expression(Inc); // dereference has the same precedence as Inc(++)

            if (expr_type >= PTR) {
                expr_type = expr_type - PTR;
            } else {
                printf("%d: bad dereference\n", line);
                exit(-1);
            }

            *++text = (expr_type == CHAR) ? LC : LI;
        }
        else if (token == And) {
            // get the address of
            match(And);
            expression(Inc); // get the address of
            if (*text == LC || *text == LI) {
                text --;
            } else {
                printf("%d: bad address of\n", line);
                exit(-1);
            }

            expr_type = expr_type + PTR;
        }
        else if (token == '!') {
            // not
            match('!');
            expression(Inc);

            // emit code, use <expr> == 0
            *++text = PUSH;
            *++text = IMM;
            *++text = 0;
            *++text = EQ;

            expr_type = INT;
        }
        else if (token == '~') {
            // bitwise not
            match('~');
            expression(Inc);

            // emit code, use <expr> XOR -1
            *++text = PUSH;
            *++text = IMM;
            *++text = -1;
            *++text = XOR;

            expr_type = INT;
        }
        else if (token == Add) {
            // +var, do nothing
            match(Add);
            expression(Inc);

            expr_type = INT;
        }
        else if (token == Sub) {
            // -var
            match(Sub);

            if (token == Num) {
                *++text = IMM;
                *++text = -token_val;
                match(Num);
            } else {

                *++text = IMM;
                *++text = -1;
                *++text = PUSH;
                expression(Inc);
                *++text = MUL;
            }

            expr_type = INT;
        }
        else if (token == Inc || token == Dec) {
            tmp = token;
            match(token);
            expression(Inc);
            if (*text == LC) {
                *text = PUSH;  // to duplicate the address
                *++text = LC;
            } else if (*text == LI) {
                *text = PUSH;
                *++text = LI;
            } else {
                printf("%d: bad lvalue of pre-increment\n", line);
                exit(-1);
            }
            *++text = PUSH;
            *++text = IMM;
            *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
            *++text = (tmp == Inc) ? ADD : SUB;
            *++text = (expr_type == CHAR) ? SC : SI;
        }
        else {
            printf("%d: bad expression\n", line);
            exit(-1);
        }
    }

    // binary operator and postfix operators.
    {
        while (token >= level) {
            // handle according to current operator's precedence
            tmp = expr_type;
            if (token == Assign) {
                // var = expr;
                match(Assign);
                if (*text == LC || *text == LI) {
                    *text = PUSH; // save the lvalue's pointer
                } else {
                    printf("%d: bad lvalue in assignment\n", line);
                    exit(-1);
                }
                expression(Assign);

                expr_type = tmp;
                *++text = (expr_type == CHAR) ? SC : SI;
            }
            else if (token == Cond) {
                // expr ? a : b;
                match(Cond);
                *++text = JZ;
                addr = ++text;
                expression(Assign);
                if (token == ':') {
                    match(':');
                } else {
                    printf("%d: missing colon in conditional\n", line);
                    exit(-1);
                }
                *addr = (int)(text + 3);
                *++text = JMP;
                addr = ++text;
                expression(Cond);
                *addr = (int)(text + 1);
            }
            else if (token == Lor) {
                // logic or
                match(Lor);
                *++text = JNZ;
                addr = ++text;
                expression(Lan);
                *addr = (int)(text + 1);
                expr_type = INT;
            }
            else if (token == Lan) {
                // logic and
                match(Lan);
                *++text = JZ;
                addr = ++text;
                expression(Or);
                *addr = (int)(text + 1);
                expr_type = INT;
            }
            else if (token == Or) {
                // bitwise or
                match(Or);
                *++text = PUSH;
                expression(Xor);
                *++text = OR;
                expr_type = INT;
            }
            else if (token == Xor) {
                // bitwise xor
                match(Xor);
                *++text = PUSH;
                expression(And);
                *++text = XOR;
                expr_type = INT;
            }
            else if (token == And) {
                // bitwise and
                match(And);
                *++text = PUSH;
                expression(Eq);
                *++text = AND;
                expr_type = INT;
            }
            else if (token == Eq) {
                // equal ==
                match(Eq);
                *++text = PUSH;
                expression(Ne);
                *++text = EQ;
                expr_type = INT;
            }
            else if (token == Ne) {
                // not equal !=
                match(Ne);
                *++text = PUSH;
                expression(Lt);
                *++text = NE;
                expr_type = INT;
            }
            else if (token == Lt) {
                // less than
                match(Lt);
                *++text = PUSH;
                expression(Shl);
                *++text = LT;
                expr_type = INT;
            }
            else if (token == Gt) {
                // greater than
                match(Gt);
                *++text = PUSH;
                expression(Shl);
                *++text = GT;
                expr_type = INT;
            }
            else if (token == Le) {
                // less than or equal to
                match(Le);
                *++text = PUSH;
                expression(Shl);
                *++text = LE;
                expr_type = INT;
            }
            else if (token == Ge) {
                // greater than or equal to
                match(Ge);
                *++text = PUSH;
                expression(Shl);
                *++text = GE;
                expr_type = INT;
            }
            else if (token == Shl) {
                // shift left
                match(Shl);
                *++text = PUSH;
                expression(Add);
                *++text = SHL;
                expr_type = INT;
            }
            else if (token == Shr) {
                // shift right
                match(Shr);
                *++text = PUSH;
                expression(Add);
                *++text = SHR;
                expr_type = INT;
            }
            else if (token == Add) {
                // add
                match(Add);
                *++text = PUSH;
                expression(Mul);

                expr_type = tmp;
                if (expr_type > PTR) {
                    // pointer type, and not `char *`
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = MUL;
                }
                *++text = ADD;
            }
            else if (token == Sub) {
                // sub
                match(Sub);
                *++text = PUSH;
                expression(Mul);
                if (tmp > PTR && tmp == expr_type) {
                    // pointer subtraction
                    *++text = SUB;
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = DIV;
                    expr_type = INT;
                } else if (tmp > PTR) {
                    // pointer movement
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = MUL;
                    *++text = SUB;
                    expr_type = tmp;
                } else {
                    // numeral subtraction
                    *++text = SUB;
                    expr_type = tmp;
                }
            }
            else if (token == Mul) {
                // multiply
                match(Mul);
                *++text = PUSH;
                expression(Inc);
                *++text = MUL;
                expr_type = tmp;
            }
            else if (token == Div) {
                // divide
                match(Div);
                *++text = PUSH;
                expression(Inc);
                *++text = DIV;
                expr_type = tmp;
            }
            else if (token == Mod) {
                // Modulo
                match(Mod);
                *++text = PUSH;
                expression(Inc);
                *++text = MOD;
                expr_type = tmp;
            }
            else if (token == Inc || token == Dec) {
                // postfix inc(++) and dec(--)
                // we will increase the value to the variable and decrease it
                // on `ax` to get its original value.
                if (*text == LI) {
                    *text = PUSH;
                    *++text = LI;
                }
                else if (*text == LC) {
                    *text = PUSH;
                    *++text = LC;
                }
                else {
                    printf("%d: bad value in increment\n", line);
                    exit(-1);
                }

                *++text = PUSH;
                *++text = IMM;
                *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
                *++text = (token == Inc) ? ADD : SUB;
                *++text = (expr_type == CHAR) ? SC : SI;
                *++text = PUSH;
                *++text = IMM;
                *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
                *++text = (token == Inc) ? SUB : ADD;
                match(token);
            }
            else if (token == Brak) {
                // array access var[xx]
                match(Brak);
                *++text = PUSH;
                expression(Assign);
                match(']');

                if (tmp > PTR) {
                    // pointer, `not char *`
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = MUL;
                }
                else if (tmp < PTR) {
                    printf("%d: pointer type expected\n", line);
                    exit(-1);
                }
                expr_type = tmp - PTR;
                *++text = ADD;
                *++text = (expr_type == CHAR) ? LC : LI;
            }
            else {
                printf("%d: compiler error, token = %d\n", line, token);
                exit(-1);
            }
        }
    }
}
```

##### 更新`main()`函数

```C
int main(int argc, char **argv)
{
    ...

    program(); // 词法分析、语法分析、生成汇编指令放到text段

    if (!(pc = (int *)idmain[Value])) {
        printf("main() not defined\n");
        return -1;
    }

    // setup stack
    sp = (int *)((int)stack + poolsize);
    *--sp = EXIT; // call exit if main returns
    *--sp = PUSH; tmp = sp;
    *--sp = argc;
    *--sp = (int)argv;
    *--sp = (int)tmp;

    return eval(); // 启动虚拟机，执行汇编指令
}

```

编译`gcc -o compiler compiler.c`，
- 运行`compiler printf.c`，输出：

> “Hello, my compiler.”

- 运行`compiler ifStatement.c`，输出：

>

- 运行`compiler whileStatement.c`，输出：

>

- 运行`compiler functionCall.c`，输出：

>

- 运行`compiler recursiveCall.c`，输出：

>

- 运行`compiler compiler.c`，输出：

>

Congratulations!~~~ 我们想要的编译器大功告成！！！

###### 联系方式
* 邮箱：victorypiter@msn.com（欢迎大家发邮件跟我交流）
* QQ群：399128966
* 简书：[Kenny_W_Zhang](http://www.jianshu.com/users/fcfa0c6182e4)
