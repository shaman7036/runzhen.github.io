---
layout: post
comments: yes
title: "Aho–Corasick 算法，AC 自动机"
category: "algorithm"
tags: [algorithm, aho, ac, automaton]
---

*  目录
{:toc}
***

AHO 算法，或者叫 AC 自动机、又或者叫 Aho–Corasick string matching algorithm，是一个高效的多模式匹配算法，它的特点是可以同时匹配多个模式串。

最常见的应用就是`病毒扫描` : 把所有病毒的特征码（类似一段字符串）构造成一个 AC 自动机，把用户的文件或者网络数据流作为输入文本，只要在输入文本中找到了任何一个特征码，那么就表示有病毒存在。

对于这样的一个应用场景，我们最希望的功能就是`只扫描一遍输入文本，找出所有的病毒`，而 AC 自动机恰恰就有这样的能力。


一般使用 AC 自动机需要以下三步：

1. 根据待匹配的字符串 P1, P2, Pn 建立 Trie
2. 给 Trie 添加失败路径，实际上是生成了一个自动机。
3. 将输入文本 str 的字符逐个通过自动机，在 O(n) 的时间复杂度内找出 P1, P2 ... Pn 是否存在于 str 内。


## 举例

假设我们有模式字符串 { `fat`,  `fare`,  `hat`,  `are` }， 输入的文本为 “fatehatfare”。

显然，所有的模式串都出现在了我们的输入文本中。我们的目标就是只扫描一遍文本串，找出所有的模式串。

## 建立 Trie

首先根据模式串建立起一个 trie，一般来说每个节点的结构大概是这样：

```
Node {
	Node * children[26]; /* 指向子节点的指针 */
	Node * parent;       /* 指向父节点的指针 */ 
	Node * fail;         /* 失败指针 */
	bool   terminate;    /* 是否是一个终结节点，即一个串的最后一个字符 */
}
```

Trie 建立好之后大概是这个样子：

![aho1](/image/2018/aho1.jpg){:height="300" width="300"}


## 给每个节点添加失败指针

这是非常核心的一步，整体的思路是这样的。

> 构造失败指针的方法：假设当前节点上的字母为 ‘a’，沿着父节点的失败指针走，直到走到一个节点，他的子节点中也有字母为 'a' 的节点。然后把当前节点的失败指针指向那个子节点。如果一直走到了root都没找到，那就把失败指针指向root。


那么再具体一点，有以下几个步骤。

- 对于整个 Trie 树，使用`广度优先搜索 BFS` 来处理每个节点。
- root 节点的子节点特殊处理， 先将 root 的子节点的失败路径直接指向 root 节点。


算法结构：

1. 先把 root 节点加入队列。
2. 进入 while 循环，直到 队列为空
3. 取队列头元素，构造它的失败指针
4. 再将它的子节点加入队列


![aho1](/image/2018/aho2.jpg){:height="300" width="300"}

一个添加了失败指针的 Trie 树如上图所示。

## 在输入文本中搜索

- 从root节点开始，根据输入文本的字符沿着自动机向下移动。
- 读入的字符在当前节点没有指向的子节点时，走失败路径。
- 如果走失败路径走到了root节点，则跳过该字符，处理下一个字符。
- 如果当前节点有终结符，则表明命中了一个模式串。（这一步可以优化为是否最长匹配）

## 实现

最后给出一个我自己的 Go 语言实现，最近学习了一下 Golang, 顺便拿来练练手。

[aho.go](/image/2018/aho.go)


一个简单的 test 代码检验结果是否正确。

```
func TestBasic(t *testing.T) {
    trie := New()
    trie.Add("fat", 1)
    trie.Add("fare", 1)
    trie.Add("hat", 1)
    trie.Add("are", 1)

    trie.AhoBuilder()

    trie.Find("fatehatfare")
}
```


## AHO 的优化

优化要么就是时间，要么就是空间。 Trie 的实现就有链表指针方式，三数组实现，双数组实现，HAT，burst trie等。

具体可以参考下面列出的资料，等我有空再来深入研究 :-)

- [前缀树匹配(Double Array Trie)](https://turbopeter.github.io/2013/09/02/prefix-match/)
- [Trie树的数组实现原理](http://blog.jqian.net/post/trie.html)
- [AC 自动机在 Tengine 中的应用](http://blog.aka-cool.net/blog/2013/08/03/aho-corasick-automaton-in-tengine/)
- https://www.cnblogs.com/xudong-bupt/p/3433506.html


