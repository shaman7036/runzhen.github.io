---
layout: post
comments: yes
title: "使用 socketpair 实现进程间通信"
category: "Linux"
tags: [linux]
---

* table
{:toc}
***

## socketpair 牛刀小试

```
int socketpair(int d, int type, int protocol, int sv[2]);

第1个参数d，表示协议族，只能为 AF_LOCAL 或者 AF_UNIX；

第2个参数 type，表示类型，只能为0。

第3个参数 protocol，表示协议，可以是 SOCK_STREAM 或者 SOCK_DGRAM 
```

AF_UNIX 指的就是 Unix Domain socket，那么它与通常网络编程里面的 TCP socket 有什么区别呢？ 查阅了资料后发现：

- Unix Domain socket 是同一台机器上不同进程间的通信机制。
- IP(TCP/IP) socket 是网络上不同主机之间进程的通讯机制。


socketpair() 只支持 AF_LOCAL 或者 AF_UNIX，不支持 TCP/IP，也就是 AF_INET， 所以用 socketpair() 的话无法进行跨主机的进程间通信。

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

编译后运行，可以看到每次在终端输入信息，子进程都会回显到屏幕上。

## nginx 中的进程间通信

nginx 一开始启动只有一个 master 进程，然后 master 进程 fork 出若干个 worker 进程， 

从 `ngx_start_worker_processes()` 函数开始看起：

```
static void
ngx_start_worker_processes(ngx_cycle_t *cycle, ngx_int_t n, ngx_int_t type)
{
    for (i = 0; i < n; i++) {

        ngx_spawn_process(cycle, ngx_worker_process_cycle,
                          (void *) (intptr_t) i, "worker process", type);

        ch.pid = ngx_processes[ngx_process_slot].pid;
        ch.slot = ngx_process_slot;
        ch.fd = ngx_processes[ngx_process_slot].channel[0];

        ngx_pass_open_channel(cycle, &ch);
    }
}
```

可以看到有两个函数：
- ngx_spawn_process()  创建新进程，包括调用 socketpair() 设置好进程间通信用的文件描述符。
- ngx_pass_open_channel()  把新进程的信息传递给其他已存在的进程。

### 创建新进程
```
ngx_spawn_process(ngx_cycle_t *cycle, ngx_spawn_proc_pt proc, void *data,
    char *name, ngx_int_t respawn)
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, ngx_processes[s].channel) == -1) {
        return NGX_INVALID_PID;
    }

    /* 全局变量， 没懂为啥一定要这样  */
    ngx_channel = ngx_processes[s].channel[1];

    pid = fork();
    switch (pid) {
    case -1:
        return NGX_INVALID_PID;

    case 0:
        ngx_pid = ngx_getpid();
        /* 调用ngx_worker_process_cycle() 函数*/
        proc(cycle, data);
        break;

    default:
        break;
    }

    return pid;
}
```

proc 是一个函数指针，详见 `ngx_process_t` 结构体中的定义，它实际是调用下面这个函数：

```
ngx_worker_process_cycle(ngx_cycle_t *cycle, void *data)
{
    ngx_worker_process_init(cycle, worker);
    for(;;) {
        ngx_process_events_and_timers(cycle);
        /* 循环，但与本文不相关*/
    }
}
```

继续看 `ngx_worker_process_init()`:

```
ngx_worker_process_init(ngx_cycle_t *cycle, ngx_int_t worker)
{
    
    /* 子进程关闭 channel[0],只用channel[1] */
    if (close(ngx_processes[ngx_process_slot].channel[0]) == -1) {
    }

    if (ngx_add_channel_event(cycle, ngx_channel, NGX_READ_EVENT,
                              ngx_channel_handler) == NGX_ERROR)
    {
    }
}
```

可以看到调用了 `ngx_add_channel_event()`， 它的作用就是设置好 channel 监听的读写事件，当有事件触发时，调用相应的处理函数。

在这里的事件处理函数是 `ngx_channel_handler()`，在下面我们会看到。 


### 传递新子进程信息给已存在的子进程

父进程每调用一次 `ngx_spawn_process()` 函数后，将新fork的子进程pid和 channel[0]复制给ch变量，然后调用 `ngx_pass_open_channel()`函数把ch消息传递给之前创建的子进程。

```
ngx_pass_open_channel(ngx_cycle_t *cycle, ngx_channel_t *ch)
{
    ngx_int_t  i;

    for (i = 0; i < ngx_last_process; i++) {

        ngx_write_channel(ngx_processes[i].channel[0],
                          ch, sizeof(ngx_channel_t), cycle->log);
    }
}
```

已存在的 worker 进程调用`ngx_channel_handler()`函数，读取消息并解析成`ngx_channel_t`，根据command做相应的处理.

worker 进程收到信息更新自己的`ngx_processes`进程表，子进程就得到新创建的子进程的信息，worker 之间就可以通信了。

```
ngx_channel_handler(ngx_event_t *ev)
{
    c = ev->data;
    for ( ;; ) {

        n = ngx_read_channel(c->fd, &ch, sizeof(ngx_channel_t), ev->log);
        switch (ch.command) {
            case NGX_CMD_OPEN_CHANNEL:
            ngx_processes[ch.slot].pid = ch.pid;
            ngx_processes[ch.slot].channel[0] = ch.fd;
        }
    }
}
```



最后，总结一下 nginx 启动 worker 进程的步骤：

先创建子进程，调用 socketpair() 生成文件描述符 channel[]，并设置好一旦这个 FD 上有读/写事件，将调用哪个处理函数，然后将新 worker 信息通知给其他 worker，其他 worker 因为在创建时也设置了处理函数，于是会用 `ngx_channel_handler()` 处理。



参考资料
- [http://yikun.github.io/2014/03/16/nginxchannel/](http://yikun.github.io/2014/03/16/nginxchannel/)
- [http://don7hao.github.io/2015/02/05/nginx/2015-01-04-nginx-ipc/](http://don7hao.github.io/2015/02/05/nginx/2015-01-04-nginx-ipc/)
- [http://www.voidcn.com/article/p-uuqodvat-bkh.html](http://www.voidcn.com/article/p-uuqodvat-bkh.html)





