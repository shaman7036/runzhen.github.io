---
layout: post
comments: no
title: "nginx HTTP 链接建立过程"
category: "nginx"
tags: [nginx]
---

* table
{:toc}
***

# ngx_http_init_connection()

负责建立 http 链接的是 ngx_http_init_connection()， 我们先来看一下谁会调用这个函数。

ngx_http_optimize_servers() 是一个负责合并配置项的函数，在有关 http 配置项解析的博客中曾经提到过，它会调用 ngx_http_add_listening()。

继续看 ngx_http_add_listening(), 看到 ls->handler = ngx_http_init_connection;

即 ngx_http_init_connection() 作为一个回调函数，被设置在了 listening 结构体的 handler 中。

那么它是从哪儿被调用的呢？ 最简单的方法就是打印一下backtrace

```
#0  ngx_http_init_connection (c=0x7ffff7fa90f8) 
#1  ngx_event_accept (ev=0x73c3a0) 
#2  ngx_epoll_process_events (cycle, timer, flags) 
#3  ngx_process_events_and_timers (cycle=cycle@entry=0x716860) 
#4  ngx_single_process_cycle (cycle=cycle@entry=0x716860) 
#5  main (argc=<optimized out>, argv=<optimized out>)
```

原来是在 epoll 中由事件触发的，正所谓事件驱动。现在知道了它的调用者，那么继续看 init connection 具体做了哪些事。

我们先忽略对 ipv6、ssl 的处理，直接看最简单的情况，ssl 相关后续博客再分析。精简后的代码如下：

```
void ngx_http_init_connection() {
    c->data = hc; // data 是专门存数据的地方，之后使用

    // 拿到以前提到很多次的 ngx_http_conf_ctx_t 
    hc->conf_ctx = hc->addr_conf->default_server->ctx;

    // 连接的读事件，rev 是 ngx_event_t 类型
    rev = c->read;

    // 处理读事件，读取请求头
    // 设置了读事件的 handler，可读时就会调用 handler
    rev->handler = ngx_http_wait_request_handler;

    // 前面把读事件加入epoll，当socket有数据可读时就调用 
    // ngx_http_wait_request_handler
    // 同时因为事件也加入了定时器，超时时也会调用 handler
    ngx_handle_read_event(rev, 0);
}
```

# ngx_http_wait_request_handler()

前面我们把 read 事件加入了 epoll，当 socket 有数据可读是就会调用本函数。又因为是事件触发，可能会被多次调用，即重入。

本函数的主要做的工作非常简单：把 socket 上能读的数据读出来，然后创建一个 http request 结构体，最后调用其他函数来解析 http 请求头。

```
void ngx_http_wait_request_handler() {
    // 从事件的data获得连接对象
    c = rev->data;

    // 获取配置数组。回忆前面在 init_connection里设置
    hc = c->data;
    cscf = ngx_http_get_module_srv_conf(hc->conf_ctx, 
                            ngx_http_core_module);

    // 这里的 recv 是相当于nginx 封装的 accept() 函数
    // 具体可参考 ngx_event_accept()
    n = c->recv(c, b->last, size); 

    // 创建ngx_http_request_t
    c->data = ngx_http_create_request(c);

    rev->handler = ngx_http_process_request_line;
    ngx_http_process_request_line(rev);
}
```

最后的 ngx_http_process_request_line(rev) 要在设置给 handler 之后立马调用一次，是因为：

1）改变读事件的 handler，之后再有数据来将会调用 request_line       
2）epoll 的 ET 触发模式，需要立即处理不然就没有了。而


# ngx_http_process_request_line()

本函数调用recv读取数据，解析出请求行信息,存在 r->header_in里。使用用无限循环，保证读取完数据，如果返回 again ，说明客户端发送的数据不足，会继续读取。

在请求行处理完毕后，设置读事件处理函数 process_request_headers



