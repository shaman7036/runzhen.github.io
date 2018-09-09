---
layout: post
comments: yes
title: "nginx 文件锁、自旋锁的实现"
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

先用 make 命令生成可执行文件，

```
$ make
gcc -o set set.c -lrt -lpthread
gcc -o get get.c -lrt -lpthread

```

然后执行 `make run`, 开启 6 个 set 进程同时运行，再 sleep 5 秒钟，这是为了让 set 有充分的时间去执行完毕，最后再用 get 进程去获取共享内存里面的 count 值。

```
$ make run
[+] PID 21314 start
[+] PID 21316 start
[+] PID 21318 start
[+] PID 21320 start
[+] PID 21322 start
sleep 5
[+] PID 21324 start
./GET pid 21326: Read from shared memory: 60000
```

多运行几次，可以发现每次的结果都是 60000，证明文件锁对临界区起到了保护作用。

> 如果看到运行的结果不是 60000，可以修改 Makefile 把 sleep 的时间增大，确保所有的 set 都执行完毕再调用 get。

> 另一个方法是修改 Makefile 的 run 部分，不自动运行 get 程序，只执行多个 set；然后用 `ps aux | grep set` 查看系统中有多少个 set 还在运行，确保没有之后再手动执行  ./get 


## 基于原子操作的锁

文件锁的操作效率不及原子锁，原子锁是利用 CPU 提供的原子操作功能来实现锁，比如 `Compare-and-Swap` 指令 `cmpxchgl`， 可见效率上更胜一筹。

不过，CPU 只提供了汇编级别的操作指令，我们的 C 语言程序想要用原子操作，要么自己实现，要么调用库函数。在 nginx 的实现中，针对不同的平台、不同的函数库都有相应的处理。

在本文的例子中，为了简单起见，直接使用了 nginx 的 `ngx_atomic_cmp_set()` 在 x86 架构上的代码，位于 `/src/os/unix/ngx_gcc_atomic_x86.h`。

### 自旋锁的简单实现

完整的代码请在这里下载。

有了自己的 `Compare-and-Swap` 函数后，就可以实现 lock 和 unlock。另外有一点很重要：因为是对多个进程进行互斥，所以`锁的结构必须保存在共享内存区域`，这一点不难理解，如果放在进程的私有空间，那么各个进程各玩各的，起不到互斥的作用。

下面是 **加锁**函数，

```
void atomic_lock(struct shared_area *m, uint64_t pid)
{
    for (;;) {
        if (m->lock == 0 && atomic_cmp_set(&m->lock, 0, pid)) {
            return;
        }
    }
}
```

可以看出，上面的加锁代码直接使用了一个 for 循环，当无法获取锁时，无限等待，所以这是一个 `自旋锁`。为了避免无限等待，nginx 中会用信号量等技巧作一些避免，具体请参考 `src/core/ngx_shmtx.c` 。

下面是**解锁**代码

```
void atomic_unlock(struct shared_area *m, uint64_t pid)
{
    atomic_cmp_set(&m->lock, pid, 0);
}
```

最后，我们的测试代码与文件锁的非常类似，只是把加锁解锁函数换成了原子锁实现而已，运行的结果也非常类似：
```
$ make run
[+] SET PID 21641 start
[+] SET PID 21643 start
[+] SET PID 21645 start
[+] SET PID 21647 start
[+] SET PID 21649 start
sleep 5
[+] SET PID 21651 start
./GET pid 21653: Read from shared memory: 60000
``` 

> 如果结果不是 60000，修改 Makefile 把 sleep 时间增大，确保所有 set 执行完毕再调用 get。

## nginx 中的实现

先来总结一下 nginx 中的锁，主要是两大类：

1. 文件锁：根据传给 fcntl() 的参数，又可以加锁时阻塞和非阻塞
    - F_SETLKW，表示获取不到文件锁时，阻塞直到可以获取
    - F_SETLK, 获取不到锁时会直接返回，不会阻塞进程。因为会直接返回，所以需要在外部在包装一个 `ngx_shmtx_trylock()` 函数。
2. 原子锁：用库函数或者nginx实现，取决于 configure 脚本生成的宏定义。
    - 也分为阻塞的和非阻塞的，但是这与锁本身的实现没有关系，而是靠额外的信号量来实现阻塞。


关于源代码的分析，网上有很多资料（比如参考资料1），下面是我的整理，

