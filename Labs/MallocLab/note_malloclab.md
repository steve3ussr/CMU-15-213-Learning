# Malloc Lab

## Target

- 只修改mm.c, 实现`malloc, free, realloc`
- eval by `mdriver.c`

## Update

### v 1.0

使用隐式链表, 全部为双重边界标记, first-fit放置策略, 尽可能的分割策略, 尽可能的合并策略

### v 1.1

优化了部分宏

### v 1.2

优化了remalloc, 如果当前块的next是空闲的, 并且可以合并, 那就不需要free并且malloc

```
Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   66%      12  0.000000    120000
 1       yes   89%      12  0.000000    120000
 2       yes   99%    5694  0.004379      1300
 3       yes   99%    5848  0.004375      1337
 4       yes   99%    6648  0.006359      1046
 5       yes  100%    5380  0.004282      1257
 6       yes   66%   14400  0.000081    178439
 7       yes   91%    4800  0.005406       888
 8       yes   92%    4800  0.005084       944
 9       yes   55%   12000  0.102540       117
10       yes   51%   24000  0.144661       166
11       yes   80%   14401  0.000113    127330
12       yes   46%   14401  0.000069    207807
Total          80%  112396  0.277349   405

Perf index = 48 (util) + 27 (thru) = 75/100
```

此时coalescing, binary, binary2, realloc2的利用率较差, 除了short系列, realloc系列, coalescing之外性能都比较差

### v 2.0

>  使用显式链表



## Working on Version 2

### macro, global vars

- [x] ALIGN by 4 Bytes (WORD)
- [x] macro FD=bp, BK=bp+4
- [x] macro PREV_BLKP = BK content, content
- [x] list_head, list_tail point to prologue, epilogue

### mm init

- [x] extend 8 WORDS manually, include 1 blank, 4 prologue, 3 epilogue
- [x] connect prologue and epilogue
- [x] extend chunksize

### extend

> join bp to the list immediately. this may induce low pert if fill the block right after extend, then join list is unnecessary

- [x] ALIGN by even WORDS
- [x] preserve var tail_prev
- [x] bp = sbrk. fill HDR FIR
- [x] create a new tail:
  - [x] list_tail=NEXT_BLKP(bp)
  - [x] FD(tail}=NULL
  - [x] BK(tail)=bp
  - [x] HDRP(tail)=0/1
- [x] tail_prev
  - [x] FD(tail_prev)=bp
- [x] bp
  - [x] FD(bp)=tail
  - [x] BK(bp)=tail_prev
- [x] coalesce(bp)

### coalesce

> if prev/next is free and coherent, merge

### mm_malloc

save footer space; if request 19 bytes, round up to 20; if request 21 bytes, round up to 28; `payload=((x+3)/8)*8+4`, `block size=((x+11)/8)*8`

- [x] find-fit (return bp)
  - [x] return place
- [x] extend (return bp)
  - [x] return place

### find_fit

> search from head, find dirst fit

### void * place

