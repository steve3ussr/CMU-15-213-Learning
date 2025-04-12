# Shell Lab

## system call

`errno`: 一个全局变量

`getpid, getppid`return pid_t var

`waitpid(pid, *status, options)`: 比较复杂

- pid决定了等待进程的集合是什么
- status是可选的, 返回一个导致子进程退出的状态码
- options有几种, 决定了这个函数在什么情况下返回

`wait(status) == wait(-1, status, 0)`, 代表等待caller的所有子进程中的其中一个结束时就退出

`execve(argv[0], argv[], envp[])`在当前进程中加载并执行, 除非找不到argv[0]否则不会返回

`kill(pid, sig)`发送信号

`setpgid()`设置group id

`signal(signum, handler)`设置反应

`sigprocmask`系列用于显式地设置ctx中的block bit

`SIGCHLD`当子进程结束时, 会给父进程发送这个信号

## Todo

- [ ] builtin cmd中kill如何操作
- [ ] wait_fg中, 对SIGCHLD的处理是否正确? 是否会导致竞争?
- [ ] sigint_hdl发送SIGTERM这对吗? 
- [ ] do_bgfg指的是操作一个进程组还是什么? 如果是进程组的话是否需要对每个pid都处理
- [ ] setpgid是在哪里用上的...我怎么不知道...
- [ ] chld_hdl为什么要block信号？
- [ ] chld_hdl为什么waitpid要用option
- [ ] 



## Arch

- job_t 定义了pid, jid, state (UNDEF, BG, FG, ST), 记录cmdline
- nextjid, 从1递增
- **eval**: 
  - 执行每个cmd
  - 如果是子进程, 就fork并且执行
  - 在父进程中addjobs
  - 如果是builtin, 就直接执行
  - 如果是fg, 就等待fg结束
- **_builtin cmd**: 执行内置命令
- **_do_bgfg**:
  - `bg &5`代表jid, `bg 5`代表pid
  - fg, 发送SIGCONT, 等待进程完成
  - bg: ST -> BG, 发送SIGCONT

- **_waitfg**: 等待前台进程结束
- **sigchld_hdl**: deletejob
- **_sigtstp_hdl**: ctrl+Z, FG -> ST
- **_sigint_hdl**: ctrl+C, 结束fgjob
- parseline:
  - 解析cmdline, 将结果填入argv
  - 如果cmd为空, 返回1
  - 如果bg, 返回1
  - 如果fg, 返回0
- sigquit_hdl
- clearjob: 清空所有jobs
- initjobs: 初始化jobs
- maxjid: 返回jobs中最大的jid
- addjob: 添加一个job; 这个函数会同时增加nextjid, 如果达到上限就会回到1; 成功返回1, 失败返回0
- deletejob: 删除一个job, 如果成功返回1, 失败返回0
- fgpid: 返回前台pid, 如果没有fg就返回0
- getjobpid: 返回NULL或者job对象
- getjobjid: 返回NULL或者job对象
- pid2jid: 找到了就返回jid，否则返回0
- listjobs: show jobs



## Traces

### 01

能编译成功就行, 没有要求

### 02

抄书上的`eval`函数实现, 并且能匹配`builtin_cmd`就行, 重点是匹配`exit`

### 03

要求运行一个外部的fg, 02也可以实现

### 04

要求运行一个外部的bg, 需要做一些改进.

问题在于`main`函数, 检测到EOF就会直接exit; 但我们希望exit前await所有的bg jobs结束. 

`WNOHANG | WUNTRACED`立即返回，如果等待集合中的子进程都没有被停止或终止，则返回值为0; 如果有一个停止或终止，则返回值为该子进程的PID 。可以利用这一点, 在退出前循环检查jobs中是否有剩余的job. 

为了完成这一点, 需要:

1. 在eval中addjobs, 加上state
2. 改变main中EOF之后的行为, 检查是否有剩余的任务, 如果有的话就
   1. wait一轮
   2. 打印出wait一轮中结束的子进程的信息
   3. 等待1秒 (让出控制权)
3. 在addjobs中+job数量, 在del中-job数量
4. **有可能在addjobs之前就已经delete, 所以需要“加锁”, 在addjobs前阻塞SIGCHLD**
5. deletejobs如果成功delete会返回1, 如果没找到对应的jobs信息就会返回0
6. 修改SIGCHLD handler, 如果触发了就会尝试回收尽可能多的子进程, 并且deletejobs







