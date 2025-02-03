# BombLab

## TODO

- [ ] gdb with VSC

## Introduction

- 炸弹有多个阶段, 每个阶段都需要你在stdin输入一个特殊的字符串
- 如果输入正确, 就会进入下一阶段; 否则就会在屏幕上打印boom
- 如果炸弹爆炸, 就会失去一般的分数, 不过还好我这是自学, 不会扣分
- 可以传递一个txt, 这样就不用一直打字

## hint

- 用gdb一步一步地看反汇编代码
- 单步调试
- **打断点**
- 查看寄存器
- 查看内存
- strings bomb查看字符串

## Book

![image-20250202170316250](C:\Users\steve3ussr\AppData\Roaming\Typora\typora-user-images\image-20250202170316250.png)

![image-20250202171751337](C:\Users\steve3ussr\AppData\Roaming\Typora\typora-user-images\image-20250202171751337.png)



## Instructions

### test

- test 负数, 负数, 得到的结果开头一定是1, 所以会给SF置1
- test 0, 0, 得到的结果一定是0, 所以会给ZF置1



## Into the Unknown

- 从main上来看 (反编译和源码), 首先call initialize_bomb, 然后是 phase_1
- init_bomb
  - 把两个值存入了寄存器, 第一个参数 edi=2, 第二个参数esi=0x4012a0
  - call 400b90, signal@plt
- signal@plt
  - jmp到 系统的signal处理函数
  - ==push 0xb到栈上==
  - jmp .plt
- .plt
  - push 602ff0这个值
  - jmp 602ff8

## Phase 1: 400ee0

- 将0x402400存入esi, 这通常表示第二个参数
- 在call phase_1前, 已经将 40149e read_line 结果存入rdi作为第一个参数
- read_line的结果可能是输入字符串的起始地址
- call 401338 strings_not_equal
- test 返回结果rax, rax
- 检查返回结果, 如果ZF=1 (%rax=0), 就会跳过引爆

TODO:

- [x] gdb中打断点, `break *0x400ee9`, 不要进入比较string的函数
- [x] `continue`, 直接运行到断点
- [x] 观察内存中 0x402400的内容, `print (char *) 0x00402400`
- [x] 结果: `Border relations with Canada have never been better.`
- [x] 在call strings_not_equal前打断点, 观察edi的值 (一个指针), 再去看内存地址里是否为输入的值; 
- [x] 在检查结果前打断点, 观察eax的值, `print /x $rax`
- [x] 在进入phase2前打断点, 阻止进入phase2



### strings_not_equal: 401338

> - arg1: char* of stdin
> - arg2: char* of answer, 0x402400
> - return: 两个字符串长度, 内容都一样, 返回0, 否则返回1

- 先后运行了两次40131b string_length, 比较结果

- 如果stdin和answer的长度不一样, 那就return 1, boom

- 有一个while循环, byte-wise读取两个参数的内容, 直到stdin遇到\0

- 如果循环中发现两个bytes不一样, 就返回1

  

 

### string_length: 40131b

- 只用到了rdi, 所以这个函数只有一个arg

- rdi里是一个char array的指针, 内容是个地址; 如果字符串为空, 指针就等于NULL

- 比较rdi是否为0, 是的话就return 0

- 有一个while循环, 确定字符串的长度

- 返回char array的长度 (不包括\0)

  



## Phase 2: 400efc

- [x] 在400f05打断点, 查看rdi, 确认为输入字符串的地址
- [x] 在40148f打断点, 查看eax, 确认为匹配长度（应该为6）
- [x] 在40148f打断点, 确认rdx， rcx， r8, r9, x, x递增的地址中，分别存储了解析后的6个数字， int， 各占4Bytes：但是在read_six_numbers中无法访问，可能是在sscanf中先往寄存器中的地址中存放了6个数字，然后清除了寄存器内容。函数重新回到phase2后，使用`print *(int *) ($rsp + 4*i)`，i=0-5可查看6个数字
- [ ] 

- rsp-28, 紧接着rsi指向栈顶
- rdi在进入phase2前, 被设置为stdin的起始地址
- call read_six_numbers 40145c, rax=6，如果stdin满足6个数字+空白分隔
- num1 必须等于1，否则就会炸
- num2 必须等于2
- 以此类推



### read_six_numbers: 40145c

- rsi为6个%d, 带有空格, 为sscanf的format
- 如果rdi长度<=5, boom; 否则返回6
- 没有改变rbx的值



## Phase 3: 400f43

- [ ] break at 400f60, 查看rsp+偏移量的结果





