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
