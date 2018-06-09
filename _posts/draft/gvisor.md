
handshake 独立出来处理，有专门的结构体 struct handshake, 这是与其他 TCP 实现区别最大的地方，我见过的都是所有类型数据包放在一个状态机中处理。


常见的三次握手分主动方和被动方，因此代码的调用路径也不一样，同一个函数中代码执行路径也不一样。

A 调用 Connect() 发起一个 SYN，于是状态转变成 `SYN-SENT`，B 收到 SYN 后状态变为 `SYN-RECEIVED`。 * A 不可能有 SYN-RECEIVED 状态 * 。 

体现到代码上，handshake结构体的成员函数 handleSegment() 中有两个处理函数， A 会调用 synSentState()，B 会调用 synRcvdState()
```

      TCP A                                                TCP B

  1.  CLOSED                                               LISTEN

  2.  SYN-SENT    --> <SEQ=100><CTL=SYN>               --> SYN-RECEIVED

  3.  ESTABLISHED <-- <SEQ=300><ACK=101><CTL=SYN,ACK>  <-- SYN-RECEIVED

  4.  ESTABLISHED --> <SEQ=101><ACK=301><CTL=ACK>       --> ESTABLISHED

  5.  ESTABLISHED --> <SEQ=101><ACK=301><CTL=ACK><DATA> --> ESTABLISHED

          Basic 3-Way Handshake for Connection Synchronization

                                Figure 7.
```

