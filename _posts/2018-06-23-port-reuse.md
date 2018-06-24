---
layout: post
comments: yes
title: "SO_REUSEPORT 和 epoll 的 Thundering Herd"
category: "linux"
tags: [linux]
---

`SO_REUSEPORT` 顾名思义就是重用端口，是指不同的 socket 可以 bind 到同一个端口上。 Linux 内核 3.9 版本引入了这个新特性，有兴趣的同学可以移步到这个链接查看更加详细的内容。 [https://lwn.net/Articles/542629/](https://lwn.net/Articles/542629/)

我们先通过一段简单的代码来看看怎么使用这个选项（完整的 server 端代码在本文最后下载）。

```
    //创建套接字
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // 一定要在 bind() 函数之前设定好 SO_REUSEPORT
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int));

    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    //进入监听状态，等待用户发起请求
    listen(serv_sock, 20);

    //接收客户端请求
    accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

```





## 参考资料 
既然提到了 `SO_REUSEPORT`，还有一个 `SO_REUSEADDR`，请移步 [https://zhuanlan.zhihu.com/p/31329253](https://zhuanlan.zhihu.com/p/31329253)

