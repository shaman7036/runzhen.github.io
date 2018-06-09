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


## 参考资料

[Go 并发、管道](http://www.woola.net/detail/2017-04-27-goroutines.html)

