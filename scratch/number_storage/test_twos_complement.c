//
// Created by steve3ussr on 1/16/2025.
//
#include <stdio.h>

int main() {
    int x = 0xFFFFFFFF;
    printf("x = %d\n", x);
    int y = 0x7FFFFFFF;
    printf("y = %d\n", y);
    return 0;
    for (unsigned int i = 10; i>=0; i--) {
        printf("%8x\n", i);
    }

    return 0;
}
