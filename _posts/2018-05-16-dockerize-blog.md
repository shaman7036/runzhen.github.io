---
layout: post
comments: true
title: "容器化博客的编译环境"
category: Docker
tags: [docker, dockerize]
---

### 前言

在上一篇博客 [博客十年的变迁: 从 Wordpress 到 Jekyll](/2018/03/wordpress-to-jekyll/) 中，提到了把博客从原来的 Workpress 迁移到了 Jekyll, 开始用 Markdown 语法写博客正文，用 `jekyll build` 生成博客内容并部署到 Web 服务器。

最近这两天在考虑把 "jekyll build" 这个过程容器化，达到传说中 **一次部署到处运行** 的终极目标。


### 目标

废话不多说，开始折腾。 首先要制定一下具体的目标：

- jekyll build 是将 Markdown 语法写成的纯文本文件生成 html 文件，这也是我希望将她容器化的部分，因为我并不希望再次搭建 jekyll 的环境，只要有一个稳定能用的编译环境就可以了。

- 生成了 html 文件以后，拷贝到 nginx 的目录下，这样就可以通过浏览器访问博客内容了，这是第二各部分。而我也在学习 nginx 的源码，希望时不时的能把我修改过的 nginx 部署到博客上，因此，`不容器化 nginx` 。 

### 步骤

首先去 [docker hub](https://hub.docker.com/r/runzhen/) 上找了一个可用的镜像 jekyll/jekyll，这个镜像已经部署好了一个基本的 jekyll 环境。

```
docker pull jekyll/jekyll:latest
```

然后启动这个镜像并进入到容器中 (这一步如果非常清楚需要安装哪些软件的话可以用 Dockerfile 代替完成。)

```
docker run --rm --volume=/LOCAL_DIR:/srv/jekyll -it jekyll/jekyll  /bin/bash
```

安装一些自己的博客需要额外组件。

最后在容器外用 docker 命令 commit，生成一个新的 image 
```
docker commit CONTAINER runzhen/blog:v1
```

这样，一个编译环境就搭建完成了。以后每次写完博客生成新的 html 的时候，只要运行一下 `dcoker run`，再用一个简单的 Makefile 配合完成部署到 nginx，非常方便。

Makefile 如下：

```
DST := /usr/local/nginx/html/

all:
	rm -rf $(DST)
	docker run --rm --volume="$(CURDIR):/srv/jekyll" --volume="$(DST):/tmp" runzhen/blog:v2

build:
	docker build -t runzhen/blog:v2 .
```