```
ngx_shmtx_create(ngx_shmtx_t *mtx, ngx_shmtx_sh_t *addr, u_char *name)
{
    mtx->lock = &addr->lock;
    // 不支持信号量时，spin 表示锁的自旋次数，为0或负数表示不进行自旋，直接让出cpu，
    // 支持信号量时，-1 表示不要使用信号量将进程置于睡眠状态
    if (mtx->spin == (ngx_uint_t) -1) {
        return NGX_OK;
    }
    // 默认自旋次数
    mtx->spin = 2048;
    
#if (NGX_HAVE_POSIX_SEM)
    mtx->wait = &addr->wait;
    //初始化信号量，第二个参数1表示，信号量使用在多进程环境中，第三个参数0表示信号量的初始值
    //当信号量的值小于等于0时，尝试等待信号量会阻塞
    //当信号量大于0时，尝试等待信号量会成功，并把信号量的值减一。
    if (sem_init(&mtx->sem, 1, 0) == -1) {
    } else {
        mtx->semaphore = 1;
    }
#endif
    return NGX_OK;
}
```

### 阻塞式的锁

这是唯一理解起来麻烦一点的地方。总的来说，我们的目标是`原子操作的互斥锁，阻塞式的`，那么阻塞怎么实现呢？

如果系统不支持信号量，那么获取不到锁的时候，我们让他自旋一会儿，这与我自己的实现是一样的，只不过我用了无限 for 循环，粗暴。而 nginx 的实现则优雅一点，用 mtx->spin 的值指定重试的最大次数。

如果系统支持信号量，那么就不用 spin 自旋了，从`ngx_shmtx_create()` 函数就能看出来，直接用系统的信号量实现阻塞。

同一时间，二者选其一就行了。 不过信号量的效率没有自旋好，所以一般不使用。例如 `ngx_accept_mutex` 就直接把 spin 设置为 -1 了。

```
void
ngx_shmtx_lock(ngx_shmtx_t *mtx)
{
    for ( ;; ) {
        //注意：由于在多进程环境下执行，*mtx->lock == 0 为真时，并不能确保后面的原子操作执行成功
        if (*mtx->lock == 0 && ngx_atomic_cmp_set(mtx->lock, 0, ngx_pid)) {
            return;
        }
        // 获取锁失败了，这时候判断cpu的数目，如果数目大于1，则先自旋一段时间，然后再让出cpu
        // 如果cpu数目为1，则没必要进行自旋了，应该直接让出cpu给其他进程执行。
        if (ngx_ncpu > 1) {
            for (n = 1; n < mtx->spin; n <<= 1) {
                for (i = 0; i < n; i++) {
                    // ngx_cpu_pause函数并不是真的将程序暂停，
                       而是为了提升循环等待时的性能，并且可以降低系统功耗。
                    // 实现它时往往是一个指令： `__asm__`("pause")
                    ngx_cpu_pause();
                }
                
                // 再次尝试获取锁
                if (*mtx->lock == 0
                    && ngx_atomic_cmp_set(mtx->lock, 0, ngx_pid))
                {
                    return;
                }
            }
        }
        // 支持信号量
#if (NGX_HAVE_POSIX_SEM)
        // 上面自旋次数已经达到，依然没有获取锁，将进程在信号量上挂起，等待其他进程释放锁后再唤醒。
        if (mtx->semaphore) {         
            // 当前在该信号量上等待的进程数目加一
            (void) ngx_atomic_fetch_add(mtx->wait, 1);
            // 尝试获取一次锁，如果获取成功，将等待的进程数目减一，然后返回
            if (*mtx->lock == 0 && ngx_atomic_cmp_set(mtx->lock, 0, ngx_pid)) {
                (void) ngx_atomic_fetch_add(mtx->wait, -1);
                return;
            }
            //  在信号量上进行等待
            while (sem_wait(&mtx->sem) == -1) {
                ngx_err_t  err;
                err = ngx_errno;
                if (err != NGX_EINTR) {
                    break;
                }
            }

            // 执行到此，肯定是其他进程释放锁了，
            所以继续回到循环的开始，尝试再次获取锁
            continue;
        }
#endif
        // 在没有获取到锁，且不使用信号量时，会执行到这里.
           通过 sched_yield 函数让调度器暂时将进程切出，让其他进程执行。
        // 在其它进程执行后有可能释放锁，那么下次调度到本进程时，则有可能获取成功。
        ngx_sched_yield();
    }
}
```

### 非阻塞式的锁

非阻塞的锁就非常简单了
```
ngx_shmtx_trylock(ngx_shmtx_t *mtx)
{
    return (*mtx->lock == 0 && ngx_atomic_cmp_set(mtx->lock, 0, ngx_pid));
}
```

## 相关资料

- [nginx互斥锁](http://shibing.github.io/2017/06/22/nginx%E4%BA%92%E6%96%A5%E9%94%81%E7%9A%84%E5%AE%9E%E7%8E%B0%E4%B8%8E%E4%BD%BF%E7%94%A8/)
- [http://simohayha.iteye.com/blog/658012](http://simohayha.iteye.com/blog/658012)

