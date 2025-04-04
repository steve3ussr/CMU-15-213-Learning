# Overview

- 模拟cache的行为

# Trace File

通过`valgrind`这个命令行工具产生的，可以跟踪执行程序时的内存使用。

操作：

- I：指令

- Space+L：data load

- Space+S: data store

- Space+M: data modify (load+store)

  

# Part A

## Target

- 写一个C程序，以trace file作为输入
- 模拟cache的行为
- 返回hit，miss，eviction（替换掉某个line）的次数

## Ref

- `csim-ref -v -s ? -E ? -b ? -t <file>`
- 64 bit mem addr: t-s-b

## Rules

- 只关注Data Cache, 忽略Instruction Cache
- 所有的指令行都是第一个字符为I, 所有的数据行都是第一个字符为空格, 第二个为L/S/M
- 如果是M, 那意味着先L后M
- L和M对cache的影响没有区别, 每次访问几个byte也没区别
- 有些内存不是64 bit int, 而是72 bit, 7开头; 这种内存地址我直接取后面的64位, 没啥问题
- 题目中没有内存访问超过block的情况

## Example

- if s=4, b=4
- `L 10, 1` 会miss，同时S=1的区域tag=0，读取0x0000001<0-f>的16个byte到cache
- `M 20, 1`一开始会miss，所以S=2的区域tag=0，读取0x0000002<0-f>的16个byte；到cache；随后load就能hit了
- `L 22, 1`会hit
- `S 18,1`会hit
- `L 110,1`会miss，S=1的区域的tag将被替换成1，读取0x0000011<0-f>的16个byte；
- `L210, 1`会miss，并且替换
- `M12, 1`会miss，eviction，然后hit

## Method

- t - s - b
- cache是一个array of Set
- Set是一个对象包含若干个line, 通过双向链表实现LRU
- 每个line中有tag, v, 还有前后节点
- access时，遍历set内的node，如果
  - hit：取出节点，放到末尾，HIT
  - miss，长度未达上限：加到末尾,长度+1，MISS
  - miss，长度已达上限：pop head，新的内容加到末尾，MISS EVICT
  - **需要free内存!**




# Part B

## Target

- 优化`trans.c`,提高运行速度 (cache hit rate)
- 写一个新的函数, 对于各种尺寸的矩阵都能有良好的缓存命中率

## Guides

- 必须register, 参考已有代码
- 同一时刻最多只能用12个local int (禁止用bit-tricks, 禁止用long)
- 不能用递归
- 不能修改矩阵A
- 不能写任何的array, 也不能malloc
- 一共只有3种size, 允许根据不同的size选择不同的策略, 做特殊的优化
- size: `32*32, 64*64, 61*67`
- **cache size是(s=5, E=1, b=5)**
- 测试函数会生成trace文件, 可以用Part A的工具debug
- **考虑 conflict miss, 尤其是斜线上的**
- **Blocking据说是个好东西**

## Method

在题目的要求中:

- 每个block的size是32B, (能储存8个int); 
- 一共有32个set
- 对于32X32和64X64两种情况, 在非对角线上的8*8 block不用考虑cache hit的事情, 直接转置就好了, 因为一定会miss
- 对于对角线上的8X8, 需要仔细考虑一下
- 现在问题分解成: 对角线上的8X8如何解决, 以及非对称的如何解决



| -    | -    | -    | -    | -    | -    | -    | -    |
| ---- | ---- | ---- | ---- | ---- | ---- | ---- | ---- |
| 3    | 3    | 3    | 3    | 4    | 4    | 4    | 4    |
| 3    | 3    | 3    | 3    | 4    | 4    | 4    | 4    |
| 2    | 2    | 2    | 2    | 4    | 4    | 4    | 4    |
| 2    | 2    | 2    | 2    | 4    | 4    | 4    | 4    |
| 1    | 1    | 1    | 1    | 6    | 6    | 6    | 6    |
| 1    | 1    | 1    | 1    | 6    | 6    | 6    | 6    |
| 1    | 1    | 1    | 1    | 5    | 5    | 5    | 5    |
| 1    | 1    | 1    | 1    | 5    | 5    | 5    | 5    |

| Number | miss | hit  | evict | description                                                  |
| ------ | ---- | ---- | ----- | ------------------------------------------------------------ |
| 1      | 8    | 24   |       | 读取A的前四行, 写到B的后四行                                 |
|        |      |      |       | 此时A的1234行在缓存中, B的5678行在缓存中                     |
|        |      | 8    |       | 从A中读取`[2][0]`到`[3][3]`的8个元素到临时变量               |
| 2      | 2    | 10   | 2     | 向B中写入`[2][2]-[3][3]`(从临时变量), `[2][0]-[3][1]`(从A转置) |
|        |      | 4    |       | 从A中读取 `[0][0]-[1][1]`的4个元素, 加上未使用的`[2][0]-[3][1]` |
| 3      | 2    | 6    | 2     | 写入B的`[0][0]-[1][3]`, 8个元素                              |
|        |      |      |       | 此时B的8行都在缓存中, A中没有元素在                          |
| 4      | 4    | 28   | 4     | 读取A的5678行, 写入到B的1234行                               |
|        |      |      |       | 此时A的5678在缓存中, B的1234在缓存中                         |
|        |      | 8    |       | 从A中读取`[6][4]-[7][7]`的8个元素到临时变量                  |
| 5      | 2    | 10   | 2     | 写入B的`[6][4]-[7][5]`(转置), `[6][6]-[7][7]`(从临时变量)    |
|        |      | 4    |       | 从A中读取`[4][4]-[5][5]`到临时变量                           |
| 6      | 2    | 6    | 2     | 写入B的`[4][4]-[5][7]`, 8个元素                              |



