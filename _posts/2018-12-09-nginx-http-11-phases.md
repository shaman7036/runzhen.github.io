---
layout: post
comments: no
title: "nginx HTTP 的 11 个阶段"
category: "nginx"
tags: [nginx]
---

* table
{:toc}
***

nginx 源码的特点是用了很多回调函数，阅读起来非常麻烦，因为不知道当前这个 hanlder 到底对应哪个函数。

在正式开始研究这 11 个阶段之前，我们先看几个结构体，然后再看 ngx_http_core_run_phases() 函数，希望能更快的理解这些 phase 是怎么 run 的。

# ngx_http_core_main_conf_t 

回顾一下 ngx_http_core_main_conf_t，在前面的博客中已经介绍过，它还有两个兄弟 ngx_http_core_srv_conf_t 和 ngx_http_core_loc_conf_t。

ngx_http_core_main_conf_t 中有两个成员是本文比较关心的： phase_engine 和 phases。


```
typedef struct {
    ngx_array_t                handlers;
} ngx_http_phase_t;

typedef struct {
    // 所有的http请求都要使用这个引擎处理
    ngx_http_phase_engine_t    phase_engine;

    // http handler模块需要向这个数组添加元素
    ngx_http_phase_t           phases[NGX_HTTP_LOG_PHASE + 1];

} ngx_http_core_main_conf_t;
```

配置解析后的 postconfiguration 里向cmcf->phases数组添加元素，phases数组存放了所有的phase，其中每个元素是ngx_http_phase_t类型的，表示的就是对应的phase handler的数组。ngx_http_core_main_conf_t->phases数组主要用于handler的注册。

# ngx_http_phase_engine_t

```
typedef struct {
    ngx_http_phase_handler_t  *handlers;

    ngx_uint_t                 server_rewrite_index;
    ngx_uint_t                 location_rewrite_index;
} ngx_http_phase_engine_t;
```

# ngx_http_phase_handler_t 
```
struct ngx_http_phase_handler_s {

    ngx_http_phase_handler_pt  checker;
    ngx_http_handler_pt        handler;
    ngx_uint_t                 next;
};
```

看完了相关数据结构，特别是看到 checker、handler 的时候，是不是突然觉得熟悉了？没错，这就是上一篇博客 http 请求处理流程中，最后的 run core phase。

```
ngx_http_core_run_phases(ngx_http_request_t *r){
    ngx_http_phase_handler_t   *ph;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_get_module_main_conf(r, ngx_http_core_module);
    ph = cmcf->phase_engine.handlers;

    while (ph[r->phase_handler].checker) {
        rc = ph[r->phase_handler].checker(r, &ph[r->phase_handler]);
    }
}
```

# phase_engine[] 中的成员

看了上面的代码不禁有疑问：phase_engine[] 的成员是在哪赋值的呢？答案是 ngx_http_init_phase_handlers() 

再回头看看 ngx_http_init_phase_handlers() 的调用者，会发现是在 ngx_http_block()，原来就是在 http 模块的初始化函数中，惊不惊喜意不意外？

细看这个函数的实现，就是先计算数组大小，然后申请一块内存给 phase_engine，而如何计算数组大小呢？看代码是从 cmcf->phases[].handlers 数组获得的。

那么哪些模块可以被加入到 phases[].handlers 数组呢？ 

可以在代码中加一个简单的 printf 就能知道 11 个阶段中，每个阶段各有多少个 handler 需要加入到 phase_engine数组中。

```
    for (i = 0; i < NGX_HTTP_LOG_PHASE; i++) {
        n += cmcf->phases[i].handlers.nelts;
    }


n = 3, use_rewrite = 1, use_access = 1
n = 3, phase[0].handlers.nelts = 0
n = 4, phase[1].handlers.nelts = 1
n = 4, phase[2].handlers.nelts = 0
n = 5, phase[3].handlers.nelts = 1
n = 5, phase[4].handlers.nelts = 0
n = 7, phase[5].handlers.nelts = 2
n = 9, phase[6].handlers.nelts = 2
n = 9, phase[7].handlers.nelts = 0
n = 11, phase[8].handlers.nelts = 2
n = 14, phase[9].handlers.nelts = 3
```

