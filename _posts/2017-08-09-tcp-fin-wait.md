---
layout: post
comments: yes
title: "TCP 的 FIN_WAIT1 状态"
category: "linux"
tags: [linux, tcp]
---

最近看到了一篇有关 TCP 关闭连接时 FIN-WAIT1 状态的文章（见参考资料），觉得很有意思，于是也在自己的电脑上验证了一下。

首先，开局一张图：

![](/image/2017/tcpclose.png){:height="300" width="300"}

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
```

再来看一下此时服务端的状态：

根据上面的流程图，此时 TCP 应该处于 `CLOSE-WAIT` 状态，等待上层应用告诉 TCP "我没有数据发了，请关闭连接"。

在我们这个例子中，因为应用层本身没有数据要发送，因此**服务端 FIN 报文和 ACK 一起发送**，所以 CLOSE WAIT 状态被很快跳过，直接进入 LAST-ACK 状态：
```
$ sudo netstat -anp | grep tcp

tcp        0      1 192.168.122.183:1234    192.168.122.167:60482   LAST_ACK    -
```

在客户端抓包 [tcp-fin.pcap](/image/2017/tcp-fin.pcap)，可以看到 TCP 发出 FIN，但没有接受到 ACK 之间，一直在试图重传 FIN 报文，间隔的时间 200ms，400ms 递增。

![](/image/2017/tcpclose2.png){:height="200" width="600"}


原文中还提到了 2 个系统参数 ：

1) `/proc/sys/net/ipv4/tcp_orphan_retries`

> This value influences the timeout of a locally closed TCP connection, when RTO retransmissions remain unacknowledged. See tcp_retries2 for more details.
> The default value is 8. If your machine is a loaded WEB server, you should think about lowering this value, such sockets may consume significant resources

Linux TCP协议栈 FIN 报文重传间隔 由 `tcp_orphan_retries` 控制。


2) `/proc/sys/net/ipv4/tcp_fin_timeout`

> The length of time an orphaned (no longer referenced by any application) connection will remain in the FIN_WAIT_2 state before it is aborted at the local end. While a perfectly valid “receive only” state for an un-orphaned connection, an orphaned connection in FIN_WAIT_2 state could otherwise wait forever for the remote to close its end of the connection.


## 参考资料

- [https://huoding.com/2014/11/06/383](https://huoding.com/2014/11/06/383)
- [Nginx 的 reset_timedout_connection 选项和 FIN_WAIT1](http://nginx.org/en/docs/http/ngx_http_core_module.html#reset_timedout_connection)
- http://valerieaurora.org/tcpip.html

测测看自己对 TCP [了解多少](http://valerieaurora.org/tcpip.html) ？

1）TCP 最小的端口号是多少?

2）TCP 帧中有一个叫做 URG Pointer 的字段，什么时候会用到该字段？

3）RST 包能有荷载么？

4）什么时候会用到 IPv6 里的“flow”字段？

5）socket中的 IP_FREEBIND 选项有什么用？

6）PSH 标志实际上有什么用？

7）TCP 时间戳和 SYN Cookie 是如何协同工作的？

8）“UDP” 包可以把校验和字段设置为0么？

9）TCP 的同时开放连接是如何工作的？真的能工作么？

10）什么是愚笨窗口综合征（stupid window syndrome）？

11）TCP 头里的 CWE 和 ECE 标志有什么用？

12）IP 头里的 ID 字段是什么？ID 字段必须和 DF 比特位一起完成什么工作？为什么有些 IP 包的 ID 字段不是零并且设置了 DF？

13）SYN 包可以有荷载么？（提示：新RFC提案）

14）SYN+ACK 包可以有荷载么？

15）ICMP 包太大（packet-too-big ）的消息会由路由器返回，并且荷载里包含了原始包的一部分。Linux 系统中可接受的最小荷载长度是多少？

16）当 ICMP包太大（packet-too-big ）的消息被中间路由返回时会包含这个路由的源IP。但在实际操作中，我们经常可以看到 ICMP 消息的源 IP 与原始包的目的 IP 相同。为什么会这样？

17）Linux 有一个名为 “tcp_no_metrics_save” 的 sysctl 设置。它用于存储什么？存储多久？

18）Linux 使用了两个队列来处理到达的 TCP 连接：SYN 队列和接收队列。SYN 队列长度是多少？

19）如果 SYN 队列因变得很大而导致溢出，那么会发生什么？

20）BGP bogons 是什么？为什么说它们现在只是个小问题？

21）TCP 有一个会添加 MD5 校验和到包中的扩展。该扩展什么时候起作用？

22）IPv4 和 IPv6 的校验和算法有什么区别？






