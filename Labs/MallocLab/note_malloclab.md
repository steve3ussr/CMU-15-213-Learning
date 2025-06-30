# Malloc Lab

## Target

- 只修改mm.c, 实现`malloc, free, realloc`
- eval by `mdriver.c`

## Working On



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



