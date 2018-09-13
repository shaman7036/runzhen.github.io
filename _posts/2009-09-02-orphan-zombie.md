---
layout: post
comments: no
title: "孤儿进程和僵尸进程"
category: "Linux"
tags: [linux]
---

* table
{:toc}

孤儿进程和僵尸进程，一个老生常谈的问题，现在把它记录下来，以后查阅起来也方便。

## 定义

`孤儿进程`： 父进程退出，而它的一个或多个子进程还在运行，那么那些子进程将成为孤儿进程。孤儿进程将被init进程(进程号为1)所收养，并由init进程对它们完成状态收集工作。

`僵尸进程`： 子进程退出，而父进程并没有调用 wait 或 waitpid 获取子进程的状态信息，那么子进程的进程描述符仍然保存在系统中。


所以，**孤儿进程并没有什么危害，而僵尸进程，因为它一直占据进程描述符，占用系统资源，所以有一定的危害**


## 如何避免

### 通过信号机制

子进程退出时向父进程发送SIGCHILD信号，父进程处理SIGCHILD信号。在信号处理函数中调用wait进行处理僵尸进程。

### fork() 两次

原理是将子进程成为孤儿进程，从而其的父进程变为init进程，通过init进程处理僵尸进程。代码的逻辑结构大致如下:

```
int main()
{
    pid_t  pid;
    //创建第一个子进程
    pid = fork();
    if (pid < 0) {
        perror("fork error:");
        exit(1);
    } else if (pid == 0) {
       
        pid = fork();

    	if (pid >0) {
            printf("first procee is exited.\n");
            exit(0);
        }

        //第二个子进程
        do_something();    
    }

    //父进程处理第一个子进程退出
    if (waitpid(pid, NULL, 0) != pid)
    {
        perror("waitepid error:");
        exit(1);
    }
    return 0;
}
```

参考资料
- https://www.cnblogs.com/Anker/p/3271773.html

