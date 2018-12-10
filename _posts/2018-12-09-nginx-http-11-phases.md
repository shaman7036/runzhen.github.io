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

细看这个函数的实现，就是先计算数组大小，然后申请一块内存给 phase_engine，而如何计算数组大小呢？看代码是从 cmcf->phases[] 数组获得的。

那么哪些模块可以被加入到 phases[] 数组呢？ 这个还没搞清楚。

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





