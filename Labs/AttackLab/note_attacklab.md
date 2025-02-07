# Attack Lab

## Intro

- 通过buffer overflow attack vuls
- ctarget: code-injection
- rtarget: return-oriented-programming
- cookie: identifier
- hex2raw: gen attack strings
- farm.c: gen return-orient attack
- solution不能绕过校验代码片段
- ret to的addr只能是:
  - touch-x
  - code inject
  - gadget
- gadget只能从farm中获得, 从start到end
- target通过getbuf函数从stdin读取一个字符串到buf, size是个常数; 可能会overflow

---

- **如果输入合适的字符串, 让缓冲区溢出, 也许能实现一些特殊的目标**
- target verboses: h, q(dont send grades), i (file)
- exploit string 不能含有\n (0x0a)
- hex2raw中输入类似`04 aa 22 bb`而且是小端形式
- 没有失败惩罚

---

# Code Injection Attacks

该部分调用链为：`launch - test - getbuf`，CI部分的三道题分别将跳转到不同的touch函数上. Stack的情况为：

| Stack Address | Content    | Description                        |
| ------------- | ---------- | ---------------------------------- |
| ?             | launch     |                                    |
| `0x5561dcb0`  | `0x401f24` | generate when `launch` call `test` |
| `0x5561dca8`  | `test`     | `test` sub %rsp by 0x8             |
| `0x5561dca0`  | `0x401976` | `test` call `getbuf`               |
| `0x5561dc78`  | **stdin**  | `getbuf` sub %rsp by 0x28          |

stack位置是连续的, 所以在getbuf中buf overflow会影响caller call时自动存放的指令地址

## Level 1

**目标：**

1. 不用注入代码，而是让`getbuf`结束后不继续运行`test`，而是运行`touch1`

**方法：**

1. stdin前0x28 Bytes随便输入
2. 再输入8 Bytes的指令地址（指向`touch1@0x4017c0`），恰好覆盖掉`0x5561dca0`处的返回地址`0x401976`



## Level 2

**目标：**

1. 在运行完`getbuf`之后，跳转到`touch2`，不返回`test`
2. 跳转到`touch2`前，还需要将`%rdi`设置为cookie中的值
3. 不允许用jmp或者call，只能用ret

**方法：**

1. 写汇编，改变`%rdi`寄存器的值，并且return
2. 用`nop`填充前40 Bytes
3. 41-48Bytes：填充为`getbuf`中栈顶的值，这样`getbuf`运行结束后不会继续运行`test`，而是开始执行注入的代码
4. 注入的代码执行到return后，`%rsp`会自动增加到`0x5561dca8`，因此此处的地址内容需要更改为`touch2`的入口地址。*这里也可以把touch2入口地址放在更高的位置，但前提是在汇编中要改变rsp*

## Level 3

**目标：**

1. 写入一个字符串，内容为cookie，并记录其起始地址（结尾填充\0）
2. 不继续执行test，而是执行touch3
3. hexmatch和strncmp会push，所以要将cookies存在安全的内存位置中

**方法：**

1. 写汇编，改变rdi的值为指定的内存地址
2. 用ret和nop填充前40 Bytes
3. 0x40-0x47Bytes（`0x5561dca0`）：跳转到栈顶`0x5561dc78`
4. 0x48-0x4f （`0x5561dca8`）: 内容为touch3入口地址`0x4018fa`
5. 0x50-0x5a (`0x5561dcb0`): 内容为cookies字符串



# ROP

- 这部分比ctarget要难，因为启用了栈随机化，并且栈内存有不可执行flag





## Level 2

**目标：**

1. 在运行完`getbuf`之后，跳转到`touch2`，不返回`test`
2. 跳转到`touch2`前，还需要将`%rdi`设置为cookie中的值
3. 不允许用jmp或者call，只能用ret
4. 只能使用start-mid区域作为gadget
5. 只能使用movq和popq作为gadget

**Gadget: **

在区间范围内搜索有可能的gadget，实际上只有两条：

- `movq rax,rdi; ret`
-  `popq rax; ret`

**方法：**

思路为先pop到rax，然后再mov到rdi. 由于栈随机化，下面的过程用随机的地址表示。

1. 前40B: 空白
2. 0xA8: popq rax, ret; 当getbuf结束时RIP为该指令，执行时会将0xB0内容赋值给RAX，同时RSP+8（指向0xB8）；ret后RSP为0xC0，rip为0xB8
3. 0xB8内容应为gadget 2，mov；return
4. 最顶端为touch2入口地址

| Addr | Before getbuf ret | ret          | pop     | ret     | mov, ret | Content    |
| ---- | ----------------- | ------------ | ------- | ------- | -------- | ---------- |
| 0xC8 |                   |              |         |         | **rsp**  |            |
| 0xC0 |                   |              |         | **rsp** | **rip**  | touch2     |
| 0xB8 |                   |              | **rsp** | **rip** |          | mov, ret   |
| 0xB0 | test              | **rsp**      |         |         |          | cookie     |
| 0xA8 | next of call      | **rip load** |         |         |          | pop, ret   |
| 0xA0 | getbuf            |              |         |         |          | *anything* |

## Level 3

**目标：**

1. 写入一个字符串，内容为cookie，并记录其起始地址（结尾填充\0）
2. 不继续执行test，而是执行touch3
3. hexmatch和strncmp会push，所以要将cookies存在**安全的内存位置**中
4. 可以使用start-end区域作为gadget
5. 可使用的指令中包括movl，只从起始位置读取4Bytes，而不是8Bytes



**Gadget：**

如果只看pdf中的内容，可使用的gadget如下：

- popq rax：可pop出指定的内容
- popq rsp; and movl eax,edx
- movq %rsp,%rax：唯一可生成指针的方法
- movl ecx,esi
- movl eax,edx
- movl edx,ecx

效果如下所示：

| Addr | Original                        | pop rax; ret | pop rsp; ret | mov; ret |
| ---- | ------------------------------- | ------------ | ------------ | -------- |
| 0xB8 |                                 | rsp          | rsp          |          |
| 0xB0 |                                 | rip          | rip          | rsp      |
| 0xA8 | rsp                             | rax load     |              | rip load |
| 0xA0 | *rip, content here is each col* |              |              |          |

但只有以上的还不够。仔细观察会发现farm中包括了一个lea指令：`lea (%rdi,%rsi,1),%rax`。通过该指令可实现对地址偏移。整体思路如下：

1. mov rsp, rax; mov rax, rdi：将一个栈地址存入rdi
2. pop eax; mov eax edx; mov edx, ecx; mov ecx, esi：将指定内容（offset）pop到esi
3. lea：使rax为地址+offset
4. mov rax rdi：修改rdi
5. call touch3

内存结构如下：

| Addr (hex) |                       |
| ---------- | --------------------- |
| 58         | 00                    |
| 50         | TARGET BYTES          |
| 48         | call touch3           |
| 40         | mov rax edi           |
| 38         | **lea**               |
| 30         | mov ecx esi           |
| 28         | mov edx ecx           |
| 20         | mov eax edx           |
| 18         | OFFSET VALUE          |
| 10         | pop eax               |
| 08         | mov rax, rdi          |
| 00         | mov rsp, rax (rax=08) |

- offset为50-08=48
- target bytes为指定值，见c3

