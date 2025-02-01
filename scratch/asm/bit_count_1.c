#include <stdio.h>

void do_while (int x) {
    int res=0;
    do {
        res += (x>>31) & 1;
        x <<= 1;
    } while (x);
    printf("do_while: There are \t%d 1-bit in x\n", res);
}
void do_while_goto(int x) {
    int res=0;
    exec:
    res += (x>>31) & 1;
    x <<= 1;

    if (x) goto exec;
    printf("do_while_goto: The're \t%d 1-bit in x\n", res);
}

void _while(int x) {
    int res=0;
    while (x) {
        res += (x >>31) & 1;
        x <<= 1;
    }
    printf("while: There are \t%d 1-bit in x\n", res);
}

void while_goto(int x) {
    int res=0;
    goto check;
    loop:
        res += (x>>31) & 1;
        x <<= 1;
    check:
        if (x) goto loop;
    printf("while_goto: There are \t%d 1-bit in x\n", res);
}

void _for (int x) {
    int res = 0;
    for (int i=0; i<32; i++) {
        int xi = (x>>i) & 1;
        res += xi;
    }
    printf("_for: There are \t%d 1-bit in x\n", res);
}

void for_goto (int x) {
    int res = 0;
    int i = 0;
    int xi = 0;
    goto check;
loop:
    xi = (x>>i) & 1;
    res += xi;
    i++;
check:
    if (i < 32) goto loop;

    printf("for_goto: There are \t%d 1-bit in x\n", res);
}

int main () {
    int x = 0xf0000001;
    do_while(x);
    do_while_goto(x);
    _while(x);
    while_goto(x);
    _for(x);
    for_goto(x);
    return 0;
}
