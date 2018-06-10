---
layout: post
comments: no
title: SSL/TLS Perfect Forward Secrecy
category: Security 
tags: [RSA, ECDHE, DHE, cryption]
---

HTTPS 这样的加密传输，最关键的是要解决`两个问题`，一个是通信双方如何协商出一个对称加密的密钥（密钥交换），二是自己如何确认对方的服务器就是我想访问的，即认证。

第一个问题可以由非对称加密解决，第二个问题是证书的合法性校验，“签发数字签名”，指的是用hash函数，对证书中的发行者，有效期，证书名等信息计算得到一个摘要（digest），然后用私钥进行加密，得到签名A。而对应的“校验数字签名”，则是先用相同的hash函数得到digest, 再利用相应的公钥解密A，然后对比自己hash出的和解密出的A是否相同，以此来认证对方是不是我们想要通信的人。

写到这里，正好最近有一个有趣的新闻：[SHA1签名算法被破解](https://security.googleblog.com/2017/02/announcing-first-sha1-collision.html)，也就是说，安全研究人员根据理论上的SHA1破解算法，运用大量的计算资源，终于生成了两个不同的 PDF文件，但是它们的SHA1 值是一样的，所以以后我们应该使用更高级的签名算法来避免可能的安全漏洞。有兴趣的可以移步到上面的链接查看。

```
|算法     | 密钥交换 |  认证   |
|--------|---------|---------|
|基于 RSA |  RSA    | RSA    |
|基于 DH  |  DH     | RSA/DSA|
```

相比传统的 RSA 握手，ECDHE 能支持 forward secrecy(DH 算法本身没有forward secrecy，要 ECDHE 才行)。DH 算法让 client 和 server 分别独立的计算出同步加密的密钥，注意：是独立计算出，而不是通过一方传递给另一方。

加密套件，类似于 “ECDHE-ECDSA-AES256-SHA384” 这样的一串，主要包含这些信息：

* 密钥协商的算法，要么 RSA，要么 DH
* 认证方法，比如 SHA
* 同步加密的方法，即 session key 所属的类型
* hash 函数，保证传输的用户数据的完整性

在探讨 `Perfect Forward Secrecy` 之前，先来回顾一下旧的 TLS 握手过程会有什么缺陷。

# 没有 PFS 的 RSA 握手过程

![RSA handshake](/image/2017/ssl_handshake_rsa.png){:height="700" width="700"}

### 第一步，ClientHello

这一步，客户端向服务器提供以下信息：

1. 支持的协议版本，比如TLS 1.2
2. 一个客户端生成的随机数，用于生成 Master Secret。
3. 支持的加密方法，比如RSA公钥加密
4. 支持的压缩方法

### 第二步，SeverHello

服务端在接收到客户端的 ClientHello 之后，需要将自己的证书发送给客户端，这个证书是对于服务端的一种认证。例如，客户端收到了一个来自于称自己是 alipay.com 的数据，但是如何证明对方是合法的 alipay 呢？这就是证书的作用，支付宝的证书可以证明它是 alipay，而不是财付通。

证书由专门的数字证书认证机构(CA)审核之后颁发服务器。数字证书包括一个所谓的“证书”，还有一个私钥和公钥。私钥由服务端自己保存，公钥则是附带在证书的信息中。证书本身也附带一个证书电子签名，这个签名用来验证证书的完整性和真实性。

这一步，服务器向客户端提供以下信息：

1. 确认使用的加密通信协议版本，比如TLS 1.2版本。
2. 一个服务器生成的随机数，用于生成 Master Secret。
3. 确认使用的加密方法，比如RSA公钥加密
4. 服务器证书

### 第三步，客户端回应

客户端需要对服务端的证书进行检查，如果证书没有问题，客户端就会从服务器证书中取出服务器的公钥。向服务器发送下面三项信息：

1. 一个随机数 PreMaster Key 。该随机数用服务器公钥加密，防止被窃听。
2. 编码改变通知，表示随后的信息都将用双方商定的加密方法和密钥发送
3. 客户端握手结束通知，表示客户端的握手阶段已经结束。这一项同时也是前面发送的所有内容的hash值，用来供服务器校验

注意，上面三步我们提到了三次 **随机数**，这三个随机数是不一样的：

1. client random
2. server random
3. 第三个最特别，被称为 PreMaster Key，它是由 client 用 RSA 或者 Diffie-Hellman 加密算法生成的。

前面两者都随着 ClientHello，ServerHello `明文传输`，而 PreMaster Key 是需要用服务器的公钥加密后传给server，然后 server 用自己的私钥解密。

至此，客户端和服务器都有了相同的`三个随机数`，然后它们会分别使用一个 PRF(Pseudo-Random Function) 来产生 master-secret。

> master_secret = PRF(pre_master_secret, ClientHello.random + ServerHello.random)

master_secret 是一组秘钥，包含6各部分，但是对于非密码研究人员来说不需要知道的太详细，我们只要知道，master_secret 包含了对称加密的秘钥，也就是真正对传输的数据进行加密的秘钥。


这种方式把文章开头说过的**密钥交换**和**认证**用一个步骤解决了。背后的逻辑是：如果 server 可以生成正确的session key，那么它必定是有正确的私钥的。而这种方式的缺点是，一旦私钥泄露，那么攻击者可以解密所有的数据，更极端的情况是，攻击者可以先抓包保留几年的历史数据，一旦某天私钥泄露，那么之前的所有加密数据都可以被解密出来。

这种情况下，就催生出了 DH 密钥交换算法。(说“催生出”倒也不适合，确切的说DH算法和RSA几乎是同一时期出现的，但是没有RSA运用那么广泛，等到大家发现RSA在密钥交换中存在的缺陷时，暮然回首，发现在DH算法基础上做一些改动，就可以解决Perfect Forward Secrecy的问题。这个故事是不是可以提醒我们，当现有的技术出现困难时，是不是可以参考一下这个领域发表的一些经典老论文，说不定会有灵感。)

# DH 握手过程

首先来看一下 DH 算法的数学基础。

```
+-------------------------------------------------------------------+
|                    Global Pulic Elements                          |
|                                                                   |
|       p                               prime number                |
|       a                               prime number, a < p         |
+-------------------------------------------------------------------+
+-------------------------------------------------------------------+
|                    User A Key Generation                          |
|                                                                   |
|       Select private Xa               Xa < p                      |
|       Calculate public Ya             Ya = a^Xa mod p             |
+-------------------------------------------------------------------+
+-------------------------------------------------------------------+
|                    User B Key Generation                          |
|                                                                   |
|       Select private Xb               Xb < p                      |
|       Calculate public Yb             Yb = a^Xb mod p             |
+-------------------------------------------------------------------+
+-------------------------------------------------------------------+
|               Calculation of Secret Key by User A                 |
|                                                                   |
|       Secret Key K                    K = Yb^Xa mod p             |
+-------------------------------------------------------------------+
+-------------------------------------------------------------------+
|               Calculation of Secret Key by User B                 |
|                                                                   |
|       Secret Key K                    K = Ya^Xb mod p             |
+-------------------------------------------------------------------+

```

上面一共出现了 a, p, Xa, Ya, Xb, Yb, K 共 7 个数，其中：

* 公开的数：a, p, Ya, Yb    
* 非公开数：Xa, Xb, K    

通常情况下，a 一般为 2 或 5，而 p, Xa 和 Xb 的取值也非常大，其复杂度至少为 `O(p^0.5)`。对于攻击者来说，已知 Ya，Xa 的求解非常困难，同理 Xb 的求解也很困难，所以攻击者难以求出 K，所以 DH 能够保证通信双方在透明的信道中安全的交换密钥。

值得注意的是，*DH 算法本身并没有提供通讯双方的身份验证功能，也就是说 DH 算法会被中间人攻击。* 这时就需要 DSA（数字签名算法）来对传输的数据进行签名，这样使得接受双方可以确认信息的来源。

在握手过程中，client hello 和 server hello 过程都与RSA是一样的，RSA 的第三步是 “client key exchange”，而 ECDHE 的第三步是 “server key exchange”。

**Server Key Exchange** 
server 需要传递几个参数给client，同时，server 也要签发自己的数字签名，以便client 能够校验（验明正身）。在server key exchange 阶段，以上信息会被发给client。

**Client Key Exchange**
client 在收到上一步的信息后，首先验证证书是它想访问的server的，同时也发送自己的参数给server。现在，client 和 server 都拥有了 DH 算法中的3个参数，然后分别计算 pre-master key，然后根据 client random, server random，pre-master key 生成 session key。

![RSA handshake](/image/2017/ssl_handshake_diffie_hellman.png)

以上就是一个粗略版本的 RSA 和 DH 握手过程，而ECDH 是基于ECC（Elliptic Curve Cryptosystems）的 DH 密钥交换算法。ECDH 每次用一个固定的DH key，所以并不支持forward sercecy，支持 forward sercecy 的是 ECDHE 算法或其他版本的ECDH算法。

# ECDHE 的具体过程



参考资料
* https://blog.cloudflare.com/keyless-ssl-the-nitty-gritty-technical-details/
* https://vincent.bernat.im/en/blog/2011-ssl-perfect-forward-secrecy#diffie-hellman-with-discrete-logarithm






