# Malloc Lab

## Intro

- 只修改mm.c, 实现`malloc, free, realloc`
- eval by `mdriver.c`

## Working On

### Self

- `mm_init`用于初始化 heap, 如果有问题就返回-1, 否则返回0
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

---

### Helper

memlib.c中包含了若干有用的工具

- mem_sbrk: 增长堆, 返回新申请的区域开头的指针, 只接受正整数
- mem_heap_lo: 返回堆的开头指针
- mem_heap_hi: 返回堆的末尾指针
- mem_heapsize: 返回heap size
- mem_pagesize: 返回系统page size





## Hint

- `man malloc`
- 不允许自定义全局数据结构变量, 例如array; 但是可以自定义标量
- `mdriver -v -V`用来打印额外的调试信息
- 建议用`gcc -g`
- **仔细看书**
- 对指针的操作建议用宏
- 一个trace一个trace地来
- profiler: 优化性能



