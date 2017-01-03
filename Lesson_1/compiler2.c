#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token;                    // current token
int token_val;                // value of current token (mainly for number)
char *src;                    // pointer to source code string;
int poolsize;                 // default size of text/data/stack

// tokens and classes (operators last and in precedence order)
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

void next() {
    while (token = *src) {
        ++src;

        // parse token here
        if (token == ' ') {
            // do nothing
        } else if (token >= '0' && token <= '9') {
            token_val = token - '0';
            // if (token_val > 0) {
                while (*src >= '0' && *src <= '9') {
                    token_val = token_val*10 + (*src++ - '0');
                }
            // }

            token = Num;
            return;
        } else if (token == '+') {
            // only parse '+', ignore '++'
            token = Add;
            return;
        }
    }
    return;
}

void program() {
    next(); // get next token
    while (token > 0) {
        printf("token is: %d\n", token);
        next();
    }
}

int main(int argc, char **argv) {

    int i, fd;

    argv++;

    poolsize = 256 * 1024; // arbitrary size

    if ((fd = open(*argv, 0)) < 0) {
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    // read the source file
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
    src[i] = 0; // add EOF character
    close(fd);

    program();

    return 0;
}
