---
layout: post
comments: no
title: "nginx 启动流程之 init cycle"
category: "nginx"
tags: [nginx]
---

* table
{:toc}
***

从 main() 函数开始之后，很快就调用到 ngx_init_cycle()，这是 nginx 源码中一个非常重要的函数，它负责调用所有模块的init_module函数指针，初始化模块，并且解析 nginx.conf 文件中的各种参数。所以在分析 nginx 启动流程的时候，必须搞清楚这个函数做了哪些工作。

首先函数的传入参数只有一个，`ngx_cycle_t *old_cycle`。

1) 当 main() 函数调用 ngx_init_cycle() 时，因为是第一次启动 nginx，给的参数是一个刚刚初始化的变量，只填写了一些必要的信息；    
2) 另一个会调用ngx_init_cycle()是 ngx_master_process_cycle()。因为 nginx 支持动态加载 nginx.conf 文件，所以此时的传入参数就是当前的配置。


分析 ngx_master_process_cycle() 就会了解 nginx master 进程是如何等待、处理信号，并且启动新的 worker 进程的。

