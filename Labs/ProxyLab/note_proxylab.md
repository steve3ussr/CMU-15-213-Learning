# 0 Introduction

写一个HTTP代理, 用于缓存web内容, 分成两部分:

1. 设置代理, 接收客户端连接, 解析请求, 转发请求, 读取响应, 在再回复给客户端
2. 并发
3. 加上缓存

# 1 串行代理服务器

- 只处理GET
- 需要判断是否为合法的HTTP请求
- proxy会接受一个`GET http://www.cmu.edu/hub/index.html HTTP/1.1 `请求
  - 解析为主机名
  - 后面的一切
- 发送一个`HTTP/1.0`请求
- HTTP请求每行结尾都是`\r\n`, 结尾是空的`\r\n`
- 参考RFC 1945的稳定性, 除了一条不需要满足: 多行requests不用处理

## 1.2 Request Headers

- host: 根据情况解析
- User-Agent: 必须加上
- Connection: close
- Proxy-Connection: close
- 客户端发送的其他headers: 转发

## 1.3 Port numbers

- 默认80, 除非客户端请求里明确说了

# 2 并发处理请求

# 3 缓存

# 4 其他

## 4.1 稳定性

应当正确处理, 而不是一遇到问题就退出程序, 例如段错误, 内存泄漏, fd泄漏

lab除了评分脚本, 没有单独的测试用例, 必须自己思考哪里可能有问题

## 4.2 powerful tools

- Tiny Web Server
- telnet
- curl
- netcat

# 5 Hints

1. RIO
2. 想一个错误恢复
3. 可以创造新的文件, 但是记得更新makefile
4. 忽略SIGPIPE
5. 处理write操作, 可能会返回EPIPE 错误
6. read操作时, 可能会遇到提前关闭的情况, 此时read返回-1, `errno=ECONNRESET`
7. 不是所有数据都是ascii, 有些是binary









