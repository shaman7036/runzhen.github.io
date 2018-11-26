---
layout: post
comments: no
title: "nginx HTTP 配置项的解析"
category: "nginx"
tags: [nginx]
---

* table
{:toc}
***

在前面的两篇博客中我们看到，无论是实现一个 http 模块，或者是 http filter 模块，都需要实现模块自己的 ngx_http_module_t 结构体。

```
typedef struct {
    ngx_int_t   (*preconfiguration)(ngx_conf_t *cf);
    ngx_int_t   (*postconfiguration)(ngx_conf_t *cf);

    void       *(*create_main_conf)(ngx_conf_t *cf);
    char       *(*init_main_conf)(ngx_conf_t *cf, void *conf);

    void       *(*create_srv_conf)(ngx_conf_t *cf);
    char       *(*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);

    void       *(*create_loc_conf)(ngx_conf_t *cf);
    char       *(*merge_loc_conf)(ngx_conf_t *cf, void *prev, void *conf);
} ngx_http_module_t;
```

其中 main、srv、loc 分别对应 nginx.conf 中的 http，server，location 配置块，本文就来关注一下这些配置项是如何被解析和使用的。

# 解析 http 不同级别配置项

一个简单的 nginx.conf 配置如下：

```
http {
    test_cmd;
    server {
        listen       80;
        test_cmd;
        location / {
            test_cmd;
            root   html;
            index  index.html index.htm;
        }
    }
}
```

可以看到，在 http，server，location 三个配置块中都有我们自定义模块的命令 test_cmd。

在 http 框架处理到某个阶段时，例如在寻找到合适的 location 钱，如果试图去取某个模块的配置结构体，将会得到 server 级别的配置，而如果寻找到 location 之后，就会得到 location 结构下的配置。

## main 级别配置项

## server 级别配置项

## location 级别配置项

# 合并配置项