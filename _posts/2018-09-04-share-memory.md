---
layout: post
comments: yes
title: "Linux 共享内存以及 nginx 中的实现"
category: "Linux"
tags: [linux]
---

* table
{:toc}
***

## 共享内存方法简介

Linux/Unix系统中，共享内存可以通过两个系统调用来获得，mmap 和 shmget/shm_open，其中 shmget 和 shm_open 分别属于不同的标准：

- POSIX 共享内存（shm_open()、shm_unlink()）
- System V 共享内存（shmget()、shmat()、shmdt()）

shmget 和 shm_open 类似的地方在于都是创建一个共享内存，挂载到 `/dev/shm` 目录下，并且返回一个文件描述符，fd。

区别是 POSIX 共享内存没有提供将 fd 映射到进程地址空间的方法，而 System V 共享内存则直接提供了 shmat()，之后再 nginx 的实现中会再次看到。


mmap 语义上比 shmget 更通用，因为它最一般的做法，是将一个打开的实体文件，映射到一段连续的内存中，各个进程可以根据各自的权限对该段内存进行相应的读写操作，其他进程则可以看到其他进程写入的结果。

而 shmget 在语义上相当于是匿名的 mmap，即不关注实体文件，直接在内存中开辟这块共享区域，mmap 通过设置调用时的参数，也可达到这种效果，一种方法是映射`/dev/zero` 设备,另一种是使用`MAP_ANON`选项。


mmap() 的函数声明如下：
```
void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);

addr： 要映射的起始地址，通常指定为NULL，让内核自动选择
len：  映射到进程地址空间的字节数
prot： 映射区保护方式
flags：标志
fd：   文件描述符
offset：从文件头开始的偏移量
返回值： 成功返回映射到的内存区的起始地址；失败返回-1
```

## nginx 中的实现

## 动手实验

## 参考资料



