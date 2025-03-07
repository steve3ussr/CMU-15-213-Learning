# Part  A-B

太简单, 暂时跳过

# Part C

## Cheat Sheet

- `make VERSION=full; ./correctness.pl; cd ../y86-code; make testpsim; ./benchmark.pl`
- `./benchmark.pl`
- `./psim -g sdriver.yo`

## 思路

- [x] 合并指令
- [ ] 减少可能的分支预测错误
- [ ] 减少两个MOV之间的stall
- [ ] 减少其他不必要的bubble
- [x] 循环展开
- [ ] 循环展开: 优化条件分支的顺序

## 减少两个MOV之间的stall

- 当mrmovq在M阶段时, 已经读出了m_valM, 将在下一步中写到M_dstM(实际是r10)中; 
- 指令2在D阶段时, 因为指令1在E阶段, 并且E_dstM==d_srcA, 所以D阶段会stall
- 如果解决了pipeline控制条件的话, 指令2在E阶段时, e_valA直接等于m_valM, 可以实现数据的forward
- 如何解决pipeline控制条件:
  - 指令2在D不stall (应该能排除特殊情况)
  - 指令2在E不bubble



```
mrmovq (%rdi), %r10
rmmovq %r10, (%rsi)
```

