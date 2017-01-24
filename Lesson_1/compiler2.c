#include <stdio.h>
#include <fcntl.h> // for open()
#include <unistd.h> // for read() and close()
#include <stdlib.h> // for malloc()

int token; // current token
int token_val;                // value of current token (mainly for number)

char *src; // pointer to source code string

int poolsize; // the size of source area

// tokens and classes (operators last and in precedence order)
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge,
  Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

void next() {
    while ((token = *src) > 0) {
        src++;

        // parse token here
        if (token == ' ') {
            // do nothing
        } else if (token >= '0' && token <= '9') {
            token_val = token - '0';
            // if (token_val > 0) {
                while (*src >= '0' && *src <= '9') {
                    token_val = token_val * 10 + (*src - '0');
                    src++;
                }
            // }

            token = Num;
            return;
        } else if (token == '+') {
            // now we only parse '+' and ignore '++'
            token = Add;
            return;
        }
    }
    return;
}

void program() {
    next(); // parse out one token and store its type into variable token
    while (token > 0) {
        printf("token is: %d\n", token);
        next(); // get next token
    }
}

int main(int argc, char *argv[]) {

  argv++;

  poolsize = 256 * 1024; // assume the default size of source area is 256K

  // open the source file and store the file descriptor into variable fd
  int fd;
  if ((fd = open(*argv, 0)) < 0) {
    printf("could not open source file: %s\n", *argv);
    return -1;
  }

  // allocate memory for source area
  if (!(src = malloc(poolsize))) {
    printf("could not malloc memory (%d bytes) for source area\n", poolsize);
    return -1;
  }

  // read the source file to source area
  // the last byte is reserved for EOF flag
  int i;
  if ((i = read(fd, src, poolsize-1)) <= 0) {
    printf("could not read data from file: read() returned %d\n", i);
    return -1;
  }

  close(fd); // close file

  src[i] = -1; // add EOF flag

  program(); // start paring
}
