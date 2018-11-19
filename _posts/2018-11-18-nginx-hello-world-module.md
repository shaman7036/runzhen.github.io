---
layout: post
comments: yes
title: "nginx 模块开发入门"
category: "nginx"
tags: [nginx]
---

* table
{:toc}
***

想要学习如何开发一个 nginx 模块，最快速简单的方法莫过于写一个 Hello World 模块，没错，还真有这么一个 nginx-hello-world-module，而且 nginx.org 官网还介绍了这个模块。

首先，对于所有的 nginx 模块来说，都需要实现一个 `ngx_module_t` 结构体，如下所示，需要特别注意的是，如果去看 module 结构体的定义，它与下面的代码并不是一一对应的，这是因为 `NGX_MODULE_V1` 宏把其他变量都赋值了，帮我们屏蔽了一些细节。 总而言之，开发一个 nginx 模块，我们跟着这个套路走就行了。

```
ngx_module_t ngx_http_hello_world_module = {
    NGX_MODULE_V1,
    &ngx_http_hello_world_module_ctx, /* module context */
    ngx_http_hello_world_commands, /* module directives */
    NGX_HTTP_MODULE, /* module type */
    NULL, /* init master */
    NULL, /* init module */
    NULL, /* init process */
    NULL, /* init thread */
    NULL, /* exit thread */
    NULL, /* exit process */
    NULL, /* exit master */
    NGX_MODULE_V1_PADDING
};
```

其中第一个变量和最后一个变量都是固定的，我们不需要关心。如果开发的是 HTTP 模块，那么 module type 那儿写上 HTTP 的宏就行了，都是固定死的操作。

剩下来最重要的是设置 ctx 和 command (directives) 成员，command 成员，顾名思义，就是定义这个模块新增哪些 nginx.config 文件中的指令，比如 listen，include 等。

在我们这个 hello world 模块中，新增了一个 hello_world 指令，该指令不接受任何参数（`NGX_CONF_NOARGS`）。

```
static ngx_command_t ngx_http_hello_world_commands[] = {

    { ngx_string("hello_world"), /* directive */
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS, /* location context and takes
                                            no arguments*/
      ngx_http_hello_world, /* configuration setup function */
      0, /* No offset. Only one context is supported. */
      0, /* No offset when storing the module configuration on struct. */
      NULL},

    ngx_null_command /* command termination */
};
```

接下来我们看到一个 configuration setup function， `ngx_http_hello_world`，这个函数意味着，当在 nginx.config 文件中，某个配置块中出现了 hello_world 指令时，nginx 将会调用 `ngx_http_hello_world()` 函数。

因为这个模块非常简单，我们就顺便看一眼这个函数。

```
static char *ngx_http_hello_world(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf; /* pointer to core location configuration */

    /* Install the hello world handler. */
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_hello_world_handler;

    return NGX_CONF_OK;
} 
```

说完了 command 成员，现在来说说 ctx 成员，这两个成员是一个 nginx 模块必须定义的。我们先看一下 hello world 模块是怎么定义 ctx 的。

```
static ngx_http_module_t ngx_http_hello_world_module_ctx = {
    NULL, /* preconfiguration */
    NULL, /* postconfiguration */

    NULL, /* create main configuration */
    NULL, /* init main configuration */

    NULL, /* create server configuration */
    NULL, /* merge server configuration */

    NULL, /* create location configuration */
    NULL /* merge location configuration */
};
```

ctx 的作用是当 nginx 启动，http 框架开始初始化时，把本模块的配置和 http{} server{} location{} 的配置整合使用的。hello world 的指令作为非常简单，仅仅只是返回一个字符串，不需要复杂的操作，因此全部为 NULL。 稍后我们会看一个复杂一点的例子。

最后就剩下 `ngx_http_hello_world()` 中的 `ngx_http_hello_world_handler()`函数了。它是告诉 nginx，在 HTTP 处理的 11 个阶段中，如果碰到了 hello_world 指令该如何处理，也就是我们这个模块真真做事情的部分。具体的实现大家可以自己下载源码看看，非常简单就不细说了。




# 参考资料
- [https://www.nginx.com/blog/compiling-dynamic-modules-nginx-plus/](https://www.nginx.com/blog/compiling-dynamic-modules-nginx-plus/)
- [https://www.cnblogs.com/tinywan/p/6965467.html](https://www.cnblogs.com/tinywan/p/6965467.html)
- [https://github.com/perusio/nginx-hello-world-module](https://github.com/perusio/nginx-hello-world-module)




