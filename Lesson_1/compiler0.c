#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

int poolsize;         // default size of text/stack
int *text,            // text segment
    *stack;           // stack, moves from hight address to low address
int *pc, *sp, ax; // virtual machine registers

// instructions
enum {
    LEA, IMM, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, LI, LC, SI ,SC, PUSH,
    OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,
    OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, EXIT
};

int eval() {
    int op;
    while (1) {
        op = *pc++; // get next operation code

        if (op == IMM) { // load immediate value to ax
        	ax = *pc;
            pc++;
        }  else if (op == PUSH) {
            --sp; // move the stack pointer to insert an element into the stack
        	*sp = ax;
        } else if (op == ADD) {
        	ax = *sp + ax; // add the value in ax register with the value in stack top
            sp++; // move the stack pointer to pop an element off the stack
        } else if (op == EXIT) {
        	printf("exit(%d)\n", *sp); // print out the value in stack top
        	return *sp; // on exit instruction, shut down the virtual machine
        } else {
            printf("unknown instruction: %d\n", op);
            return -1; // on unknown instruction, shut down the virtual machine
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {

	poolsize = 256; // assume the size of text area and stack area is 256 bytes
    // since one instruction need 8 bytes space, the text area contains max 32 instructions

    // The malloc() function allocates size bytes of memory and returns a
    // pointer to the allocated memory.

    // allocate memory for text area
    if (!(text = malloc(poolsize))) {
        printf("could not allocate memory(%d bytes) for text area\n", poolsize);
        return -1;
    }

    // allocate memory for stack area
    if (!(stack = malloc(poolsize))) {
        printf("could not allocate memory(%d bytes) for stack area\n", poolsize);
        return -1;
    }

    memset(text, 0, poolsize); // initialize the text area to 0
    memset(stack, 0, poolsize); // initialize the stack area to 0

    sp = (int *)((int *)stack + poolsize); // the stack moves from hight address towards low address

    ax = 0; // initialize the value of ax register to 0

    // put some testing instructions into text area
    int i = 0;
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;

    pc = text; // let the program counter pc point to the start of the text area

    eval(); // start the virtual machine to run the code in text area
}
