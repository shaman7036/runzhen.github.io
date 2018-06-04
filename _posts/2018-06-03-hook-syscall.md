---
layout: post
comments: no
title: "如何拦截一个系统调用 ? "
category: "Linux"
tags: [linux]
---

系统调用是用户态程序进入内核态的唯一途径，如果内核的系统调用处理函数存在 bug，那么攻击者就会利用这个漏洞拿到 root 权限。所以很多沙箱都会对系统调用进行拦截，或者拦截后自己处理（例如容器沙箱 gVisor）。

那么问题来了，如何拦截一个系统调用 ? 有两种方法：

- LD_PRELOAD 环境变量 : 直接作用在可执行文件上。
- ptrace() : 拦截子进程的系统调用。


## LD_PRELOAD

LD_PRELOAD 的优势:

- 使用简单。
- 不需要修改被拦截程序的源码。

例如我想拦截程序 A 所有调用 malloc() 的地方，那么程序 A 不需要任何修改，只要准备好自己的 malloc() 函数，编译成动态链接库 .so 文件，然后在运行 A 之前先用 LD_PRELOAD 设定好环境变量就可以了。

LD_PRELOAD 的原理就是链接器在动态链接的时刻，优先链接 LD_PRELOAD 指定的函数。

说到这里，LD_PRELOAD 的缺点也非常明显，它`只能作用于动态链接库`，要是静态链接的就没戏了。


## ptrace()

ptrace 是 linux 内核原生提供的一个功能，因此功能比 LD_PRELOAD 强大的多。它最初的目的是用来 debug 的，例如大名鼎鼎的 gdb 就是依赖于 ptrace。

要使用 ptrace 拦截程序 A 的系统调用，有两种方法：

* ptrace 一个新进程：在代码中 fork 一个子进程，子进程执行 ptrace(PTRACE_TRACEME, 0, 0, 0)函数，然后通过 execv() 调用程序 A。
* attach 到已运行的程序 A ：执行ptrace(PTRACE_ATTACH, pid, 0, 0)。


以上两种方式，ptrace 都会拦截发送到 A 进程的所有信号（除 SIGKILL 外），然后我们需要自己选择哪些系统调用需要拦截，并在拦截后转到我们自己的处理函数。


## Talk is cheap, show you the code




### 其他资料
* https://www.linuxjournal.com/article/6100