如果是32X32, 那非对角线上的元素也可以分成8X8的block, 共有12个, 每个block的理论值是miss+eviction 16, hit 112

| case        | hit  | miss    | eviction     |
| ----------- | ---- | ------- | ------------ |
| 8x8 est.    | 108  | 20      | 20-8=12      |
| 8x8 real    | 110  | 23      | 14           |
| 32x32 est.  | 1776 | 272     | 272-32=240   |
| 32x32 real  | 1777 | 276<300 | 244          |
| 64x64 est.  | 7136 | 1056    | 1056-32=1024 |
| 64x64 real  |      | <1300   |              |
| 61x67 est.  |      |         |              |
| 61x67 real. |      |         |              |

尽管如此, 64x64的实际表现并不如预期, 同时8x16的表现也不如预期, 这表明之前的分析有问题. 之前的分析没有考虑s-bits的影响。

从trace文件来看, A矩阵起始地址为`0x10e100`, B矩阵其实地址为`0x14e100`; 每个元素+0x4

如果仔细查看，会发现：

- 对于32x32的矩阵的非对角子矩阵，set是完全不重合的；所以运行效率接近完美
- 对于8x8的矩阵，以及32x32中的对角矩阵，set也是不会重合的
- **但对与64x64就很糟糕了，之前的对角/非对角矩阵转置方式有大量的set重合，所以恐怕需要一些新的转置方式**

64x64矩阵的非对角矩阵实例：

| A    | B    |
| ---- | ---- |
| 0x09 | 0x08 |
| 0x11 | 0x10 |
| 0x19 | 0x18 |
| 0x01 | 0x00 |
| 0x09 | 0x08 |
| 0x11 | 0x10 |
| 0x19 | 0x18 |
| 0x01 | 0x00 |

可见在一个8x8的非对角小矩阵中也存在着set重合，如果按照原先的方法，B中的每个元素写入都是miss的；新的想法是

- 读取A的前四行，写入B的前四行（8 miss+eviciton）
- 读取A的前四行，写入B的后四行（4 miss+eviction）
- 读取A的后四行，写入B的后四行（4 miss+eviction)
- 读取A的后四行，写入B的后四行 (4 miss+eviction)

---

64x64矩阵对角矩阵实例：

| A    |
| ---- |
| 0x08 |
| 0x10 |
| 0x18 |
| 0x00 |
| 0x08 |
| 0x10 |
| 0x18 |
| 0x00 |

在8x8的对角矩阵中也存在重合，按照之前的方法，会有很多miss。新的方法每次都写入B的12/34/56/78行的前4/后4个元素，提高B的cache hit rate。

---

















## Blocking

> 可以提升空间局部性

- 主要思想:将数据分成大的块, 称作blocks
- cache直接读取这个block中所有的数据, 处理完, 然后再处理下一个block

以计算矩阵为例，如果计算C=AB这个N阶矩阵，大概是这样的：

```python
for i in range(N):
    for j in range(N):
        sum = 0
        for k in range(N):
            sum += A[i][k] * B[k][j]
        C[i][j] = sum
            
```

但这可能会导致比较低的cache hit rate。Blocking的意思是，将 $N\times N$ 矩阵分成：
$$
A\times B = 
\begin{bmatrix}
A_{11} & A_{12} & A_{13} & A_{14}\\
A_{21} & A_{22} & A_{23} & A_{24}\\
A_{31} & A_{32} & A_{33} & A_{34}\\
A_{41} & A_{42} & A_{43} & A_{44}\\
\end{bmatrix}
\times
\begin{bmatrix}
\bf{B_{11}} & \bf B_{12} & B_{13} & B_{14}\\
\bf B_{21} & \bf B_{22} & B_{23} & B_{24}\\
B_{31} & B_{32} & B_{33} & B_{34}\\
B_{41} & B_{42} & B_{43} & B_{44}\\
\end{bmatrix}
=
\begin{bmatrix}
C_{11} & C_{12} & C_{13} & C_{14}\\
C_{21} & C_{22} & C_{23} & C_{24}\\
C_{31} & C_{32} & C_{33} & C_{34}\\
C_{41} & C_{42} & C_{43} & C_{44}\\
\end{bmatrix}
$$
对于B，每次只取 $bsize\times bsize$ 大小的sub matrix（如上式中加粗的B元素所示），计算和这个小矩阵有关的C元素。

- 和这个小矩阵有关的C元素，第一个index是1-n，第二个index和B中小矩阵的列一样；
- 假设B矩阵的大小N恰好可以整除bsize，这样小矩阵的最小下标也可以迭代：
- 这样做有一些好处，对A矩阵的访问有良好的空间局部性，B有良好的时间局部性，C也有良好的空间局部性

```python
for k1 in range(0, N, bsize):  # base row index of sub B matrix
	for k2 in range(0, N, bsize):
        for i in range(N):
            for j in range(k2, k2+bsize):
                sum = 0
                for k in range(k1, k1+bsize):
                    sum += A[i][k] * B[k][j]
                C[i][j] += sum
```











