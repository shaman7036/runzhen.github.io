---
layout: post
comments: yes
title: "SO_REUSEPORT 和 epoll 的 Thundering Herd"
category: "linux"
tags: [linux]
---

`SO_REUSEPORT` 顾名思义就是重用端口，是指不同的 socket 可以 bind 到同一个端口上。 Linux 内核 3.9 版本引入了这个新特性，有兴趣的同学可以移步到这个链接查看更加详细的内容。 [https://lwn.net/Articles/542629/](https://lwn.net/Articles/542629/)

## Reuse Port 
我们先通过一段简单的代码来看看怎么使用这个选项（完整的 server 端代码在本文最后下载）。

```
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // 一定要在 bind() 函数之前设定好 SO_REUSEPORT
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int));

    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(serv_sock, 20);
    accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

```

将上面的代码编译生成两个可执行文件，分别启动运行，并监听在相同的端口上

```
./port_reuse1 127.0.0.1 1234
./port_reuse2 127.0.0.1 1234
```
再用 telnet/nc 等工具发送请求到 1234 端口上，多重复几次，会看到两个进程轮流的处理客户端发来的请求。

> 这里说一个题外话，上面的例子是手动启动两个进程。而我发现如果是进程自动 fork() 生成 2 个进程的话，似乎不用设置 SO_REUSEPORT 也能自动监听同一个端口。这是为什么？

## Thundering Herd / 惊群现象

> The thundering herd problem occurs when a large number of processes waiting for an event are awoken when that event occurs, but only one process is able to proceed at a time. After the processes wake up, they all demand the resource and a decision must be made as to which process can continue. After the decision is made, the remaining processes are put back to sleep, only to all wake up again to request access to the resource.  

所以，thundering herd 就是指多个进程在阻塞等待一个事件，当事件发生时，会唤醒所有的进程，但最终只有一个进程进行处理。




## 参考资料 
既然提到了 `SO_REUSEPORT`，还有一个 `SO_REUSEADDR`，请移步 [https://zhuanlan.zhihu.com/p/31329253](https://zhuanlan.zhihu.com/p/31329253)

