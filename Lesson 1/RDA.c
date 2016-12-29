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
