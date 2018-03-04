---
layout: post
comments: true
title: "博客十年的变迁: Wordpress 到 Jekyll"
category: 无分类
tags: [blog]
---


### 前言

印象中我的第一博客是在大约2007年左右创建的，那个时候是个人博客非常流行的几年，各个网站都推出了自己的博客服务，如今已经是2018年，转眼间10年过去了，个人博客不如以前那么盛行了，现在流行的是微博自媒体和微信公众号。但是作为一个程序员，总觉得还是自己的博客比较 “geek”，所以一直还在折腾。

一开始的博客是建立在《电脑爱好者》网站推出的“博墅”服务上的，跟所有提供博客服务的网站一样，用户只要在网上点几下鼠标博客就建立好了。

然后就折腾更加高级的：自己买域名，买主机空间，自己建数据库，自己安装博客程序（也就是当时最流行的博客程序 WordPress）。

Wordpress 是我目前用的最长的博客系统了，从2009年左右一直到现在2018年，当初买的域名，买的主机空间，一直续费到现在，**算一算有近10年了**。

现在，准备把 wordpress 博客停用了，换成更 “geek” 的方式，用 **Markdown** 语法写内容，用 **Jekyll** 程序生成博客。生成的博客可以托管在 github pages 上，也可以放在云主机上。


### 博客源代码

博客的源码已经放在了我的 github 上，通过 github pages 服务这个博客已经可以运行，博客的地址是 `https://username.github.io`

但是还是想在自己的云主机上也创建一个（太喜欢折腾了...），因此同样的一个博客内容在 xxx.github.io 上和云主机上各有一份，我的博客域名 	`blog.nlogn.cn` 随意指向其中一个（看我心情咯）。 


### Ubuntu 部署 Jekyll

1. 安装 ruby 

`sudo apt install ruby-full ruby-bundler` 

2. 安装 jekyll

`sudo gem install jekyll`

3. git clone github.io 上我的博客。

4. jekyll build 博客

`jekyll build --destination /usr/local/nginx/html/myblog`  自动将生成的博客部署到指定目录下。至此，一个静态博客就部署完成了。

云主机上我用的是自己编译的 nginx 服务器，通过设置 nginx 的配置文件，指定一个针对域名 "blog.nlogn.cn" 的虚拟主机，并把博客网站根目录指定为 "html/myblog" 就 OK 了。





