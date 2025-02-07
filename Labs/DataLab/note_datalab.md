# TODO



# Lab 1: DataLab

## Instructions

- 只需要修改`bits.c`, 其中包括13个puzzle的框架
- 不允许使用循环, 条件等, 只能用指定的运算符, 更具体的描述见`bits.c`
- 不允许使用超过8bits的常量
- 

## Puzzle 1: XOR

只允许使用`~, &`

```
a^b = (~a)&b | a&(~b)
~ a|b = (~a)&(~b)
```



## Puzzle 2: tmin

生成1，左移

## Puzzle 3: isTmax

```
Tmax+1 = Tmin
~Tmax  = Tmin
(~x)^x = 0
```

利用以上的性质，可判断x为Tmax或者-1

通过`!(x+1)`可判断具体是哪个数

## Puzzle 4: allOddBits

1. 利用`0xAA`和左移生成`0xAAAAAAAA`
2. `(x & 0xAA) ^ 0xAA=0`，说明x的奇数位全为1，否则说明x不满足条件

## Puzzle 5: Negate

`(~x)+1`

## Puzzle 6: isAsciiDigit

```
x - 0x30 >= 0, a=(x-0x30)>>31 全0, !a为1
x - 0x3a <  0, b=(x-0x3a)>>31 全1, !(b+1)为1
第二条不能用0x39减, 因为0x39-0x39=0, 0>>31全0, 和第一种情况一样
```

## Puzzle 7: conditional

```
寻找f(x, y), 使得x!=0时值为y, x=0时值为0; 
可以猜测, 很容易就有g(x, z), 使得x!=0时值为0, x=0时值为z;
这样就能返回 f(x, y)+g(x, z), 当x!=0时值为y, x=0时值为z

Sf(x) = !(!x), 当x!=0时值为1, x=0时值为0
Sg(x) = !(x),  当x!=0时值为0, x=0时值为1

Rf(x) = (~Sf(x))+1, 当x!=0时为全1 (0xff), x=0时值为全0
Rg(x) = (~Sf(x))+1, 当x!=0时为全0, x=0时值为全1 (0xff)

f(x, y) = Rf(x) & y
g(x, z) = Rg(x) & z
```

## Puzzle 8: isLessEqual

```
f(x) = (x>>31)+1
if x>=0: f(x)=1
if x <0: f(x)=0 
```

考虑特殊情况：

1. x=Tmin，此时必然会溢出，且结果一定为1
2. x==y，结果可能为0或者溢出，且结果一定为1

令x1=~x+1

1. 如果x1>0, y>0, 但s<=0时发生正溢出
2. 如果x1<0, y<0, 但s>=0时发生负溢出

s=x1+y

1. 如果正溢出，返回1
2. 如果负溢出，返回0
3. 其他情况下，返回s>=0





## Puzzle 9: logicalNeg

```
f(x) = x ^ (~x+1)
g(x) = (f(x)>>31) & 1, when x==0/Tmin, f(x)=0

h(x) = (x>>31) & 1,    when x>=0,      g(x)=0

k(x) = (1-g(x)) & (1-h(x)), when x==0, k(x)=1
```

## Puzzle 10: howManyBits

问题基本等效为从头开始，寻找第一个发生变化的位置。所以可以：

1. 记住第一个bit
2. 设置一个flag，为了方便，让flag初始为0
3. res初始值为32
4. 从第二个bit开始遍历，直到最后一个bit，如果某个bit和第一个bit不一样，就永久地改变flag，可以用 | 运算
5. `x>>(i-1) & 0x1`可提取第i位bit，计算``xi ^ x32`
6. 每次比较后，res -= 1-flag，如果flag=0说明一样的，res需要-1；如果flag=1说明res不需要更改了；
7. 解循环，最终优化后可得结果

---

以上的方法操作数会超，新方法：

1. `y = x^(x>>31)`，将变成从0开始，比如`0011010`，这个数字需要7位才能表达
2. 循环的话，操作数太多太多，考虑二分法
3. 如果一个数字可以用k+1个bit表达，那这个数字在`[2^(k-1), 2^k)`之间，问题转化为求log2y
4. 用二分法，将y分为前后两部分
   1. 如果前面的全是0，结果就是求后一半对应的k
   2. 如果前面的值有，结果就变成一半的位数+前一半对应的k
5. 例子1：00110101，k=6=4+func(0011) = 4+func(11) = 4+1+func(1) = 6
6. 例子2：00000111，k=3 = func(0111) = 2+func(01) = 2+func(1) = 3
7. 尽管二分法行数可能更多，但操作数小得多

## Puzzle 11: floatScale2

从输入值中提取:

- M: 32位, 前9位为0, 后23位为输入值
- S: 32位, 第一位为输入值, 后31位为0
- E: 32位, 后8位为实际的E field

### case1: E field all 1

此时为NaN, 直接返回输入值

### case2: E field 非全0

让E field+1, 重新组装

### case3: E filed 全0

M左移1, 前面+7个0, 加sign bit

## Puzzle 12: floatFloat2Int

从输入值中提取:

- M: 32位, 前9位为0, 后23位为输入值
- S: 32位, 第一位为输入值, 后31位为0
- E: 32位, 后8位为实际的E field

### case1: E field all 1

此时为NaN, 直接返回0x80000000

### case2: E field 非全0

计算exponent，为E-127

M左移exponent-23

- 如果左移超过31位，返回0x80000000；
- 如果右移超过31位，返回0；

加sign bit



### case3: E filed 全0

肯定是0



## Puzzle 13: floatPower2

- x < -126，返回0
- x > 127，返回+inf
- 返回x+127左移23
