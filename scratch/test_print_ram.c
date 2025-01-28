//
// Created by steve3ussr on 1/17/2025.
//
#include <stdio.h>
#define uint unsigned int
typedef unsigned char *pointer;  // cast a pointer to unsigned char *
/*
 * 给指针+1就会偏移对应类型的量，
 * uchar比较好的一点是只占1 byte，
 * 所以可以用来看每个byte的实际内容
 */

void show_bytes (pointer start, uint len) {
    for (uint i=0; i<len; i++) {
        printf("%p    0x%.2x\n", start+i, start[i]);
    }
    printf("\n");
}

/*
 * sizeof返回的是uint
 */
int main() {
    float a = 5.75;
    show_bytes((pointer) &a, sizeof(a));
    return 0;
}

