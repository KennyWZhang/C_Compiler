#include <stdio.h>
#include <fcntl.h> // for open()
#include <unistd.h> // for read() and close()
#include <stdlib.h> // for malloc()

int token; // current token

char *src; // pointer to source code string

int poolsize; // the size of source code area

void next() {
    token = *src; // assign the char which the src pointer points to to variable token
    src++; // move to next char
}

void program() {
    next();                  // get next token
    while (token > 0) {
        printf("token is: %c\n", token);
        next();
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
