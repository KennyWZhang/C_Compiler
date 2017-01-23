#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

int poolsize;         // the size of text and stack area

int *text,            // text area
    *stack;           // the stack, grows from hight address to low address

int *pc, // the program counter pointer
    *sp, // the stack (top) pointer
    ax; // the ax register

// instructions
enum {
    LEA, IMM, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, LI, LC, SI ,SC, PUSH,
    OR, XOR, AND, EQ, NE, LT, GT, LE, GE,
    SHL, SHR, ADD, SUB, MUL, DIV, MOD,
    OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, EXIT
};

int eval() {
    int op;
    while (1) {
        op = *pc; // get the instruction to be executed in this cpu cycle

        pc++; // move the program counter pc
              // to let it point to the operand of current instruction
              // or next instruction

        if (op == IMM) {
        	ax = *pc; // load immediate value to ax

            pc++; // move the pc pointer
        }  else if (op == PUSH) {
            --sp; // move the sp pointer to increase the size of the stack

        	*sp = ax; // put the value of ax into the stack
        } else if (op == ADD) {
        	ax = *sp + ax; // add the value in ax with the value in stack top

            sp++; // move the sp pointer to decrease the size of the stack
        } else if (op == EXIT) {
        	printf("exit(%d)\n", *sp); // print out the value in stack top

        	return *sp; // on EXIT instruction, step out the while loop,
                        // this means shutting down the virtual machine
        } else {
            printf("unknown instruction: %d\n", op);

            return -1; // on unknown instruction, also step out the while loop,
                       // this means shutting down the virtual machine
        }
    }

    return 0;
}

int main() {

	poolsize = 256; // assume the size of text area and stack area is 256 bytes
    // since one instruction need 4 or 8 bytes space,
    // in this case, the text area contains max 64 instructions

    // The malloc(size_t size) function allocates size bytes of memory and returns a
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

    sp = (int *)((int *)stack + poolsize); // the stack grows from hight address towards low address

    ax = 0; // initialize the value of ax to 0

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
