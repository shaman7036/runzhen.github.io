---
layout: post
comments: true
title: Raft 一致性算法
---

论文原文链接 [In Search of an Understandable Consensus Algorithm](/papers/raft-extended.pdf)


Raft 保证一致性的 4 个重要组成部分： **领导人的选举**，**日志复制**，**安全性**，**集群成员的变化**。

Raft 是一个强领导者算法，意思就是所有的数据以当前领导者（Leader）上的为准，领导者上有的就是正确的，领导者上没有的就是错误的。

任期的概念：任期(term) 用来区分时间轴上不同的领导者，当领导者换了，日期也也立刻变更。raft 算法用 term 来表示一个 `逻辑时钟`，而不是用传统的几月几日几点几分几秒，term 是一个递增的整数，当领导者变了，term 也 +1。


## 5.2 领导人的选举

领导人不断的向集群中其他人发送心跳包，当集群中一个 server 在一段时间里没有收到领导人的心跳包，那么他就要起义，自己竞选当领导人。

#### 开始一次选举

Follower 把自己的 term+1，身份转变成 Candidate，向集群中其他 server 发起请求投票 RPC，当他获得大多数选票时，就成功当选。

> 问题：他怎么知道 “大多数” 是多少？

每个 server 只会对一个 termId 投出一张票。   

如果 Candidate 在竞选中收到一个自称是 Leader 的 心跳包 RPC，他先查看 termId，如果这个数字比他自己发起的大，那么他立刻承认这个人是集群中现在的 leader，然后退出选举。


## 5.3 日志复制


## 5.4 安全性


## 5.5 集群成员的变化

