---
layout: post
comments: true
title: Raft 一致性算法
category: 分布式系统
tags: [paper, distribution]
---

论文原文链接 [In Search of an Understandable Consensus Algorithm](/papers/raft-extended.pdf)


Raft 保证一致性的 4 个重要组成部分： **领导人的选举**，**日志复制**，**安全性**，**集群成员的变化**。

Raft 是一个强领导者算法，意思就是所有的数据以当前领导者（Leader）上的为准，领导者上有的就是正确的，领导者上没有的就是错误的。

## 5.1 Raft 的基本术语和概念

任期的概念：任期(term) 用来区分时间轴上不同的领导者，当领导者换了，日期也也立刻变更。raft 算法用 term 来表示一个 `逻辑时钟`，而不是用传统的几月几日几点几分几秒，term 是一个递增的整数，当领导者变了，term 也 +1。

集群中的每一个服务器都存储了一个 currentTerm，任意两个 server 要相互通信时都需要包含各自的 currentTerm。如果一个 server 的 currentTerm 值小于对方，那么它需要更新自己的值。如果 candidate 或者 leader 发现他们自己的 currentTerm 比其他人的要小，那个立马变成 follower。如果收到的请求包含一个旧的 currentTerm，直接忽略。

原论文的图 2 包含了大量的信息，我自己制作了下面四个图片。

![状态](/image/2018/raft1.png)

![请求投票 RPC](/image/2018/raft2.png)

![附加日志条目 RPC](/image/2018/raft3.png)

![所有服务器的规则](/image/2018/raft4.png)

## 5.2 领导人的选举

领导人不断的向集群中其他人发送心跳包，当集群中一个 server 在一段时间里没有收到领导人的心跳包，那么他就要起义，自己竞选当领导人。

在竞选的过程中， candidate 一直保持着 candidate 的身份知道下面三个事件中的任何一个发生：

1. 它赢得了选举，成为 leader
2. 其他人赢得了选举
3. 一段时间过去了，没有人成为 leader （大家都在选举，没有人得票数过半），所以准备重新开始。

在一个特定的 term 内，server 只会给一个人投票，先来先到。也就是说，比如 term=100，serverA 投给了 B，那么它不会再投票给任何人，在下一个 term=101，他才可以继续投票。

在 candidate 发起了投票等待回应的过程中，它有可能会收到其他 server 发来的 AppendEntries RPC，（这个 RPC 只能是 leader 发，但是这个 server 认为自己是 leader），这时，candidate 比较 currentTerm，如果比自己的大，就认输了，如果比自己的小，直接拒绝。

#### 开始一次选举

Follower 把自己的 term+1，身份转变成 Candidate，向集群中其他 server 发起请求投票 RPC，当他获得大多数选票时，就成功当选。

> 问题：他怎么知道 “大多数” 是多少？

每个 server 只会对一个 termId 投出一张票。   

如果 Candidate 在竞选中收到一个自称是 Leader 的 心跳包 RPC，他先查看 termId，如果这个数字比他自己发起的大，那么他立刻承认这个人是集群中现在的 leader，然后退出选举。


## 5.3 日志复制


## 5.4 安全性


## 5.5 集群成员的变化

