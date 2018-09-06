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

区别是 POSIX 没有提供将 fd 映射到进程地址空间的方法，而 System V 方式则直接提供了 shmat()，之后再 nginx 的实现中会再次看到。


mmap 语义上比 shmget 更通用，因为它最一般的做法，是将一个打开的实体文件，映射到一段连续的内存中，各个进程可以根据各自的权限对该段内存进行相应的读写操作，其他进程则可以看到其他进程写入的结果。

而 shmget 在语义上相当于是匿名的 mmap，即不关注实体文件，直接在内存中开辟这块共享区域，mmap 通过设置调用时的参数，也可达到这种效果，一种方法是映射`/dev/zero` 设备,另一种是使用`MAP_ANON`选项。


mmap() 的函数原型如下，具体参数含义在最后的参考资料中给出。

`void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);`


## nginx 中的实现

nginx 中是怎么实现的呢？ 我们看一下源码 `src/os/unix/ngx_shmem.c`。

一目了然，简单粗暴有木有！ 分三种情况

1. 如果mmap系统调用支持 MAP_ANON选项，则使用 MAP_ANON 
2. 如果1不满足，如果mmap系统调用支持映射/dev/zero设备，则映射/dev/zero来实现。
3. 如果1和2都不满足，且如果支持shmget的话，则使用该shmget来实现。

```
#if (NGX_HAVE_MAP_ANON)

ngx_int_t
ngx_shm_alloc(ngx_shm_t *shm)
{
    shm->addr = (u_char *) mmap(NULL, shm->size,
                                PROT_READ|PROT_WRITE,
                                MAP_ANON|MAP_SHARED, -1, 0);
    return NGX_OK;
}

#elif (NGX_HAVE_MAP_DEVZERO)

ngx_int_t
ngx_shm_alloc(ngx_shm_t *shm)
{
    ngx_fd_t  fd;

    fd = open("/dev/zero", O_RDWR);

    shm->addr = (u_char *) mmap(NULL, shm->size, PROT_READ|PROT_WRITE,
                                MAP_SHARED, fd, 0);

    return (shm->addr == MAP_FAILED) ? NGX_ERROR : NGX_OK;
}

#elif (NGX_HAVE_SYSVSHM)

ngx_int_t
ngx_shm_alloc(ngx_shm_t *shm)
{
    int  id;

    id = shmget(IPC_PRIVATE, shm->size, (SHM_R|SHM_W|IPC_CREAT));

    shm->addr = shmat(id, NULL, 0);

    if (shmctl(id, IPC_RMID, NULL) == -1) {
        ngx_log_error(NGX_LOG_ALERT, shm->log, ngx_errno,
                      "shmctl(IPC_RMID) failed");
    }

    return (shm->addr == (void *) -1) ? NGX_ERROR : NGX_OK;
}
```
 
> 上面代码中的宏定义（比如 NGX_HAVE_MAP_ANON ）是怎么来的呢？编译 nginx 源码之前的一步是运行 configure 文件，它会调用 auto/unix 脚本 ，该脚本会写一端测试程序来判断相应的系统调用是否支持，如果支持，则在自动生成的 objs/ngx_auto_config.h 文件中定义对应的宏。 

## 一个简单的示例




## 参考资料

- [https://rocfang.gitbooks.io/dev-notes/content/nginxzhong_de_jin_cheng_jian_tong_xin.html](https://rocfang.gitbooks.io/dev-notes/content/nginxzhong_de_jin_cheng_jian_tong_xin.html)


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


