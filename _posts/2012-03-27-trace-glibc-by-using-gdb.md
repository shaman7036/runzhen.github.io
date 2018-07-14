---
layout: post
comments: yes
title: "用GDB追踪glibc代码执行过程"
category: gdb
tags: [linux]
---

首先需要安装一下额外的工具包，一个是 libc6-dbg，这是带有debug symbol信息的 libc.so；另一个是libc6-dev，这是glibc的源代码，获取之后我们就可以在gdb中查看代码了。

在Ubuntu/Debian 系统上，我们可以通过以下2条命令获得：

`$sudo apt-get install libc6-dbg`

`$sudo apt-get source libc6-dev`


在Fedora/Red Hat 系的OS上，需要安装的软件包的名字不叫 libc6-dbg，libc6-dev，貌似应该是glibc-debuginfo。

之后，以一段小程序为例来演示整个过程，小程序包含了一个系统调用fork()，一个库函数printf()

```
int main(){
    pid_t son;
    if((son=fork())==0)
       printf("I am son\n");
    else
       printf("I am farther\n");
    return 0;
}
```

接着，编译产生带有调试信息的可执行文件 `$gcc -g -o f fork.c`
然后开启gdb调试 $gdb fork

在开始调试之前，需要指定一下刚刚获得的带有libc6-dev源码文件夹的路径，比如我把这些源码放在了 ~/glibc/lib 文件夹下，通常一般程序需要的是stdio-common这个目录内的文件，

于是输入 `(gdb) directory ~/glibc/lib/stdio-common`

![gdb](/image/2012/gdb.png)

注意看其中几条命令的用法。

程序在调用fork函数后，其实执行的是glibc包装过的__libc_fork ，并且我们可以查看其源代码。
这里有几个常用命令：
- s 单步执行；
- list 查看源代码；

start 程序开始执行，并在main函数处停下，相当于在main处加断点。
但是在执行了几步之后出现了这样的错误：

>_IO_list_lock () at genops.c:1299    
>1299 genops.c: No such file or directory.   
>in genops.c   


这是因为gdb没有找到genops.c文件，我们需要像刚才一样，用directory命令指明路径。
怎么知道路径呢？很简单，在刚刚获得的libc6-dev源码目录下搜索genops.c文件，然后拷贝路径就可以了。

`(gdb)directory ~/glibc/lib/libio`

之后就可以正常执行了。