- [x] preserve vars: prev, next
- [x] if no enough space to split: connect prev and next, mark bp as allocated
- [x] if can be splitted:
  - [x] calc split size, resize bp HDR/FTR size
  - [x] allocate data to higher RAM (don't need to manage FD/BK)
  - [x] coalesce split block

### free

- [x] mark bp as free
- [x] create FTR (size, 0)
- [x] located bp, link with prev and next
- [x] coalesce
#### loc method: LIFO
- [ ] preserve head_next
- [ ] modify FD/BK of head, head_next, bp

#### loc method: Addr-Ordered

- [x] traverse from head, until find a free block (curr) with addr>bp
- [x] find prev of curr
- [x] modify FD/BK of prev. bp, curr
- [x] coalesce(bp)

### mm_realloc

> equal size, 0 size, NULL bp: do as it is

#### request smaller size

- [x] cannot split: do nothing
- [ ] split:
  - [ ] search for free block prev, next
  - [ ] modify bp HDR
  - [ ] modify bp FTR
  - [ ] modify split HDR
  - [ ] modify split FTR
  - [ ] modi+y FD/BK of prev, split, next

#### request greater size (aligned requested size > current block size)

- [x] try to find a coherent next free block
- [x] if there is a coherent next free block, and it has sufficient size (curr)
  - [x] structure: prev, bp-curr, next
  - [x] can split
    - [x] mod bp HDR
    - [x] locate new_curr
    - [x] mod new_curr HDR/FTR
    - [x] link prev-new_curr-next
    - [x] coalesce new_curr
  - [x] can not split
    - [x] connect prev-next
    - [x] mod bp HDR
- [x] else: malloc, memcpy, free





## Working On Version 1



### memlib.c

学生将使用这个库里的函数, 而不是系统调用

- mem_init: 申请20MB的堆内存 (根据config.h)
- mem_deinit: 释放这些内存
- mem_reset_brk: 重置brk指针位置为堆的初始位置
- mem_sbrk: 增长一些内存, 前提是不超过20MB的限制
- mem_heap_lo, mem_heap_hi: 实际使用的堆内存上下限
- mem_heapsize: 实际使用的堆内存大小
- mem_pagesize: 返回系统的pagesize



### macros in mm.c

- [x] GET PUT是否一定要每次操作4B啊

- Word Size, Double Word, Chunk (4K)
- **PACK**, 将29 bits的size (后3 bit为隐式的8B对齐, 全0意味着必须是8的倍数) 和3bits的 alloc status打包
- GET PUT, 强转指针为WordSize指针(4 Bytes), 一次操作4 Bytes
- GET_SIZE, mask掉后3 bits; GET_ALLOC, 取最后一位
- HDRP, 给block pointer强转 1Byte的指针, -4, 相当于-32, 指向block header
- FTRP: header + get_size - 2B; get_size获得的值可能是任意字节, 所以强转 char*
- NEXT_BLKP: `(char*)(bp) + GETSIZE((char *)bp - WSIZE) `
- PREV_BLKP: `(char*)(bp) - GETSIZE(char* bp - DSIZE)`

### mm.c

#### opt

- [ ] explicit free only list
- [ ] if block is allocated, donot write footer
- [ ] if prev block is allocated, current block bit is 1
- [x] realloc: 如果需要扩容, 并且next是空的, 并且curr+next空间足够, 就不用find first fit

#### mm_init

- 返回初始化状态
- 首先申请4 words的大小, 用于储存prologue and epilogue block, 并且赋值
- heap_listp指向合适的位置
- 申请 1 chunk的内存

#### extend_heap

- 理论上, 当前brk的值就是应该返回的bp
- 接收一个words, 重整为8B对齐的bytes, 然后再mem_sbrk
- 改变bp对应的header和footer的大小和状态
- extend后, epilogue block被挤占了, 需要重新赋值; next header处应该赋值
- coalesce

#### coalesce

- 根据前后block的状态, 修改对应的大小
- 注意: 如果和prev block合并, 需要修改bp的值

#### mm_free

- 修改header, footer
- coalesce

#### mm_malloc

- 先做内存对齐
- 寻找合适的block
- 如果没有合适的block, 就需要extend
- 找到一个位置, 并且place

#### find_fit

- 从头开始, 寻找一个free并且空间足够的block
- 如果指向了epilogue, 就return NULL

#### place

- 放置
- 如果空闲长度 > 8, 就split

#### mm_realloc

- requested size <= curr size: mod in-place
  - eq: do nothing
  - diff == 8 Bytes
    - next block is free: coalesce
    - otherwise: fragment, ignore it (题目要求block不能缩小, 所以这里什么都不许做)
  - diff >= 16 Bytes: split, coalesce 
- requested size > curr size: malloc，memcpy，free





- `mm_malloc(size_t size)`返回一个指针 (指向payload), malloc不应超出heap, 也不应该和已经分配的blocks overlap; **8 bytes对齐 (为了提高内存访问效率, 64位机器内存都是8B对齐的)**
- `mm_free(void *ptr)`释放, 前提是这个指针必须是通过malloc/realloc获取的
- `mm_realloc(void* ptr, size_t size)`比较灵活
  - ptr==NULL, 等于malloc
  - size==0, 等于free
  - 正常情况下, 这个函数将改变ptr指向的block size; 返回值可能是ptr, 也可能是另外的地方; 这个函数不改变内容
- heap 连续性检查 (`mm_check()`), 仅在debug时使用
  - 是否有没合并的free blocks
  - free list中是否每个block都被标记为free
  - 是不是每个free block都在list中
  - free list 中的是否真的free
  - 是否有allocated blocks overlap
  - heap block是否指向有效的heap address

- 





## Hint

- `man malloc`
- 不允许自定义全局数据结构变量, 例如array; 但是可以自定义标量
- `mdriver -v -V`用来打印额外的调试信息
- 建议用`gcc -g`
- **仔细看书**
- 对指针的操作建议用宏
- 一个trace一个trace地来
- profiler: 优化性能



## debug commands

compile: 

```
clear; gcc -Wall -Og -m32 -DDEBUG  -c -o mm.o mm.c; gcc -Wall -Og -m32 -o mdriver mdriver.o mm.o memlib.o fsecs.o fcyc.o clock.o ftimer.o -DDEBUG

clear; make clean; make;
```

objdump:

```
```

final exec

```
./mdriver -f ./traces/short1.rep -V
```



gdb: 

```
gdb ./mdriver

(gdb) run -f ./traces/short1.rep -V


```



0x56557940  coalesce

0x56557951 in coalesce ()

```
Dump of assembler code for function coalesce:
   0x56557940 <+0>:     push   %ebp
   0x56557941 <+1>:     mov    %eax,%ecx
   0x56557943 <+3>:     push   %edi
   0x56557944 <+4>:     push   %esi
   0x56557945 <+5>:     push   %ebx
   0x56557946 <+6>:     mov    -0x8(%eax),%edx
   0x56557949 <+9>:     and    $0xfffffff8,%edx
   0x5655794c <+12>:    sub    %edx,%ecx
   0x5655794e <+14>:    mov    -0x4(%eax),%edx
   0x56557951 <+17>:    mov    -0x4(%ecx),%ebp
   0x56557954 <+20>:    lea    -0x4(%ecx),%ebx
   0x56557957 <+23>:    and    $0xfffffff8,%edx
   0x5655795a <+26>:    mov    -0x4(%eax,%edx,1),%ecx
   0x5655795e <+30>:    mov    %ebp,%esi
   0x56557960 <+32>:    and    %ecx,%esi
   0x56557962 <+34>:    and    $0x1,%esi
   0x56557965 <+37>:    jne    0x5655798d <coalesce+77>
   0x56557967 <+39>:    mov    %ebp,%edi
   0x56557969 <+41>:    mov    %ebp,%esi
   0x5655796b <+43>:    not    %edi
   0x5655796d <+45>:    and    $0xfffffff8,%esi
   0x56557970 <+48>:    and    %ecx,%edi
   0x56557972 <+50>:    and    $0x1,%edi
   0x56557975 <+53>:    je     0x56557998 <coalesce+88>
   0x56557977 <+55>:    add    %esi,%edx
   0x56557979 <+57>:    mov    %edx,(%ebx)
   0x5655797b <+59>:    mov    -0x4(%eax),%ecx
   0x5655797e <+62>:    and    $0xfffffff8,%ecx
   0x56557981 <+65>:    mov    %edx,-0x4(%eax,%ecx,1)
   0x56557985 <+69>:    mov    -0x8(%eax),%edx
   0x56557988 <+72>:    and    $0xfffffff8,%edx
--Type <RET> for more, q to quit, c to continue without paging--
   0x5655798b <+75>:    sub    %edx,%eax
   0x5655798d <+77>:    pop    %ebx
   0x5655798e <+78>:    pop    %esi
   0x5655798f <+79>:    pop    %edi
   0x56557990 <+80>:    pop    %ebp
   0x56557991 <+81>:    ret
   0x56557992 <+82>:    lea    0x0(%esi),%esi
   0x56557998 <+88>:    mov    %ecx,%edi
   0x5655799a <+90>:    not    %ecx
   0x5655799c <+92>:    and    %ebp,%ecx
   0x5655799e <+94>:    and    $0xfffffff8,%edi
   0x565579a1 <+97>:    and    $0x1,%ecx
   0x565579a4 <+100>:   je     0x565579b8 <coalesce+120>
   0x565579a6 <+102>:   add    %edi,%edx
   0x565579a8 <+104>:   mov    %edx,-0x4(%eax,%edx,1)
   0x565579ac <+108>:   mov    %edx,-0x4(%eax)
   0x565579af <+111>:   pop    %ebx
   0x565579b0 <+112>:   pop    %esi
   0x565579b1 <+113>:   pop    %edi
   0x565579b2 <+114>:   pop    %ebp
   0x565579b3 <+115>:   ret
   0x565579b4 <+116>:   lea    0x0(%esi,%eiz,1),%esi
   0x565579b8 <+120>:   add    %edi,%esi
   0x565579ba <+122>:   add    %edx,%esi
   0x565579bc <+124>:   mov    %esi,(%ebx)
   0x565579be <+126>:   mov    -0x4(%eax),%edx
   0x565579c1 <+129>:   and    $0xfffffff8,%edx
   0x565579c4 <+132>:   mov    -0x4(%eax,%edx,1),%ecx
   0x565579c8 <+136>:   lea    -0x4(%eax,%edx,1),%edx
   0x565579cc <+140>:   and    $0xfffffff8,%ecx
   0x565579cf <+143>:   mov    %esi,(%edx,%ecx,1)
   0x565579d2 <+146>:   pop    %ebx
--Type <RET> for more, q to quit, c to continue without paging--
   0x565579d3 <+147>:   pop    %esi
   0x565579d4 <+148>:   pop    %edi
   0x565579d5 <+149>:   pop    %ebp
   0x565579d6 <+150>:   ret
End of assembler dump.
```

