/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  int a = (~x)&y;
  int b = x&(~y);
  int res_inverted = (~a)&(~b);
  return ~res_inverted;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {

  return 1<<31;

}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  int possible = !((x+1) ^ (~x));
  int not_neg1 = !(!(x+1));  // !(!( x+1 ))
  return possible & not_neg1;
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  int base_a = 0xAA;
  int full_a = base_a + (base_a<<8) + (base_a<<16) + (base_a<<24);
  int res = !(full_a ^ (full_a & x));
  return res;
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return (~x)+1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  int a = (x + (~0x30)+1) >> 31;
  int b = ((x + (~0x3a)+1) >> 31) + 1;
  return (!a) & (!b);
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  int mask_y = 1 + ~(!(!x));
  int mask_z = 1 + ~(!x);
  return (mask_y&y) + (mask_z&z);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
    int x1 = ~x+1;
    int delta = y+x1;
    int flag_x_eq_Tmin = !(x^(0x1<<31));

    int flag_x_ge0 = !((x1>>31)&1);
    int flag_y_ge0 = !((y>>31)&1);
    int flag_d_l0 = (delta>>31)&1;
    // x>00 && y>=0 && s<0
    int of_pos = flag_x_ge0 & flag_y_ge0 & flag_d_l0;
    // x<0 && y<0 && s>=0
    int not_of_neg = flag_x_ge0 | flag_y_ge0 | flag_d_l0;
    // res >= 0
    int res = !flag_d_l0;

    // if (of_pos): res=1;
    res = res | of_pos;
    // if (of_neg): res=0;
    res = res & not_of_neg;

    // if x==Tmin, return 1;
    return flag_x_eq_Tmin | res;
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
    int fx = x ^ (~x+1);
    int gx = (fx>>31) & 1;
    int hx = (x>>31) & 1;

    return (2+~gx) & (2+~hx);
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
    int y = x ^ (x>>31);
    int res = 63;
    res = res +~ !(y>>30);
    res = res +~ !(y>>29);
    res = res +~ !(y>>28);
    res = res +~ !(y>>27);
    res = res +~ !(y>>26);
    res = res +~ !(y>>25);
    res = res +~ !(y>>24);
    res = res +~ !(y>>23);
    res = res +~ !(y>>22);
    res = res +~ !(y>>21);
    res = res +~ !(y>>20);
    res = res +~ !(y>>19);
    res = res +~ !(y>>18);
    res = res +~ !(y>>17);
    res = res +~ !(y>>16);
    res = res +~ !(y>>15);
    res = res +~ !(y>>14);
    res = res +~ !(y>>13);
    res = res +~ !(y>>12);
    res = res +~ !(y>>11);
    res = res +~ !(y>>10);
    res = res +~ !(y>>9);
    res = res +~ !(y>>8);
    res = res +~ !(y>>7);
    res = res +~ !(y>>6);
    res = res +~ !(y>>5);
    res = res +~ !(y>>4);
    res = res +~ !(y>>3);
    res = res +~ !(y>>2);
    res = res +~ !(y>>1);
    res = res +~ !(y);
    return res;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
    unsigned int m = uf & ~((0xff<<24) + (0x1<<23));
    unsigned int s = uf & (0x1<<31);
    unsigned int e = (uf>>23) & 0xff;

    if (!(e^0xff)) return uf;
    if (!(e)) return (m<<1) + s;

    e = e+1;
    return s + (e<<23) + m;
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */

int floatFloat2Int(unsigned uf) {
    int m = uf & ~((0xff<<24) + (0x1<<23));
    unsigned int s = uf & (0x1<<31);
    unsigned int e = (uf>>23) & 0xff;
    // int expo = e-127; //e[1, 254], expo[-126, 127]
    // int sh = expo-23;  // sh>=0, shl; sh<0, shr
    int sh_num = e+1+(~150);
    m = m+(0x1<<23);

    if (!(e^0xff)) return 0x80000000;
    if (!(e)) return 0;

    if (!(sh_num>>31)) {   // if sh>=0
        if (sh_num>>5) return 0x80000000;
        m = m<<sh_num;
    }
    else {
        int sh_neg = ~sh_num+1;
        if (sh_neg>>5) return 0;
        m = m>>(sh_neg);
    }
    if (s) {return ~m+1;}
    else {return m;}
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
    if (!(x>>31)) {// x>=0
        // if x>127
        if (x>>7) return (0x7f<<24) + (0x80<<16);
        x = (x&0x7f) + 127;
    }
    else {
        x = x+126;
        // x+126<0, x<-126
        if (x>>31) return 0;
        x = x+1;
    }
    return (x&0xff)<<23;
}
