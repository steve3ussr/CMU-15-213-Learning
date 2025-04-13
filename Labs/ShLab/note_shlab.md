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

`SIGCHLD`当子进程结束/被stop时, 会给父进程发送这个信号

`pause()`时收到一个信号就会恢复



## Helper Routings

- job_t 定义了pid, jid, state (UNDEF, BG, FG, ST), 记录cmdline
- nextjid, 从1递增
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

## Self-Finished Routings

### eval

- 创建子进程前阻塞CHLD, 父进程中在addjob后恢复, 子进程中立即恢复
- 修改全局变量前阻塞所有信号
- 创建子进程后set group id
- waitfg中前不解除对CHLD的阻塞

### builtin cmd

很简单, 没什么可说的

### do_bgfg

- 解析命令行, 考虑以下情况: 没输入id, 输入的不是id, 输入的id不存在
- 不管bgfg, 都发送SIGCONT
- 如果是fg就等待完成



### waitfg

> explicitly等待一个信号

- 加上对SIGCHLD的block也没问题, 可以保证while循环前这个信号是被阻塞的; 在调用前 (在eval中调用)阻塞SIGCHLD
- `sigsuspend`暂时解锁所有信号 (因为前台进程可能收到各种信号), pause, 因为一个信号而恢复, 恢复原有的block vector
- **这个函数有缺陷, 没有检测是否同时存在多个前台进程; 不过按照逻辑, 同一时刻也不会出现多个前台进程**

### sigchld_hdl

- 回收子进程
- waitpid中加入WNOHANG: 不加的话就会一直阻塞父进程, 直至有子进程终止, 这样很浪费时间; 
- waitpid中加入WUNTRACED: 
  - 除了TERM, 还能看到STOPPED; 
  - 一个子进程可以给他自己发送SIGSTOP/SIGTSTP, shell应该发现这点, 并且在父进程的jobs中设置状态为ST; 如果一个前台进程给自己发送了STOP, 但是没有被父进程捕捉, 就会一直卡在waitfg; 所以必须要加这个option
  - 如果一个子进程被stopped了, 也会给父进程发SIGCHLD
- deletejob前必须阻塞所有的信号, 结束后恢复



### sigtstp_hdl, sigint_hdl

- 只发送给前台进程
- 没必要阻塞信号, waitfg中阻塞信号是因为pause()可能一直阻塞, 而kill一个不存在的进程只会返回错误码
