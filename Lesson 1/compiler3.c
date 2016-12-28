#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token;                    // current token
int token_val;                // value of current token (mainly for number)
char *src;          // pointer to source code string;
int poolsize;                 // default size of text/data/stack
char *data;                   // data segment
int *current_id,              // current parsed ID
    *symbols;                 // symbol table
int *idmain;                  // the `main` function

// instructions
enum { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
       OR ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };

// tokens and classes (operators last and in precedence order)
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

// fields of identifier
enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};

// types of variable/function
enum { CHAR, INT, PTR };

void next() {
    char *last_pos;
    int hash;

    while (token = *src) {
        ++src;

        // parse token here
        if (token == '\n') {
            ++line;
        }
        else if (token == '#') {
            // skip macro, because we will not support it
            while (*src != 0 && *src != '\n') {
                src++;
            }
        }
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
        else if (token >= '0' && token <= '9') {
            // parse number, three kinds: dec(123) hex(0x123) oct(017)
            token_val = token - '0';
            if (token_val > 0) {
                // dec, starts with [1-9]
                while (*src >= '0' && *src <= '9') {
                    token_val = token_val*10 + *src++ - '0';
                }
            } else {
                // starts with 0
                if (*src == 'x' || *src == 'X') {
                    //hex
                    token = *++src;
                    while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F')) {
                        token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
                        token = *++src;
                    }
                } else {
                    // oct
                    while (*src >= '0' && *src <= '7') {
                        token_val = token_val*8 + *src++ - '0';
                    }
                }
            }

            token = Num;
            return;
        }
        else if (token == '"' || token == '\'') {
            // parse string literal, currently, the only supported escape
            // character is '\n', store the string literal into data.
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
        else if (token == '/') {
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
        else if (token == '=') {
            // parse '==' and '='
            if (*src == '=') {
                src ++;
                token = Eq;
            } else {
                token = Assign;
            }
            return;
        }
        else if (token == '+') {
            // parse '+' and '++'
            if (*src == '+') {
                src ++;
                token = Inc;
            } else {
                token = Add;
            }
            return;
        }
        else if (token == '-') {
            // parse '-' and '--'
            if (*src == '-') {
                src ++;
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
                src ++;
                token = Le;
            } else if (*src == '<') {
                src ++;
                token = Shl;
            } else {
                token = Lt;
            }
            return;
        }
        else if (token == '>') {
            // parse '>=', '>>' or '>'
            if (*src == '=') {
                src ++;
                token = Ge;
            } else if (*src == '>') {
                src ++;
                token = Shr;
            } else {
                token = Gt;
            }
            return;
        }
        else if (token == '|') {
            // parse '|' or '||'
            if (*src == '|') {
                src ++;
                token = Lor;
            } else {
                token = Or;
            }
            return;
        }
        else if (token == '&') {
            // parse '&' and '&&'
            if (*src == '&') {
                src ++;
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
    }
    return;
}

void program() {
    next(); // get next token
    while (token > 0) {
        printf("token is: %3d('%c')\n", token, token);
        next();
    }
}

int main(int argc, char **argv) {

    int i, fd;

    argv++;

    poolsize = 256 * 1024; // arbitrary size
    line = 1;

    if (!(data = malloc(poolsize))) {
        printf("could not malloc(%d) for data area\n", poolsize);
        return -1;
    }

    if (!(symbols = malloc(poolsize))) {
        printf("could not malloc(%d) for symbol table\n", poolsize);
        return -1;
    }

    memset(data, 0, poolsize);
    memset(symbols, 0, poolsize);

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
