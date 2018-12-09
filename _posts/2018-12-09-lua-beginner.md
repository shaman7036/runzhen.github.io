---
layout: post
comments: no
title: "Lua 语法入门"
category: "nginx"
tags: [nginx]
---

* table
{:toc}
***

Lua 是一门小巧的编程语言，但麻雀虽小五脏俱全，而且与 C 语言的交互非常友好，所以有人称它是 “胶水语言”。最近在研究 nginx，另一个广泛应用的、基于 nginx 的开源项目 OpenResty 就是把 lua 嵌入到了 nginx 中，很有意思。于是就来学习一下 lua。

## 基本语法

单行注释用 ` -- `，多行注释用 
```
--[[
多行注释
--]]
```

## 数据类型

1. nil 表示一个无效值（在条件表达式中相当于false）。
2. boolean 包含两个值：false和true。
3. number  表示双精度类型的实浮点数
4. string  字符串由一对**双引号**或**单引号**来表示
5. function    由 C 或 Lua 编写的函数
6. userdata    表示任意存储在变量中的C数据结构
7. thread  执行协同程序
8. table   表或者数组

### table 类型
在 Lua 里，table 的创建是通过"构造表达式"来完成，最简单构造表达式是{}，用来创建一个空表。

```
local tbl1 = {}
local tbl2 = {"apple", "pear", "orange", "grape"}
```

另外，数组的**索引可以是数字或者是字符串**。比如有以下代码：
```
a = {}
a["key"] = "value"
key = 10
a[key] = 22
a[key] = a[key] + 11
```
最后 table a 中的内容是 (10, 33) 和 (key, value)

### 函数
和 go 之类的高级语言一样，lua 也有匿名函数，比如下面这样：
```
function testFun(tab,fun)
    for k ,v in pairs(tab) do
        print(fun(k,v));
    end
end


tab={key1="val1",key2="val2"};
testFun(tab,
            function(key,val)
            return key.."="..val;
            end
);
```

## Lua 变量

连接一个字符串： `a = "hello" .. "world"`。

`t.i`  当索引为字符串类型时， t.i = t[i]

## 循环

无论是 for 还是 while 循环，循环体都被包在 “do  end” 之中。 看几个例子：

### for 
```
for var=exp1,exp2,exp3 do  
    <执行体>  
end  
```
或者
```
a = {"one", "two", "three"}
for i, v in ipairs(a) do
    print(i, v)
end 
```

### while 

while 循环比较容易记
```
while( a < 20 )
do
   a = a+1
end
```

### repeat until
这个是 lua 语言独有的循环方式，但它本质上就是 `do{ }while`

```
repeat
   statements
until( condition )
```


## if else 

lua 中的 if 需要带上 “then” 

```
if( 布尔表达式 1)
then
   --[ 在布尔表达式 1 为 true 时执行该语句块 --]
elseif( 布尔表达式 2)
then
   --[ 在布尔表达式 2 为 true 时执行该语句块 --]
else 
   --[ 如果以上布尔表达式都不为 true 则执行该语句块 --]
end
```

## 函数多值返回

Lua函数可以返回多个结果值，比如string.find，其返回匹配串"开始和结束的下标"

## 函数可变参数

这个比较有意思，在函数参数列表中使用三点 ... 表示函数有可变的参数。

```
function add(...)  
local s = 0  
  for i, v in ipairs{...} do 
    s = s + v  
  end  
  return s  
end  
print(add(3,4,5,6,7)) 
```
也可以通过 select("#",...) 来获取可变参数的数量:

```
function average(...)
   result = 0
   local arg={...}
   for i,v in ipairs(arg) do
      result = result + v
   end
   print("总共传入 " .. select("#",...) .. " 个数")
end
```


