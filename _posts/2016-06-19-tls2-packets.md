---
layout: post
comments: no
title: "抓包分析 TLS 1.2 连接过程"
category: Security
tags: [tls]
---

本文是《Traffic Analysis of an SSL/TLS Session》的笔记，原文见[此处](http://blog.fourthbit.com/2014/12/23/traffic-analysis-of-an-ssl-slash-tls-session)。

在 [TLS 协议格式](/2016/06/tls/) 中详细分析了协议数据的各个字段，
现在就来实战 ——用 Wireshark 抓包并观察数据。

# 第一发（Client -> Server）

```
 Full contents of the packet

 0000   02 00 00 00 45 00 00 98 13 ed 40 00 40 06 00 00  ....E.....@.@...
 0010   7f 00 00 01 7f 00 00 01 ec 26 01 bb 43 7c ee 74  .........&..C|.t
 0020   60 b5 50 0a 80 18 31 d7 fe 8c 00 00 01 01 08 0a  `.P...1.........
 0030   21 62 1e 1e 21 62 1e 1e 16 03 01 00 5f 01 00 00  !b..!b......_...
 0040   5b 03 01 54 9a ab 72 98 65 11 2f da 9e cf c9 db  [..T..r.e./.....
 0050   6c bd 4b 4c 56 4b 0c a5 68 2b aa 60 1f 38 66 e7  l.KLVK..h+.`.8f.
 0060   87 46 b2 00 00 2e 00 39 00 38 00 35 00 16 00 13  .F.....9.8.5....
 0070   00 0a 00 33 00 32 00 2f 00 9a 00 99 00 96 00 05  ...3.2./........
 0080   00 04 00 15 00 12 00 09 00 14 00 11 00 08 00 06  ................
 0090   00 03 00 ff 01 00 00 04 00 23 00 00              .........#..

 ------------

 Family: IP

 0000   02 00 00 00                                      ....

 IP protocol    <--- IP协议首部 20 个字节，无“选项”字段。

 0000   45 00 00 98 13 ed 40 00 40 06 00 00 7f 00 00 01  E.....@.@.......
 0010   7f 00 00 01                                      ....

 TCP protocol  <-- TCP 协议首部标准的为 20 个字节，这里一共32个字节，说明有“选项”字段。 

 0000   ec 26 01 bb 43 7c ee 74 60 b5 50 0a 80 18 31 d7  .&..C|.t`.P...1.
 0010   fe 8c 00 00 01 01 08 0a 21 62 1e 1e 21 62 1e 1e  ........!b..!b..

 ------------

 TLSv1 protocol

 0000   16 03 01 00 5f 01 00 00 5b 03 01 54 9a ab 72 98  ...._...[..T..r.
 0010   65 11 2f da 9e cf c9 db 6c bd 4b 4c 56 4b 0c a5  e./.....l.KLVK..
 0020   68 2b aa 60 1f 38 66 e7 87 46 b2 00 00 2e 00 39  h+.`.8f..F.....9
 0030   00 38 00 35 00 16 00 13 00 0a 00 33 00 32 00 2f  .8.5.......3.2./
 0040   00 9a 00 99 00 96 00 05 00 04 00 15 00 12 00 09  ................
 0050   00 14 00 11 00 08 00 06 00 03 00 ff 01 00 00 04  ................
 0060   00 23 00 00                                      .#..

 TLSv1 Record protocol

 0000   16 03 01 00 5f                                   ...._

        16             Handshake protocol type
        03 01          SSL version (TLS 1.0)
        00 5f          Record length (95 bytes)

 TLSv1 Handshake protocol

 0000   01 00 00 5b 03 01 54 9a ab 72 98 65 11 2f da 9e  ...[..T..r.e./..
 0010   cf c9 db 6c bd 4b 4c 56 4b 0c a5 68 2b aa 60 1f  ...l.KLVK..h+.`.
 0020   38 66 e7 87 46 b2 00 00 2e 00 39 00 38 00 35 00  8f..F.....9.8.5.
 0030   16 00 13 00 0a 00 33 00 32 00 2f 00 9a 00 99 00  ......3.2./.....
 0040   96 00 05 00 04 00 15 00 12 00 09 00 14 00 11 00  ................
 0050   08 00 06 00 03 00 ff 01 00 00 04 00 23 00 00     ............#..

        01             ClientHello message type
        00 00 5b       Message length           <---- 这里消息长度为3字节，大于记录协议的2字节，
		                                              也就是说，一个记录块放不下可以放到下一个？
        03 01          SSL version (TLS 1.0)
        54 .. b2       32-bytes random number
        00             Session Id length
        00 2e          Cipher Suites length (46 bytes, 23 suites)
        00 39 .. ff    23 2-byte Cipher Suite Id numbers
        01             Compression methods length (1 byte)
        00             Compression method (null)
        00 04          Extensions length (4 bytes)
        00 23          SessionTicket TLS extension Id
        00 00          Extension data length (0)
```

# 第二发（Server -> Client）

这个例子中，服务端发送了三种消息，ServerHello，Certificate，ServerHelloDone。这是一种最简单的方式，服务端没有要求验证客户端证书。

```
ServerHello message    

0000   16 03 01 00 35 02 00 00 31 03 01 54 9a ab 72 85  ....5...1..T..r.
0010   91 a4 a7 a9 27 fe 3d e4 da f6 38 a5 aa 6e 5a 2f  ....'.=...8..nZ/
0020   31 90 5b 41 b0 5d de d8 9d ae f6 00 00 35 00 00  1.[A.].......5..
0030   09 ff 01 00 01 00 00 23 00 00                    .......#..

       16             Handshake protocol type
       03 01          SSL version (TLS 1.0)
       35             Record length (53 bytes)

       02             ServerHello message type
       00 00 31       Message length (49 bytes)
       03 01          SSL version (TLS 1.0)
       54 9a ab 72    First 4 bytes of random (Unix time)
       85 .. f6       Last 28 bytes of the random number
       00             Session Id length
       00 35          Selected Cipher Suite (RSA with AES-256-CBC SHA)
       00             Selected compression method (null)
       00 09          Extensions length
       ff 01 00 01 00 Extension (Renegotiation Info)
       00 23 00 00    Extension (SessionTicket TLS)

Certificate message           

0000   16 03 01 01 e4 0b 00 01 e0 00 01 dd 00 01 da 30  ...............0
0010   82 01 d6 30 82 01 3f 02 01 01 30 0d 06 09 2a 86  ...0..?...0...*.
0020   48 86 f7 0d 01 01 04 05 00 30 45 31 0b 30 09 06  H........0E1.0..
0030   03 55 04 06 13 02 41 55 31 13 30 11 06 03 55 04  .U....AU1.0...U.
0040   08 13 0a 53 6f 6d 65 2d 53 74 61 74 65 31 21 30  ...Some-State1!0
0050   1f 06 03 55 04 0a 13 18 49 6e 74 65 72 6e 65 74  ...U....Internet
0060   20 57 69 64 67 69 74 73 20 50 74 79 20 4c 74 64   Widgits Pty Ltd
0070   30 1e 17 0d 39 39 30 35 30 31 30 31 32 36 33 35  0...990501012635
0080   5a 17 0d 39 39 30 35 33 31 30 31 32 36 33 35 5a  Z..990531012635Z
0090   30 22 31 0b 30 09 06 03 55 04 06 13 02 44 45 31  0"1.0...U....DE1
00a0   13 30 11 06 03 55 04 03 13 0a 54 65 73 74 73 65  .0...U....Testse
00b0   72 76 65 72 30 81 9f 30 0d 06 09 2a 86 48 86 f7  rver0..0...*.H..
00c0   0d 01 01 01 05 00 03 81 8d 00 30 81 89 02 81 81  ..........0.....
00d0   00 fa 23 7a 03 2a 27 b1 c3 09 64 ce 36 ab eb d0  ..#z.*'...d.6...
00e0   08 16 75 54 68 6f 39 2e d0 9e 81 ed 91 f8 2b 48  ..uTho9.......+H
00f0   0e 59 10 63 0e bc ff c3 1b 4f 7a 2e d2 97 45 01  .Y.c.....Oz...E.
0100   c2 fd 20 68 98 63 76 34 48 73 3d 3e a1 74 d1 13  .. h.cv4Hs=>.t..
0110   b5 30 2b 4d a6 a4 e7 17 74 9c 2e 96 e6 82 01 a3  .0+M....t.......
0120   2a 29 66 59 89 f6 6a 2e de 99 d8 cc 8d 75 4b b7  *)fY..j......uK.
0130   35 96 db 11 a0 20 60 13 59 03 77 d8 a8 1f 26 78  5.... `.Y.w...&x
0140   38 8d 78 b5 52 31 22 c8 b8 64 c3 46 5f d4 8f e0  8.x.R1"..d.F_...
0150   83 02 03 01 00 01 30 0d 06 09 2a 86 48 86 f7 0d  ......0...*.H...
0160   01 01 04 05 00 03 81 81 00 c8 0c fa c6 c0 93 c0  ................
0170   df 8d 27 da f9 17 f6 81 c1 97 99 ba ef 64 0c ca  ..'..........d..
0180   cc 2f b9 45 4d e4 6a af cd cb 12 17 00 67 28 f5  ./.EM.j......g(.
0190   d6 63 a3 3c d6 7c df f1 b8 6b a9 e5 ba 05 93 e2  .c.<.|...k......
01a0   ab 3f ec 5d 82 c6 aa 18 7b 32 ce 58 04 a2 ac f8  .?.]....{2.X....
01b0   7a 4a 8b 8d 07 95 6e 7a 23 df 7f 61 54 55 3d 32  zJ....nz#..aTU=2
01c0   13 e2 e8 95 0b 3f 18 d7 2a e9 a3 7d 7d 8b 2c d9  .....?..*..}}.,.
01d0   22 91 6e 69 bb 3f 03 7f 75 22 5f 41 22 68 9b dd  ".ni.?..u"_A"h..
01e0   ec 4c 0f f0 9e f9 b6 25 13                       .L.....%.

       16             Handshake protocol type
       03 01          SSL version (TLS 1.0)
       01 e4          Record length (484 bytes)

       0b             Certificate message type
       00 01 e0       Message length
       00 01 dd       Certificates length
       00 .. 13       Certificates data

ServerHelloDone message

0000   16 03 01 00 04 0e 00 00 00                       .........

       16             Handshake protocol type
       03 01          SSL version (TLS 1.0)
       00 04          Record length (4 bytes)

       0e             ServerHelloDone message
       00 00 00       Message length
```

# 第三发 （Client -> Server)

在发送这条消息之前，客户端和服务器已经协商好了各种算法：Key Exchange 用 RSA， 对称加密用 AES-256-CBC ，
消息摘要用 SHA。TLS 的扩展字段有 SessionTicket TLS 和 Renegotiation Info。

此时，客户端也拥有了服务器的证书链，可以验证是否信任这些证书。验证完毕之后，发送下面三种消息。

```
ClientKeyExchange message

 0000   16 03 01 00 86 10 00 00 82 00 80 2d 28 e4 30 eb  ...........-(.0.
 0010   31 35 b0 4b 5e 4c 4d c6 ee 01 f5 33 e7 f8 3f 9b  15.K^LM....3..?.
 0020   d7 53 fc 5c e0 2d d6 12 ba 55 f8 46 ab 73 d8 3d  .S.\.-...U.F.s.=
 0030   b0 0a f7 03 7f 58 e0 32 8f 91 1f b8 cf 56 aa 89  .....X.2.....V..
 0040   9e 27 84 08 ec 78 f8 74 0c d3 80 f2 ec 04 65 e1  .'...x.t......e.
 0050   3e 92 91 52 b5 5e aa 67 e9 e6 40 e9 10 67 3c 3f  >..R.^.g..@..g<?
 0060   73 f7 62 4a 0c 42 30 c1 06 6f 53 2f c2 6b d5 c8  s.bJ.B0..oS/.k..
 0070   67 6f 06 d7 92 86 6e 1d 4d dd 6b 3f b0 26 6c 25  go....n.M.k?.&l%
 0080   2c d8 81 5a 80 e0 e2 cc d1 62 9c                 ,..Z.....b.

        16             Handshake protocol type
        03 01          SSL version (TLS 1.0)
        00 86          Record length (134 bytes)

        10             ClientKeyExchange message type
        00 00 82       Message length (130 bytes)
        00 .. 9c       RSA encrypted key data (premaster secret) <--- 这个用来干嘛？

 ChangeCipherSpec message

 0000   14 03 01 00 01 01                                ......

        14             ChangeCipherSpec protocol type
        03 01          SSL version (TLS 1.0)
        00 01          Message length (1 byte)

        01             ChangeCipherSpec message

 Finished message (encrypted)

 0000   16 03 01 00 30 0c c1 86 73 a4 a3 26 62 30 21 7f  ....0...s..&b0!.
 0010   c3 2f 1a 83 34 2d 57 f0 e2 0d 37 d4 51 66 08 22  ./..4-W...7.Qf."
 0020   b0 ea b4 a4 1e 81 2a fd 5f 07 47 9f b7 2c 0a dc  ......*._.G..,..
 0030   65 08 77 40 2a                                   e.w@*

        16             Handshake protocol type
        03 01          SSL version (TLS 1.0)
        00 30          Message length (48 bytes)

        0c .. 2a       Encrypted Finished message
```

# 第四发 （Server -> Client）

服务端在接收到客户端发来的  ChangeCipherSpec 和 Finished 之后，也要发送自己的  ChangeCipherSpec 和 Finished 消息，这样握手过程才算结束。
对于报文中的 NewSessionTicket 消息，这是 TLS 的一个扩展，具体可参见 RFC5077。

```
 NewSessionTicket message

 0000   16 03 01 00 aa 04 00 00 a6 00 00 00 00 00 a0 f7  ................
 0010   2f 0c fd be ce f7 96 86 ca fd da 58 d6 16 b3 3c  /..........X...<
 0020   89 1a a5 a2 af 3c 80 50 7b 99 71 05 3b 0e d3 27  .....<.P{.q.;..'
 0030   75 78 0d 0a 20 6c e7 1c ce 7b 5d 52 ad f1 04 88  ux.. l...{]R....
 0040   ec fa 04 c9 6a 74 fc 7b 3d 99 aa 8a ec 7a a3 18  ....jt.{=....z..
 0050   81 63 2f db b0 16 5b 49 63 f4 53 bc 57 18 27 37  .c/...[Ic.S.W.'7
 0060   f2 7f 66 e6 4d 46 59 2d 17 39 d5 79 a4 49 4d 93  ..f.MFY-.9.y.IM.
 0070   d2 80 34 8b 49 f5 31 72 7f 7b 41 46 37 9b ae a9  ..4.I.1r.{AF7...
 0080   3c f0 6f 2e 7f 75 e3 bf 2f d8 fc a4 be cb 2c 84  <.o..u../.....,.
 0090   01 b2 25 01 23 91 6e c0 c1 09 9d 42 c8 b8 e6 1b  ..%.#.n....B....
 00a0   fe 1e ed b3 52 7f 25 90 ae fc 34 f5 96 1b f0     ....R.%...4....

        16             Handshake protocol type
        03 01          SSL version (TLS 1.0)
        aa 04          Message length (170 bytes)

        04             New Session Ticket message type (extension)
        00 00 a6       Message length (166 bytes)
        00 .. f0       Session Ticket data

 ChangeCipherSpec message

 0000   14 03 01 00 01 01                                ......

        14             ChangeCipherSpec protocol type
        03 01          SSL version (TLS 1.0)
        00 01          Message length

        01             ChangeCipherSpec message

 Finished message (encrypted)

 0000   16 03 01 00 30 6d 09 0e 9f dd 09 03 2f 84 65 f8  ....0m....../.e.
 0010   94 0f d6 7b 4b 54 31 a1 25 a4 27 03 ae c3 4e af  ...{KT1.%.'...N.
 0020   27 04 32 5a 1f 29 90 fa 0a 4b 89 2f af d8 88 99  '.2Z.)...K./....
 0030   41 de dd 89 3f                                   A...?

        16             Handshake protocol type
        03 01          SSL version (TLS 1.0)
        00 30          Message length (48 bytes)

        6d .. 3f       Encrypted Finished message
```

# 最后
握手成功之后，客户端和服务端就开始加密传输 TLS 之上的协议发来的数据了。

