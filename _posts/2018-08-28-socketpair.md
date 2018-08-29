---
layout: post
comments: no
title: "使用 socketpair 实现进程间通信"
category: "Linux"
tags: [linux]
---

* table
{:toc}

## 介绍

```
int socketpair(int d, int type, int protocol, int sv[2]);

第1个参数d，表示协议族，只能为 AF_LOCAL 或者 AF_UNIX；

第2个参数 type，表示类型，只能为0。

第3个参数 protocol，表示协议，可以是 SOCK_STREAM 或者 SOCK_DGRAM 
```

AF_UNIX 指的就是 Unix Domain socket，那么它与通常网络编程里面的 TCP socket 有什么区别呢？ 查阅了资料后发现：

- Unix Domain socket 是同一台机器上不同进程间的通信机制。
- IP(TCP/IP) socket 是网络上不同主机之间进程的通讯机制。

先看一个简单的示例：

```
int main()
{
    int fd[2], retpid;
    int pid , status;
    char input[MAX_LEN];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0) {
        printf("call socketpair() failed, exit\n");
        return -1;
    }

    pid = fork();
    if (pid) {
        /* parent */
        printf("Parent process, pid = %d\n", getpid());
        while (1) {
            fgets(input, MAX_LEN, stdin);
            write(fd[0], input, MAX_LEN); 
        }
    } else {
        /* child */
        printf("Child process, pid = %d\n", getpid());
        int nread = 0;
        while (1) {
            nread = read(fd[1], input, MAX_LEN);
            input[nread] = '\0';
            printf("Child: nread = %d, data = %s\n", nread, input);
        }
    }

    retpid = wait(&status);
    if (retpid) {
        printf("Parent: reap child process pid = %d\n", retpid);
    }
    return 0;
}
```

## nginx 中 channel 的实现

