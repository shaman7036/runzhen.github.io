---
layout: post
comments: yes
title: "从书上的一个错误说 Buffer Overflow"
category: "linux"
tags: [linux, kernel]
---

时间倒回到2011年5月的一天，大学的最后一门课《计算机信息安全技术》，讲到《缓冲区溢出》这一章，并且给出了一段示例代码来演示缓冲区溢出，回到宿舍后出于好奇我运行了一下这段代码，发现结果并不是书上所说的那样，当时在人人网也发过一篇吐槽的日志，但是一直拖到现在都没有仔细的去研究过，正好现在十一放假没事，就花点时间搞搞啦。

书第136页-137页。代码如下，出于简单考虑（其实书上的C++代码格式也是错的），我除去了头文件和cout函数，这样就跟纯C语言代码是一样了。

```
void function(int a)
{
    char buffer[5];
    char *ret;
    ret = buffer + 12;
    *ret += 8;
}

int main()
{
    int x;
    x = 10;
    function(7);
    x = 1;
    return 0;
}
```

书上说最后x的值是10，不是1，而我的结果恰恰相反。
接着用gcc产生汇编代码，在这里用 `gcc -O0 -S`命令告诉编译器不采用任何优化措施，产生最原始的汇编代码，这样有利于我们分析，即使是采用-O1级优化的时候，汇编代码已经很难读了，大家可以试一试。

```
function:
    pushl %ebp
    movl %esp, %ebp
    subl $16, %esp
    leal -9(%ebp), %eax
    addl $12, %eax
    movl %eax, -4(%ebp)
    movl -4(%ebp), %eax
    movzbl (%eax), %eax
    addl $8, %eax
    movl %eax, %edx
    movl -4(%ebp), %eax
    movb %dl, (%eax)
    leave

main:
    pushl %ebp
    movl %esp, %ebp
    subl $20, %esp
    movl $10, -4(%ebp)
    movl $7, (%esp)
    call function
    movl $1, -4(%ebp)
    leave
    ret
```

书上详细的解释了为什么结果是10，下面我来逐条分析。首先画一张内存图，同样处于简洁考虑，只画function函数附近的内存分布，不影响分析。

![buff](/image/2011/buffer-overflow.png)


1）书上说：“为char buffer[5]分配内存时，由于32位存储器需要4字节对齐，因此一共为buffer分配了8个字节”。
由 `leal -9(%ebp), %eax `代码可以看出，编译器实际上没有为buffer分配8个字节，而是只分配了5个字节，还有4个字节给了 char *ret,因此正好是9个字节，见图中 `0xEB – 0xE3` 部分。    

2）书上说：“执行 ret=buffer+12后，ret指向返回地址” 实际上，从图上看出加12后指向的是%ebp的最高字节0xEF。

3）书上说："执行 ret+=8后，返回地址的值加上了8个字节，而 x=1 这条语句占有8个字节，因此正好跳过，执行下一条语句"。实际上是把 `%ebp=0x00000108` 的最高有效位 `0x00加8` 了，因此运行时会产生“段错误”（是这个原因吗？求指教）   

因此，书上长篇大论的分析在我的机器上是行不通的，不过还要注意2点：

1）现代的编译器通常都引入了“栈随机化”、“破坏检测”等多种手段来阻止缓冲区溢出的攻击，像书中所说的那样通过把某个指针+8，程序就从某条特定语句开始执行并不是那么简单就能实现的。

2）即使是相同的 OS 和 gcc 版本，在不同的CPU上生成的汇编代码是不一样的，运行的结果也不一样。例如，Core i5 和奔腾双核，在Core i5上运行结果是1，甚至都不会产生“Segmentation fault”的错误，而在奔腾CPU上结果为1，产生“段错误”提示。

总之，书上的代码也许在某个特定的、老版本的编译器环境下会发生，但是结果是不可重复的，而且，在不同机器上也会表现出不同结果。


### 从旧博客拷贝的评论

> Dave: 正如你所说需要指定编译器支持,才会有指定的结果. 或者写汇编吧^^
> Me: 请教D大：保存的旧的返回地址是基于旧的esp的相对偏移量吧
> Dave: 函数保存的返回地址是函数外的下一行指令的地址。
> Me: 那么下一条指令的地址应该是基于外层函数的ebp的偏移量吧？
> Dave: 不是偏移量（当然你可以理解成与进程空间0地址的偏移量），是进程空间的虚拟地址。
> Me: 这样就对了，这里虽然ebp的值被修改了，但是返回地址并不是基于ebp的偏移量，所以可以正常返回到main函数里，所以执行结果还是print了1，并且提示“段错误”。



某热心网友 **Anonymous**
> 改成 ret=buffer+13; 试试
> 这个例子来自于 [http://phrack.org/issues/49/14.html](http://phrack.org/issues/49/14.html)
> 看看 ret 地址，是不是被放在 buffer 的前面了。另外编译时使加上 -fno-stack-protector 选项，这样栈的结构才和上文中的结构一致。


### 其他资料
-  [http://phrack.org/issues/49/14.html](http://phrack.org/issues/49/14.html)