- 输入必须满足`%d, %d`的形式
- rsp+8的地址里是第一个数字，rsp+16的地址里是第二个数字
- ja：无符号的大于
- 如果num1大于7（无符号），就会爆炸；num1的值为[0, 7]
- 400f75, 可能的值为：0x402470, 78, 80, 88, 90, 98, a0, a8

---
| num1 | jmp | $eax | Expected num2 |
|---|---|---|---|
|0|400f7c| 0xcf | 207 |
|1| 400fb9| 137 | 311 |
|2| 400f83| 2c3 | 707 |
|3| 400f8a| 100 | 256 |
|4|400f91| 185 | 389 |
|5| 400f98| ce | 206 |
|6|400f9f| 2aa | 682 |
|7|400fa6| 147 | 327 |

不知道什么原因，如果phase 3的num2后直接EOF，实际读取的值会少了最后一位，所以只要在num2最后补一个回车就好了。 以上8种都能解开phase3.

## Phase 4: 40100c



- jbe: 无符号的小于等于

- num1必须小于等于0xe, 范围[0, 15]

- func4必须返回0

- num2必须等于0

  

### func4

- init argv: num1, 0, 14
- 如果num1==7就不会多次调用func4，直接返回0
- 函数的作用不太明确，似乎是判断num1是否等于14/2^n

## Phase 5

- [ ] 可能爆炸的点： 长度不等于6，string不是期望值
- [ ] 打断点：2个爆炸点，return，400ebe



- 401096: edx本来是stdin的第一个字符，这个指令只取了低4位，现在edx可能的值是`[0, e]`, 16个选项
- 似乎是用了strcpy，将`4024b0+Offset` 开始的6个字符存入了栈中
- 0x40245e: "flyers"
- 循环中，stdin的最低4位必须满足：`9, f, e, 5, 6, 7`
- 随便搞几个ascii对应的字符就行

---

movzbl只取最低一个byte

`madu / iers / nfot / vbyl`

| RAM Addr | Content (hex) | ASCII CHAR |
| -------- | ------------- | ---------- |
| 4024b0   | 6d            | m          |
| 4024b1   | 61            | a          |
| 4024b2   | 64            | d          |
| 4024b3   | 75            | u          |
| 4024b4   | 69            | i          |
| 4024b5   | 65            | e          |
| 4024b6   | 72            | r          |
| 4024b7   | 73            | s          |
| 4024b8   | 6e            | n          |
| 4024b9   | 66            | f          |
| 4024ba   | 6f            | o          |
| 4024bb   | 74            | t          |
| 4024bc   | 76            | v          |
| 4024bd   | 62            | b          |
| 4024be   | 79            | y          |
| 4024bf   | 6c            | l          |

## Phase 6

- [x] 确定6个数字分别为1-6，但顺序还不知道
- [ ] 在0x4011ab打断点，查看rsp+0x20, 28等



- stdin的格式必须为6个数字，空白符分隔
- call read_6_numbers, 调用结束后使用`print *(int *) ($rsp + 4*i)`，i=0-5可查看6个数字

### loop 1: minus by 7

- 确保6个数字都不重复
- 将6个数字都变成7-x

- num1-6分别在rsp+0+4...+20



### loop 2: cast

- 改变内容：rsp+0x20, 28, 30, 38, 40, 48

- 根据nums数组的值，映射到rsp+20-48上

映射关系为：

| 此时nums数组的值 | rsp+20-48的hex |
| ---------------- | -------------- |
| 1                | 6032d0         |
| 2                | 6032e0         |
| 3                | 6032f0         |
| 4                | 603300         |
| 5                | 603310         |
| 6                | 603320         |



### loop 3: meaningless



### loop4: final

- 不重要的信息：rbp过去是个指针，指向rsp+20, 或者说指向nums[5]
- ebp=5
- 希望这些值按照从大到小排列，倒推stdin

```
4 -> 3 -> 0x6032F0, 924  924 4
6 -> 1 -> 0x6032d0, 332  691 3
5 -> 2 -> 0x6032E0, 168  477 2
1 -> 6 -> 0x603320, 443  443 1
3 -> 4 -> 0x603300, 691  332 6
2 -> 5 -> 0x603310, 477  168 5
```



## Phase secret 

- 当Phase 6拆除完成后，phase_defused将会开始检测第四题的答案输入，期望为`%d %d %s`
- 第四题输入为%d %d DrEvil，即可进入secret phase
- stdin应该是一个十进制数字的字符串
- 该数字不能超过1000
- 进入fun7后，会根据情况返回值，期望fun7返回2
- fun7的逻辑并不复杂，根据情况返回0，`2*fun7(ptr+0x8,v)`, `2*fun7(ptr+0x16, v)`

ptr从初始值开始移动，可能的值为：36，8，50，22，45等。

发现如果v==22，会返回`2*(2*(0)+1)=2`

游戏结束



