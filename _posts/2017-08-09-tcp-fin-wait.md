---
layout: post
comments: yes
title: "TCP 的 FIN_WAIT1 状态"
category: "linux"
tags: [linux, tcp]
---

* table
{:toc}

最近看到了一篇有关 TCP 关闭连接时 FIN-WAIT1 状态的文章（见参考资料），觉得很有意思，于是也在自己的电脑上验证了一下。

首先，开局一张图：

![](/image/2017/tcpclose.png)

FIN-WAIT1 状态出现在 `主动关闭链接方`发出 FIN 报文后，收到对应 ACK 之前。通常 Server 在收到 FIN 报文之后，会在很短的时间内回复 ACK（这个 ACK 可能携带数据，也可能只是一个纯 ACK），所以 FIN-WAIT1 状态存在的时间非常短暂，很难被观察到。

于是准备两台虚拟机，我们可以设计这样一个实验：

1）服务端监听 1234 端口：nc -l 1234

2）客户端连接服务端：nc 192.168.122.183 1234， 此时 TCP 的状态为 **ESTABLISHED**

```
$ sudo netstat -anp | grep tcp
tcp        0      0 192.168.122.167:60482   192.168.122.183:1234    ESTABLISHED 3712/nc
```

3）服务端配置 iptables，拦截从服务端发送到客户端的任何报文：`iptables -A OUTPUT -d 192.168.122.167 -j DROP`

4）客户端按下 ctrl + c 断开连接，这一步的目的是让操作系统自动发送 FIN 报文给服务端。

在完成第 4 步之后，客户端就会进入 `FIN-WAIT-1` 状态，服务端也会收到 FIN 报文，并且马上会发出一个 ACK，但是因为配置了 iptables，因此客户端会一直等待服务端的 ACK。


我们先来看一下 客户端的 TCP 状态
```
# netstat -anp | grep tcp
tcp        0      1 192.168.122.167:60482   192.168.122.183:1234    FIN_WAIT1   -
tcp6       0      0 :::22                   :::*                    LISTEN      1153/sshd
```

再来看一下此时服务端的状态，根据上面的图，此时 TCP 应该处于 LAST-ACK 状态：
```
$ sudo netstat -anp | grep tcp

tcp        0      1 192.168.122.183:1234    192.168.122.167:60482   LAST_ACK    -
```



## 参考资料

- https://huoding.com/2014/11/06/383






