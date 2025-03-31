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

## Target





## Arch

### `parseline()`

- 解析cmdline, 将结果填入argv
- 如果cmd为空, 返回1
- 如果bg, 返回1
- 如果fg, 返回0
