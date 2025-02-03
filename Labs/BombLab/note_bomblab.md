# BombLab

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

- [ ] 





- jbe: 无符号的小于等于
- num1必须小于等于0xe, 范围[0, 15]
- 

### func4

- argv: num1, 0, 15
- 
