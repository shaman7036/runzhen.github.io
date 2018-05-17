---
layout: post
comments: true
title: "Elasticsearch 从安装到使用"
category: "Distributed System"
tags: [elastic]
---

## 安装

首先需要准备一台 Linux 主机，我选用的是 DigitalOcean 的 Ubuntu 云主机，然后按照[这个配置步骤](https://www.digitalocean.com/community/tutorials/how-to-install-and-configure-elasticsearch-on-ubuntu-16-04) 很快就让 Elasticsearch 运行起来。

总的来说，Elasticsearch 的安装非常简单，需要注意以下几点：

- 先安装 JAVA，配置 JAVA_HOME。见参考资料1。
- 按照教程说的下载 deb 安装包的方式最靠谱。之前尝试过下载 Zip 包安装，不如前者简单。
- 内存要大。最开始用的是 512MB 内存的云主机，内存太小安装失败，怒加到2G。
- 不要用 root 用户，elasticsearch 不允许用 root 安装。要新建一个用户，并且给 root 权限。不知道为什么有这么奇葩的要求。


## 玩转

Elasticsearch 启动后监听 9200 端口，并且提供所谓的 RESTFul API，意思是你只要用几个 http request，就能对它进行一系列操作。

构造 http request 的工具很多，甚至可以自己用各种脚本语言。一般来说 `curl` 是一个非常强大的工具，可以快速构造 http 请求头，本文中我用了另外一个工具 `http`，ubuntu 用户可以直接用 `apt install httpie` 安装。 

一切准备就绪，先来看一下 ES 的状态：

```
elastic@ubuntu:~$ http GET localhost:9200/_cluster/health
HTTP/1.1 200 OK
content-encoding: gzip
content-length: 223
content-type: application/json; charset=UTF-8

{
    "active_primary_shards": 0,
    "active_shards": 0,
    "active_shards_percent_as_number": 100.0,
    "cluster_name": "elasticsearch",
    "delayed_unassigned_shards": 0,
    "initializing_shards": 0,
    "number_of_data_nodes": 1,
    "number_of_in_flight_fetch": 0,
    "number_of_nodes": 1,
    "number_of_pending_tasks": 0,
    "relocating_shards": 0,
    "status": "green",
    "task_max_waiting_in_queue_millis": 0,
    "timed_out": false,
    "unassigned_shards": 0
}

```

可以看到返回了一大堆结果，不过其他的先不管，目前我只关心 "status"， 它是 “green”， 意味着整个 Elasticsearch 集群状态良好。


## 参考资料
1. [安装 java，配置 JAVA_HOME](https://www.digitalocean.com/community/tutorials/how-to-install-java-with-apt-get-on-debian-8)
2. [how-to-install-elasticsearch](https://www.digitalocean.com/community/tutorials/how-to-install-and-configure-elasticsearch-on-ubuntu-16-04)