即使这样，还是没有回答刚才的问题，哪些模块被加入到 phases[].handlers 数组呢？ 通过看 http static module 来解答这个问题。

# ngx_http_static_module 模块

这个 ngx_http_static_module 模块是干什么的呢？ 从名字就能看出来，它是给 nginx 提供最最基础的 http 静态文件访问功能的，比如返回 index.html 给客户端。它工作在 Content 阶段。

这个模块一共 300 行源码，逻辑清晰简单。我们先来看一下它的 init 函数。
```
ngx_http_static_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    // 使用CONTENT_PHASE
    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);

    // 添加到phases数组，完成注册
    *h = ngx_http_static_handler;

    return NGX_OK;
}
```

可以看到，static 模块自己把自己加入到了 Content 阶段的 handlers 数组中。

在网上看 ngx_http_static_module 模块的代码，发现在 ctx 结构体的 “postconfiguration”， 注册了 ngx_http_static_init 函数，那我们的问题就是，这个函数是做什么的？以及在什么时候调用？

在查阅了[一些资料](http://tengine.taobao.org/book/chapter_12.html)以后整理如下：

## 多阶段执行链

nginx按请求处理的执行顺序将处理流程划分为多个阶段，一般每个阶段又可以注册多个模块处理函数，nginx按阶段将这些处理函数组织成了一个执行链，这个执行链保存在http主配置（ngx_http_core_main_conf_t）的phase_engine字段中。

```
typedef struct {
    ngx_http_phase_handler_t  *handlers;
    ngx_uint_t                 server_rewrite_index;
    ngx_uint_t                 location_rewrite_index;
} ngx_http_phase_engine_t;
```
其中handlers字段即为执行链，实际上它是一个数组，而每个元素之间又被串成链表，从而允许执行流程向前，或者向后的阶段跳转。

```
struct ngx_http_phase_handler_s {
    ngx_http_phase_handler_pt  checker;
    ngx_http_handler_pt        handler;
    ngx_uint_t                 next;
};
```
其中checker和handler都是函数指针，相同阶段的节点具有相同的checker函数，handler字段保存的是模块处理函数，一般在checker函数中会执行当前节点的handler函数，但是例外的是NGX_HTTP_FIND_CONFIG_PHASE，NGX_HTTP_POST_REWRITE_PHASE，NGX_HTTP_POST_ACCESS_PHASE和NGX_HTTP_TRY_FILES_PHASE这4个阶段不能注册模块函数。next字段为快速跳跃索引，多数情况下，执行流程是按照执行链顺序的往前执行，但在某些执行阶段的checker函数中由于执行了某个逻辑可能需要回跳至之前的执行阶段，也可能需要跳过之后的某些执行阶段，next字段保存的就是跳跃的目的索引。

和建立执行链相关的数据结构都保存在http主配置中，一个是phases字段，另外一个是phase_engine字段。其中phases字段为一个数组，它的元素个数等于阶段数目，即每个元素对应一个阶段。而phases数组的每个元素又是动态数组（ngx_array_t），每次模块注册处理函数时只需要在对应阶段的动态数组增加一个元素用来保存处理函数的指针。由于在某些执行阶段可能需要向后，或者向前跳转，简单的使用2个数组并不方便，所以nginx又组织了一个执行链，保存在了phase_engine字段，其每个节点包含一个next域用来保存跳跃目的节点的索引。

而执行链的建立则在nginx初始化的 `post config阶段` 之后调用ngx_http_init_phase_handlers函数完成。

所以，说来说去，都是在解析 http{} 配置的处理函数 `ngx_http_block()` 中处理的。




