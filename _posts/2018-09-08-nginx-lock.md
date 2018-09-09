---
layout: post
comments: yes
title: "nginx 文件锁、原子锁的实现"
category: "Linux"
tags: [nginx]
---

* table
{:toc}
***

在上一篇博客 [Linux 共享内存以及 nginx 中的实现](https://blog.nlogn.cn/share-memory/)的示例中，我们看到每次多个进程同时对共享内存中的 count 加一，导致每次运行结果都不一样，那么解决的方法就是对临界区加锁了，所以本文就来研究一下 nginx 中的几种加锁方式。

## 文件锁

文件锁的原理就是在磁盘上创建一个文件（操作系统创建一个文件描述符），然后多个进程竞争去获取这个文件的访问权限，因此同一时刻只有一个进程能够访问临界区。

可以看出，进程并不会真正在这个文件中写什么东西，我们只是想要一个文件描述符 FD 而已，因此 nginx 会在创建了文件后把这个文件删除，只留下文件描述符。

> 多个进程打开同一个文件，各个进程看到的文件描述 FD 值可能会不一样。例如文件 test.txt 在 进程1 中是 101， 而在进程2中是 201

### 使用文件锁举例

使用文件锁主要用到两个 libc 提供的结构体和函数。

- struct flock; 提供一些锁的基本信息，比如读锁 F_RDLCK, 还是写锁 F_WRLCK
- [fcntl()](http://man7.org/linux/man-pages/man2/fcntl.2.html): 对文件描述符进行操作的库函数。

那么如何用这个函数来实现锁的功能呢？ 

先看一个**加锁**的代码：

```
void mtx_file_lock(struct fdmutex *m)
{
    struct flock fl;
    memset(&fl, 0, sizeof(struct flock));

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;

    if (fcntl(m->fd, F_SETLKW, &fl) == -1) {
        printf("[-] PID %d, lock failed (%s).\n", getpid(), strerror(errno));
    }
}
```

再来看一个**解锁**的代码：
```
void mtx_file_unlock(struct fdmutex *m)
{
    struct flock fl;
    memset(&fl, 0, sizeof(struct flock));

    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;

    if (fcntl(m->fd, F_SETLK, &fl) == -1) {
        printf("[-] PID %d, unlock failed (%s).\n", getpid(), strerror(errno));
    }
}
```

遇到临界区，只要在需要进行保护的代码前后加上上面的 lock() 和 unlock() 函数就可以了。

下面来运行一下完整的代码，源代码包含三个文件 Makefile，set, get

- set : 对共享内存中的 count 执行 10000 次加一。
- Makefile : 开启多个 set 程序，因此多个 set 会同时对共享内存的 count 操作。
- get : 等多个 set 运行完成后，运行 get 获取共享内存中的值。

在上一篇博客 [Linux 共享内存以及 nginx 中的实现](https://blog.nlogn.cn/share-memory/) 中我们已经看到，不加锁的情况下，每次的结果都不同；现在我们对 for 循环的前后都加上文件锁，再来看看结果：

先用 make 命令生成可执行文件，然后 `make run`, 开启 6 个 set 进程同时运行

```
$ make
gcc -o set set.c -lrt -lpthread
gcc -o get get.c -lrt -lpthread


$ make run
./set &
./set &
[+] PID 16893 start
./set &
...
...
```

最后，调用 get 程序获取共享内存中的 count 值，

```
$ ./get
./GET pid 16904: Read from shared memory: 60000
``` 

多运行几次，可以发现每次的结果都是 60000，证明文件锁对临界区起到了保护作用。


## 原子锁

文件锁的操作效率不及原子锁，原子锁是利用 CPU 提供的原子操作功能来实现锁，可见效率上更胜一筹。

## nginx 中的实现

