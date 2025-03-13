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

## Cheat

```
gcc -O2 ./cachelab.c ./csim.c -o ./csim

./csim -v -s 1 -E 1 -b 1 -t traces/yi2.trace;
./csim-ref -v -s 1 -E 1 -b 1 -t traces/yi2.trace;
./csim -v -s 4 -E 2 -b 4 -t traces/yi.trace;
./csim-ref -v -s 4 -E 2 -b 4 -t traces/yi.trace;
./csim -v -s 2 -E 1 -b 4 -t traces/dave.trace;
./csim-ref -v -s 2 -E 1 -b 4 -t traces/dave.trace;
./csim -v -s 5 -E 1 -b 5 -t traces/long.trace;
./csim-ref -v -s 5 -E 1 -b 5 -t traces/long.trace;


./csim -v -s 2 -E 1 -b 3 -t traces/trans.trace;
./csim-ref -v -s 2 -E 1 -b 3 -t traces/trans.trace;

./csim -v -s 2 -E 2 -b 3 -t traces/trans.trace;
./csim-ref -v -s 2 -E 2 -b 3 -t traces/trans.trace; 

./csim -v -s 2 -E 4 -b 3 -t traces/trans.trace;
./csim-ref -v -s 2 -E 4 -b 3 -t traces/trans.trace;

./csim -v -s 5 -E 1 -b 5 -t traces/trans.trace;
./csim-ref -v -s 5 -E 1 -b 5 -t traces/trans.trace;
```



## Ref

- `csim-ref -v -s ? -E ? -b ? -t <file>`
- 64 bit mem addr: t-s-b

## Rules

- 只关注Data Cache, 忽略Instruction Cache
- 

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
- S是一个对象, 通过s来区分
- S里包含若干个line, S对象里有LRU方法
- 每个line中有tag和block
- 
  1. set内的LRU使用双向链表实现，每个节点代表一个line
  2. load时，遍历set内的node，如果
     1. hit：取出节点，放到末尾，HIT
     2. miss，长度未达上限：加到末尾，MISS
     3. miss，长度已达上限：pop head，新的内容加到末尾，MISS EVICT
  3. store时一样
  4. 数据结构：
     1. LineNode代表一个line 节点，由v，tag组成
     2. Set：len，dumb head，dumb tail
     3. cache：Set array
  5. 细节：
     1. 一次访问应该不止访问一个block，如果越界需要做处理
     2. 需要free内存，因为new需要malloc



