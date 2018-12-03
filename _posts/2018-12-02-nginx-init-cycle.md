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

先来看看 ngx_cycle_t 结构体里的成员，这里只列出本文关心的几个:

```
struct ngx_cycle_s {
    // 存储所有模块的配置结构体，是个二维数组
    // 0 = ngx_core_module
    // 1 = ngx_errlog_module
    // 3 = ngx_event_module
    // 4 = ngx_event_core_module
    // 5 = ngx_epoll_module
    // 7 = ngx_http_module
    // 8 = ngx_http_core_module
    void                  ****conf_ctx;

    // 保存模块数组，可以加载动态模块
    // 可以容纳所有的模块，大小是ngx_max_module + 1
    // ngx_cycle_modules()初始化
    ngx_module_t            **modules;

    // 拷贝模块序号计数器到本cycle
    // ngx_cycle_modules()初始化
    ngx_uint_t                modules_n;

    // 标志位，cycle已经完成模块的初始化，不能再添加模块
    // 在ngx_load_module里检查，不允许加载动态模块
    ngx_uint_t                modules_used;
}   
```

其中最最重要的莫过于 conf_ctx 了，回顾[上一篇博客里的图](/image/2018/ngx-conf.png)，重点关注一下其中 ngx_http_module，需要搞清楚这些配置是怎么解析到 conf_ctx 中去的。

看完了数据结构，现在开始分析 ngx_init_cycle()。以下章节，一级标题表示这段代码或者函数是在 ngx_init_cycle() 中的，二级标题下的代码段或者函数是在上级标题的函数中调用的。

# 分配 cycle->conf_ctx 数组

创建配置结构体数组，大小是总模块数量。  

`cycle->conf_ctx = ngx_pcalloc(pool, ngx_max_module * sizeof(void *));`

那么 ngx_max_module 是从哪来的呢？ 还记得 ./configure 生成的 objs/ngx_modules.c 吗？ ngx_preinit_modules() 负责统计所有模块的个数。

# ngx_cycle_modules() 

这个函数名字看起来好像很牛逼很复杂，其实代码非常简单，就是把 objs/ngx_modules.c 的内容复制到 cycle->modules 成员里面，并全部初始化为 NULL。

# 初始化core模块

objs/ngx_modules.c 文件中的 ngx_modules 数组决定了各个模块的顺序，然后因为有 extern 关键字，所以这些模块的定义都在其他文件中。

```
    for (i = 0; cycle->modules[i]; i++) {
        // 检查type，只处理core模块，数量很少
        if (cycle->modules[i]->type != NGX_CORE_MODULE) {
            continue;
        }

        //获取core模块的函数表
        module = cycle->modules[i]->ctx;

        // 创建core模块的配置结构体
        // 有的core模块可能没有这个函数，所以做一个空指针检查
        if (module->create_conf) {
            rv = module->create_conf(cycle);
            if (rv == NULL) {
                ngx_destroy_pool(pool);
                return NULL;
            }
            // 存储到cycle的配置数组里，用的是index，不是ctx_index
            cycle->conf_ctx[cycle->modules[i]->index] = rv;
        }
    }

```

针对 cycle->modules[] 数组中的每一个，比如 http 模块:

```
ngx_module_t  ngx_http_module = {
    ....
    &ngx_http_module_ctx,                  /* module context */
    ....
    NGX_CORE_MODULE,                       /* module type */
};
```

在 ngx_http.c 中，它自己声明了是核心模块 **NGX_CORE_MODULE**，所以它的 ctx 结构体是 ngx_core_module_t ngx_http_module_ctx，并且在 ctx 中提供了 create_conf 函数指针（http 的这个指针为 NULL）。

# ngx_conf_parse() 

开始解析配置文件 nginx.conf。前面我们调用了每个 Core 类型模块的 create_conf()，现在 ngx_conf_parse() 将会调用每个 Core 类型模块的 init_conf() 方法。

create_conf() 是创建一些模块需要初始化的结构，但是这个结构里面并没有具体的值。init_conf() 是往这些初始化结构里面填写配置文件中解析出来的信息。

先使用 ngx_conf_read_token() 先循环逐行逐字符查找，看匹配的字符，获取出cmd, 然后去所有的模块查找对应的cmd,调用那个查找后的 cmd->set 方法。

以 http 模块为例，我们的配置文件中一定有且只有一个
```
http{

}
```

先解析这个配置的时候发现了http这个关键字，然后去各个模块匹配，发现 ngx_http_module 这个模块包含了http命令。它对应的set方法是 ngx_http_block() 。 event模块也有类似的方法，ngx_events_block。

还有的关键字可以自己指定对后续内容的处理方法，比如 type 指令，在 conf/mime.types 文件中能看到这样的：

```
types {
    text/html    html htm shtml;
    text/css     css;
}
```

下面就分析一下 ngx_conf_parse() 的源码，因为其中的一个 cf->handler 让我费劲理解了很久。。。

首先，函数的主体操作流程如下：

```
ngx_conf_parse() {
    for ( ;; ) {
        rc = ngx_conf_read_token(cf);

        if (cf->handler) { // 类似 type 这样的命令设置自定义处理函数
            rv = (*cf->handler)(cf, NULL, cf->handler_conf);
        }
        //读到的 include，http，deamon，type 指令都在这里找处理函数
        rc = ngx_conf_handler(cf, rc); 
    }
}
```

在大多数情况下，cf->handler 的值都是为空的，这样比如读到 http 指令，那么会直接到 ngx_conf_handler()，然后查找处理函数。

但是比如读到了 type 命令，它也是先进入 ngx_conf_handler() 找到自己的处理函数，但是**该处理函数会设置 cf->handler**，导致 type 命令下的子命令被读取后会在 handler 处被处理。

眼见为实，我们可以用 printf 打出一些 log

```
before read token cf->handler = (nil)
after read token cf->handler = (nil)
cmd = types, nelts = 1, cf->handler = (nil)

before read token cf->handler = 0x559b461ddd9b
after read token cf->handler = 0x559b461ddd9b
cmd = text/html, nelts = 4, cf->handler = 0x559b461ddd9b
```

## ngx_http_block() 

在 ngx_conf_parse() 中被调用，解析 `http{}` 配置块，只有出现这个 http 指令才会在`conf_ctx` 里创建http配置，避免内存浪费。

同时调用每个http模块的 create_xxx_conf() 函数，创建配置结构体。














