---
layout: post
comments: yes
title: "如何拦截库函数调用 ? "
category: "Linux"
tags: [linux]
---

- `LD_PRELOAD 环境变量` : 直接作用在可执行文件上 (准确的说是**拦截库函数**) 。
- `ptrace()` : 拦截子进程的系统调用。


## 1. LD_PRELOAD

LD_PRELOAD 的优势:

- 使用简单。
- 不需要修改被拦截程序的源码。

例如我想拦截程序 A 所有调用 malloc() 的地方，那么程序 A 不需要任何修改，只要准备好自己的 malloc() 函数，编译成动态链接库 .so 文件，然后在运行 A 之前先用 LD_PRELOAD 设定好环境变量就可以了。

LD_PRELOAD 的原理就是链接器在动态链接的时刻，优先链接 LD_PRELOAD 指定的函数。准确的说 LD_PRELOAD 拦截的是动态库中的函数，但是一般我们写的应用程序都是通过库函数来调用系统调用 API，所以 LD_PRELOAD 也间接的拦截了系统调用。

说到这里，LD_PRELOAD 的缺点也非常明显，它`只能作用于动态链接库`，要是静态链接的就没戏了。

腾讯的 C++ 协程库 [libco](https://github.com/Tencent/libco)，以及 tcmalloc 的 TC_MALLOC 都用到了这种方式。

## 2. ptrace()

ptrace 是 linux 内核原生提供的一个功能，因此功能比 LD_PRELOAD 强大的多。它最初的目的是用来 debug，例如大名鼎鼎的 gdb 就是依赖于 ptrace。

要使用 ptrace 拦截程序 A 的系统调用，有两种方法：

* ptrace 一个新进程：在代码中 fork 一个子进程，子进程执行 ptrace(PTRACE_TRACEME, 0, 0, 0)函数，然后通过 execv() 调用程序 A。
* attach 到已运行的程序 A ：执行ptrace(PTRACE_ATTACH, pid, 0, 0)。


以上两种方式，ptrace 都会拦截发送到 A 进程的所有信号（除 SIGKILL 外），然后我们需要自己选择哪些系统调用需要拦截，并在拦截后转到我们自己的处理函数。


## LD_PRELOAD

先来玩转一下 LD_PRELOAD，拦截 malloc() 函数。

首先准备一个非常简单的测试程序 main.c
```
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char *p = (char *)malloc(100);
    if (p) {
        printf("malloc() success\n");
    } else {
        printf("malloc() return NULL\n");
    }
    return 0;
}
```

正常情况下我们编译运行这个 c 文件，最后会显示 “malloc success”。

再准备一个 hook.h 文件
```
#ifndef _HOOK_H_
#define _HOOK_H_

#include <stdio.h>

void *malloc(int size);

#endif
```
hook.c 文件

```
#include "hook.h"

void * malloc(int size)
{
    return NULL;
}
```

OK，万事俱备，下面来编译和链接：

* `gcc hook.c -fPIC -shared -o libhook.so`，生成我们自己的 hook 动态链接库
* `gcc main.c -o test`，生成 test 可执行文件。
* `export LD_LIBRARY_PATH=.` 设置好环境变量。
* `LD_PRELOAD=$PWD/hook.so ./test`

最后一步做完，屏幕上将会看到 “malloc() return NULL”， 证明 malloc 已经被替换，并且返回了 NULL。


## ptrace


### 其他资料
* https://www.linuxjournal.com/article/6100
* http://recursiveg.me/2014/04/programming-with-ptrace-part2/



