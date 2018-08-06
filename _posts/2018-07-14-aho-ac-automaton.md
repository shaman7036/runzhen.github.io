---
layout: post
comments: yes
title: "Aho–Corasick 算法，AC 自动机"
category: "algorithm"
tags: [algorithm, aho, ac, automaton]
---

AHO 算法，或者叫 AC 自动机、又或者叫 Aho–Corasick string matching algorithm，是一个高效的多模式匹配算法，它的特点是可以同时匹配多个模式串。

一般使用 AC 自动机需要以下三步：

- 根据待匹配的字符串 P1, P2, Pn 建立 Trie
- 给 Trie 添加失败路径，实际上是生成了一个自动机。
- 将输入文本 str 的字符逐个通过自动机，在 O(n) 的时间复杂度内找出 P1, P2 ... Pn 是否存在于 str 内。


## 举例

假设我们有模式字符串 { `fat` `fare` `hat` `are` }， 输入的文本为 “fatehatfare”。

### 建立 Trie

首先根据模式串建立起一个 trie，一般来说每个节点的结构大概是这样：

```
Node {
	Node * children[26]; /* 指向子节点的指针 */
	Node * parent;       /* 指向父节点的指针 */ 
	Node * fail;         /* 失败指针 */
	bool   terminate;    /* 是否是一个终结节点，即一个串的最后一个字符 */
}
```

![aho1](/image/2018/aho1.jpg){:height="300" width="300"}

### 给每个节点添加失败指针

这是非常核心的一步，大概的思路是这样的。

> 构造失败指针的方法：假设当前节点上的字母为 ‘a’，沿着父节点的失败指针走，直到走到一个节点，他的子节点中也有字母为 'a' 的节点。然后把当前节点的失败指针指向那个子节点。如果一直走到了root都没找到，那就把失败指针指向root。


那么再具体一点，


![aho1](/image/2018/aho2.jpg){:height="300" width="300"}