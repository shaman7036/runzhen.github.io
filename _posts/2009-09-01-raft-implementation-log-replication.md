---
layout: post
comments: true
title: "Raft 算法的简单实现"
category: "Distributed System"
tags: [raft]
---

在实现的时候发现这张状态转换图非常重要。整个 raft 的运行就是围绕着这张图发生一系列状态转换。
因此，第一步实现一个状态机就成了关键。

![状态转换图](/image/2018/raft6.png)


# 日志的复制

一旦选举成功，所有的 Client 请求最终都会交给 Leader 处理。

当 Client 请求到 Leader后，Leader 首先将该请求转化成 LogEntry，然后添加到自己的 log[] 中，并且把对应的 index 值存到 LogEntry 的 index 中，**这样 LogEntry 中就包含了当前 Leader 的 term 信息和在 log[] 中的index信息**，这两个信息在发给 Follower 以后会用到。

> 所以一旦一个节点成为 Leader 以后，那么它的 log[] 保存的这一组 LogEntry 就代表了整个集群中的最终一致的数据。用 raft 论文的话来说就是，节点是一个状态机，LogEntry 是指令集，任何一个节点，只要逐个执行这一串指令，最后状态机的状态都一样。


![](/image/2018/raft2-1.jpg)

1. firstLogIndex/lastLogIndex标识当前日志序列的起始位置
2. commitIndex 表示当前已经提交的日志，也就是成功同步到集群中日志的最大值
3. applyIndex是已经apply到状态机的日志索引，它的值必须小于等于commitIndex，因为只有已经提交的日志才可以apply到状态机


先看看与日志复制相关的数据结构成员。

```
type Raft struct {
	....
    log         []LogEntry

    commitIndex int 
    lastApplied int 

    nextIndex  []int 
    matchIndex []int 
    .....
}
```


```
type AppendEntriesRequest struct {
    Term         int32
    LeaderID     int
    PrevLogIndex int
    PrevLogTerm  int32
    Entries      []LogEntry
    LeaderCommit int
}
```

```
type AppendEntriesReply struct {
    Term          int32
    HeartBeat     bool
    Success       bool
    ConflictIndex int
    ConflictTerm  int32
}
```


对于每个follower，leader保持2个属性，一个是nextIndex即leader要发给该follower的下一个entry的index，另一个就是matchIndex即follower发给leader的确认index。

一个leader在刚开始的时候会初始化：
`
nextIndex[x] = leader的log的最大index+1
matchIndex[x] = 0
`





















## 参考资料

[Go 并发、管道](http://www.woola.net/detail/2017-04-27-goroutines.html)

