---
layout: post
comments: yes
title: "Wireshark/tcpdump 抓到的数据包可信吗？"
category: "Linux"
tags: [linux]
---

* table
{:toc}
***

# 问题的表面现象

问题的背景是这样的：

> 一个应用程序监听某端口的 UDP 包，发送者发送的 UDP 包比较大，有 65535 个字节。显然，发送者的 UDP 包在经过 IP 层时，会被拆分层多个 1460 字节长度的 IP 分片，经过网络传输之后到达接收方，接收方的网卡收到包后，在内核协议栈的 IP 层又将分片重新组合，生成打的 UDP 包给应用程序。


正常情况下，应用程序能准确无误的收到大 UDP 包，偶尔系统网络流量十分巨大的时候，会丢个别 UDP 包 ——这都在允许范围内。

突然有一天，即便网络流量不是很大的时候，UDP 包的丢包十分严重，应用程序几乎收不到任何数据包。


## 开始 debug

遇到这样的问题，第一反应有两种可能：
- 网络不通，数据包没送到网卡。
- 个别 IP 分片在传输中丢失了，导致接收方无法重组成完整的 UDP 包。

于是用 tcpdump 在 interface eth0 上抓包，出乎意料的是，`抓到的 pcap 有完整的 IP 分片`。用 wireshark 打开 pcap，wireshark 会自动把 IP 分片重组成 UDP 数据包，检查这个 UDP 包，`数据完整无误`。



## 实际导致问题的原因

因为 CVE-2018-5390 和 CVE-2018-5391，黑客可以用带有随机偏移量的 IP 分片引发 DoS 攻击（SegmentSmack and FragmentSmack，IP fragments and TCP segments with random offsets may cause a remote denial of service )

RedHat 给无法立刻升级内核的用户的临时解决方案是，减小 `net.ipv4.ipfrag_high_thresh` 的值，从 4MB 减小到 256 KB；`net.ipv4.ipfrag_low_thresh`, 从 3MB 到 192 KB。（详细见参考资料 1）

因为值改动的比较大，所以一些潜在的问题就暴露了出来。


## 重新认识 tcpdump 

tcpdump 直接从网络驱动层面抓取输入的数据，不经过任何 Linux 网络协议栈


iptables 依赖 netfilter 模块，后者工作在 Linux 网络协议栈中；


iptables 的入栈的策略不会影响到 tcpdump 抓取；iptables 的出栈策略会影响数据包发送到网络驱动层面，因此，出栈策略会影响到 tcpdump 的抓取


tcpdump 可以抓取到被 iptables 在 INPUT 链上 DROP 掉的数据包；tcpdump 不能抓取到被 iptables 在 OUTPUT 链上 DROP 掉的数据包



## 参考资料
- [https://access.redhat.com/articles/3553061](https://access.redhat.com/articles/3553061)




