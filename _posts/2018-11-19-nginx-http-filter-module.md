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

一个简单的过滤模块实现这样的功能：在返回的 http response 中，先检查 header，如果是 200 OK，则在 body 中插入一段字符。如下所示：

```
$ curl localhost
-----add-prefix-----<!DOCTYPE html>
<html>
<head>
<title>Welcome to nginx!</title>
```

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
    mycf = (ngx_http_myfilter_conf_t *)
           ngx_pcalloc(cf->pool, sizeof(ngx_http_myfilter_conf_t));

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

真正干事情的函数是 ngx_http_myfilter_header_filter() 和 ngx_http_myfilter_body_filter()


header_filter() 所做的事情就是检查 http header 决定要不要加我们的 prefix。其中涉及到用 nginx 提供的 API `ngx_http_get_module_ctx()` 获取当前模块的上下文 ctx ； 以及 API `ngx_http_get_module_loc_conf()` 获取与本模块相关的配置项。

body_filter() 就是根据 ctx 把 prefix 加到 http body 中去，并重新计算 Content-Length 的值。

详细的源码请[移步此处](https://gist.github.com/runzhen/a7f8d12617ae752ee8a61dc070c67267)


最后别忘了关系到编译的 config 文件，过滤模块的 config 也与其他类似，也能编译成动态模块，注意要加上 “FILTER” 

```
ngx_addon_name=ngx_http_myfilter_module

if test -n "$ngx_module_link"; then
    ngx_module_type=HTTP_FILTER
    ngx_module_name=ngx_http_myfilter_module
    ngx_module_srcs="$ngx_addon_dir/ngx_http_myfilter_module.c"

    . auto/module
else
    HTTP_FILTER_MODULES="$HTTP_FILTER_MODULES ngx_http_myfilter_module"
    NGX_ADDON_SRCS= "$NGX_ADDON_SRCS $ngx_addon_dir/ngx_http_myfilter_module.c"
fi
```


# 参考资料
- 《深入理解 nginx》




