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




