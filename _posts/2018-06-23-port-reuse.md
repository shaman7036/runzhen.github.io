---
layout: post
comments: yes
title: "SO_REUSEPORT 和 epoll 的 Thundering Herd"
category: "linux"
tags: [linux]
---

`SO_REUSEPORT` 顾名思义就是重用端口，是指不同的 socket 可以 bind 到同一个端口上。 Linux 内核 3.9 版本引入了这个新特性，有兴趣的同学可以移步到这个链接查看更加详细的内容。 [https://lwn.net/Articles/542629/](https://lwn.net/Articles/542629/)

## Reuse Port 
我们先通过一段简单的代码来看看怎么使用这个选项（完整的代码在[这里下载](/image/2018/server.c)）。

```
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // 一定要在 bind() 函数之前设定好 SO_REUSEPORT
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int));

    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(serv_sock, 20);
    accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

```

将上面的代码编译生成两个可执行文件，分别启动运行，并监听相同的端口。

`./port_reuse1 127.0.0.1 1234`

再用 telnet/nc 等工具发送请求到 1234 端口上，多重复几次，会看到两个进程轮流的处理客户端发来的请求。

> 这里说一个题外话，上面的例子是手动启动两个进程。而我发现如果是进程自动 fork() 生成 2 个进程的话，似乎不用设置 SO_REUSEPORT 也能自动监听同一个端口。这是为什么？

## Thundering Herd / 惊群现象

> The thundering herd problem occurs when a large number of processes waiting for an event are awoken when that event occurs, but only one process is able to proceed at a time. After the processes wake up, they all demand the resource and a decision must be made as to which process can continue. After the decision is made, the remaining processes are put back to sleep, only to all wake up again to request access to the resource.  

所以，thundering herd 就是指多个进程在阻塞等待一个事件，当事件发生时，会唤醒所有的进程，但最终只有一个进程进行处理。

回顾上一小节的“端口重用”，进程1 和进程2 都监听在 1234 端口上，在运行到 accept() 函数后，进程阻塞进入睡眠状态，当有请求到达时，这两个进程会被同时唤醒；如果有100个进程，那么这100个都会全部唤醒。

不过，在搜索了网上的资料后，有的提到 Linux 内核已经解决了 accept() 函数的惊群问题，但是目前我没有找到对应的 changelist，而且我没想出一个方法可以直观的看到是不是真的只有一个进程被唤醒了。

下面就用 epoll 直观的感受一下多个进程同时被唤醒的情况。完整的代码在[这里下载](/image/2018/epoll_server.c)

```
   /* make socket non blocking */
    flags |= O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);

    event.data.fd = fd; /* socket fd */
    event.events = EPOLLIN;

    /* add socket fd to epoll efd */
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event); 

    while (1) {
        n = epoll_wait(efd, eventsp, MAX_EVENT, -1);
        printf("child [%d] wake up from epoll_wait\n", getpid());

        /* sleep to make the "thundering herd" happen */
        sleep(1);

        for (i = 0; i < n; i++) {
            infd = accept(fd, &in_addr, &in_len);
            printf("child %d accept successed\n", getpid());
        }
    }
```

运行上面的代码后，得到下面这样的输出，可以看出，5 个子进程确实都被唤醒了。

![](image/2018/epoll.png){:height="300" width="300"}

### TODO  
nginx 中的 epool 和 accept_mutex

## 参考资料 

- 既然提到了 `SO_REUSEPORT`，还有一个 `SO_REUSEADDR`，[请移步](https://zhuanlan.zhihu.com/p/31329253)
- https://pureage.info/2015/12/22/thundering-herd.html
- https://www.cnblogs.com/Anker/p/7071849.html





