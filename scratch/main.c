#include <stdio.h>
int func(unsigned uf) {
    int m = uf & ~((0xff<<24) + (0x1<<23));
    unsigned int s = uf & (0x1<<31);
    unsigned int e = (uf>>23) & 0xff;

    if (!(e^0xff)) return 0x80000000;
    if (!(e)) return 0;

    int expo = e-127; //e[1, 254], expo[-126, 127]
    m = m+(0x1<<23);
    int sh = expo-23;  // sh>=0, shl; sh<0, shr
    if (!(sh>>31)) {
        // if sh>=0
        if (sh>>5) {return 0x80000000;}
        m = m<<sh;
    }
    else {
        int sh_neg = ~sh+1;
        if (sh_neg>>5) return 0;
        m = m>>(sh_neg);
    }

//    m = m & ((0x7f<<24) + (0xff<<16) + (0xff<<8));
    if (s) {return ~m+1;}
    else {return m;}
}
int main() {
    int res = func(0x00800000);
    printf("answer is: %d. \n", res);
    printf("%x", (-126 & 0x7f));
}
