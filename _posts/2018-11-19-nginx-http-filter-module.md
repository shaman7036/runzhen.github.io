---
layout: post
comments: yes
title: "nginx HTTP Filter 模块"
category: "nginx"
tags: [nginx]
---

* table
{:toc}
***

# 过滤模块基本概念

普通的 HTTP 模块和 HTTP filter 模块有很大的不同。普通模块，例如上篇博客提到的 hello world 模块，可以介入 nginx http 框架的 7 个处理阶段，绝大多数情况下介入 NGX_HTTP_CONTENT_PHASE 阶段，特点是一旦介入了，那么一个 http 请求在这个阶段将只有这个模块处理。

http filter 模块则不同，一个请求可以被任意个 http 过滤模块处理，而且过滤模块 **仅处理服务器发出的 HTTP 响应 header 和 body，不处理客户端发来的请求**。


## 过滤链表的顺序

编译 nginx 的第一步是执行 configure 脚本生成 objs/ngx_modules.c 文件，这个文件中的 ngx_modules 数组会保存所有的 nginx 模块，包括普通的 http 模块和本文介绍的 http filter 模块。

nginx 在启动时初始化模块的顺序就是 nginx_modules 数组成员的顺序。因此，只要看 configure 命令生成的 ngx_modules.c 文件就可以知道所有 http 过滤模块的顺序。

> 对于 http 过滤模块来说，在 ngx_modules 数组中的位置越靠后，实际执行请求时就越先执行。因为在初始化 http 过滤模块时，每个过滤模块都是将自己插入到整个链表的首部。

# 开发一个 HTTP 过滤模块

## 定义这个过滤模块

参考上一篇博客，基本的套路还是一样，只不过这次 ctx 成员需要定义 postconfiguration / create_loc_conf / merge_loc_conf 

```
static ngx_http_module_t ngx_http_myfilter_module_ctx = {
    NULL,                           /* preconfiguration */
    ngx_http_myfilter_init,         /* postconfiguration */
    NULL,                           /* create main configuration */
    NULL,                           /* init main configuration */
    NULL,                           /* create server configuration */
    NULL,                           /* merge server configuration */
    ngx_http_myfilter_create_conf,  /* create location configuration */
    ngx_http_myfilter_merge_conf    /* merge location configuration */
};
```

此外，command 成员添加一个命令 “myfilter”，所以在配置文件中可以用它开启或者关闭这个过滤模块，因为有了“开启/关闭” 这个变量，因此我们需要解析这个配置项，并把它保存到过滤模块私有的数据结构中。

```
typedef struct {
    ngx_flag_t enable;
} ngx_http_myfilter_conf_t;


static ngx_command_t ngx_http_myfilter_commands =  {
        ngx_string("myfilter"),
        NGX_HTTP_LOC_CONF|NGX_HTTP_LMT_CONF|NGX_CONF_FLAG,
        ngx_conf_set_flag_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_myfilter_conf_t, enable),
}
```

## 初始化过滤模块

所谓初始化过滤模块，就是把这个 filter 模块插入到 nginx 本身自带的 filter 模块链中一个合适的位置。

```
static ngx_int_t ngx_http_myfilter_init(ngx_conf_t *cf)
{
    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_myfilter_header_filter;

    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter = ngx_http_myfilter_body_filter;

    return NGX_OK;
}
```

其中 ngx_http_myfilter_header_filter() 和 ngx_http_myfilter_body_filter() 就是这个过滤模块**真正做事情的部分**。

## 处理配置项的 ctx 

既然在 nginx.conf 里添加了配置项，那么就需要把它整合到 nginx 中去，`ngx_http_myfilter_module_ctx` 中定义的函数正是这个作用，这一步是要实现他们。

`create_conf` 函数用于分配空间存储配置项结构体 ngx_http_myfilter_conf_t 

```
static void* ngx_http_myfilter_create_conf(ngx_conf_t *cf)
{
    ngx_http_myfilter_conf_t *mycf;
    mycf = (ngx_http_myfilter_conf_t *)ngx_pcalloc(cf->pool, sizeof(ngx_http_myfilter_conf_t));

    mycf->enable = NGX_CONF_UNSET;
    return mycf;
}
```

`merge_conf` 函数定义如何合并配置项。
```
static char *ngx_http_myfilter_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_myfilter_conf_t *prev = (ngx_http_myfilter_conf_t *)parent;
    ngx_http_myfilter_conf_t *conf = (ngx_http_myfilter_conf_t *)child;

    ngx_conf_merge_value(conf->enable, prev->enable, 0);

    return NGX_CONF_OK;
}
```

## 实现过滤模块的主体函数

真正干事情的函数是 `ngx_http_myfilter_header_filter` 和 `ngx_http_myfilter_body_filter`



# 参考资料
- 《深入理解 nginx》




