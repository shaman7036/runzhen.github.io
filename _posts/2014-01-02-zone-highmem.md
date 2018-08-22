---
layout: post
comments: yes
title: "Linux 内核在 x86-64 上的分区"
category: linux
tags: [linux, kernel]
---

如果稍微了解过 Linux 内核的内存管理，那么对内存分区的概念一定不陌生，Linux内核把物理内存分成了3个区，

1. 0 – 16M 为ZONE_DMA区,
2. 16M – 896M 为ZONE_NORMAL区，
3. 高于896M 为ZONE_HIGHMEM区

我没有去考证过为什么要取896这个数字，但是可以肯定的是这样的划分在当时看来是合理的，然而计算机发展今非昔比，现在4G的物理内存已经成为PC的标配了，CPU也进入了64位时代，很多事情都发生着改变。

在CPU还是32位的时代，CPU最大的物理寻址范围是0-4G， 在这里为了方便讨论，我们不考虑物理地址扩展（PAE）。进程的虚拟地址空间也是 4G，Linux内核把 0-3G虚拟地址空间作为用户空间，3G-4G虚拟地址空间作为内核空间。

目前几乎所有介绍Linux内存管理的书籍还是停留在32位寻址的时代，所以大家对下面这张图一定很熟悉！
（这个图画得非常详细，本篇文章我们关注的重点是 3个分区 以及最右边的线性地址空间，也就是虚拟地址空间之间的关系，另外，应该是ZONE_DMA， ZONE_NORMAL, ZONE_HIGHMEM, 图中把ZONE 写成了ZUNE）

![zone1](/image/2014/zone1.gif)

 
然而，现在是64位的时代了, 64位CPU的寻址空间是多大呢？ `16EB， 1EB = 1024 TB = 1024 * 1024 GB`，我想很多人这辈子还没见过大于1TB的内存吧，事实上也是这样，几乎没有哪个服务器能有16EB的内存，实现64位长的地址只会增加系统的复杂度和地址转换的成本，所以目前的`x86_64`架构CPU都遵循AMD的 Canonical Form, `即只有虚拟地址的最低48位才会在地址转换时被使用, 且任何虚拟地址的48位至63位必须与47位一致`, 也就是说总的虚拟地址空间为**256TB**。


那么在64位架构下，如何分配虚拟地址空间的呢？

1. 0000000000000000 – 00007fffffffffff(128TB)为用户空间,
2. ffff800000000000 – ffffffffffffffff(128TB)为内核空间。

而且内核空间中有很多空洞, 越过第一个空洞后, `ffff880000000000 – ffffc7ffffffffff(64TB)` 才是直接映射物理内存的区域, 也就是说默认的PAGE_OFFSET为 ffff880000000000.

请关注下图的最左边，这就是目前64位的虚拟地址布局。

![](/image/2014/x86mem.png)

在本文的一开头提到的物理内存分区 ZONE_DMA， ZONE_NORMAL， ZONE_HIGHMEM 就是与内核虚拟地址的直接映射有关的，如果读者不了解 内核直接映射物理地址这个概念的话，建议你去google一下，这个很简单的一一映射的概念。

既然现在内核直接映射的物理内存区域有64TB， 而且一般情况下，极少有计算机的内存能达到64TB（别说64TB了，1TB内存的也很少很少），所以整个内核虚拟地址空间都能够一一映射到计算机的物理内存上，因此，不再需要 ZONE_HIGHMEM这个分区了，现在对物理内存的划分，只有ZONE_DMA， ZONE_NORMAL。


如果你想有个更加直观的了解的话，请打开 /boot/config*** 文件，例如我的是  /boot/config-3.11.0-031100-generic  ，如果你现在用的CPU是64位，并且Linux发行版也是64位的，那么 在这个配置文件中找不到 CONFIG_HIGHMEM 这个选项，而如果您的CPU是32位的，则一定会有 CONFIG_HIGHMEM 这个选项。


### 参考资料

- [http://adam8157.info/blog/2012/07/linux-x86-64-vm](http://adam8157.info/blog/2012/07/linux-x86-64-vm)
- [https://en.wikipedia.org/wiki/X86-64](https://en.wikipedia.org/wiki/X86-64)