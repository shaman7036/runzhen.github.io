---
layout: post
comments: no
title: "Rust 语言知识点记录"
category: "rust"
tags: [rust]
---

* table
{:toc}
***

Rust 编程语言知识点笔记。

# trait 关键字

Rust没有继承，它和Golang不约而同的选择了trait(Golang叫Interface)作为其实现多态的基础。

使用trait定义一个特征：
```
trait HasArea {
    fn area(&self) -> f64;
}
```

trait里面的函数可以没有函数体，实现代码交给具体实现它的类型去补充：

```
struct Circle {
    x: f64,
    y: f64,
    radius: f64,
}

impl HasArea for Circle {
    fn area(&self) -> f64 {
        std::f64::consts::PI * (self.radius * self.radius)
    }
}

fn main() {
    let c = Circle {
        x: 0.0f64,
        y: 0.0f64,
        radius: 1.0f64,
    };
    println!("circle c has an area of {}", c.area());
}
```

## derive 属性

Rust提供了一个属性derive来自动实现一些trait，这样可以避免重复繁琐地实现他们，能被derive使用的trait包括：Clone, Copy, Debug, Default, Eq, Hash, Ord, PartialEq, PartialOrd。常用的例子是：

```
#[derive(Debug, Eq)]
struct Centimeters(f64);  // 元组结构
```

上面的代码表示 Centimeters 已经拥有了Debug 和 Eq 函数，所以一个 Centimeters 的实体 test 就可以用 println 打印，比如 `println!("{:?}", test);`

再来看一个稍微复杂的[例子](https://rustwiki.org/zh-CN/rust-by-example/trait/derive.html)

```
#[derive(PartialEq, PartialOrd)]
struct Centimeters(f64);  //可以比较的元组结构体

#[derive(Debug)]
struct Inches(i32); //可以打印的元组结构体

struct Seconds(i32); //不带附加属性的元组结构体，所以不能比较也不能打印


// 额外实现 Inches 结构体一个方法 
impl Inches {
    fn to_centimeters(&self) -> Centimeters {
        let &Inches(inches) = self;
        Centimeters(inches as f64 * 2.54)
    }
}

fn main() {
    let _one_second = Seconds(1);

    println!("One second looks like: {:?}", _one_second);
        //错。它没有实现 `Debug` trait

    let _this_is_true = (_one_second == _one_second);
        //错。它没有实现 `PartialEq` trait

    let foot = Inches(12);
    println!("One foot equals {:?}", foot);

    let meter = Centimeters(100.0);
    let cmp =
        if foot.to_centimeters() < meter {
            "smaller"
        } else {
            "bigger"
        };

    println!("One foot is {} than one meter.", cmp);
}
```

上面的例子中可以看出，derive 了某个属性的结构体，可以使用各种方法；而 Seconds 没有任何附加属性，所以不能做任何操作，毫无用处。

接下来这个例子展示了我们可以自己实现属性 PartialEq 
```
enum BookFormat { Paperback, Hardback }
struct Book {
    isbn: i32,
    format: BookFormat,
}

impl PartialEq for Book {
    fn eq(&self, other: &Book) -> bool {
        self.isbn == other.isbn
    }
}

fn main() {
    let a = Book{isbn:2, format:BookFormat::Hardback};
    let b = Book{isbn:1, format:BookFormat::Paperback};

    println!("{}", a.eq(&b));
}
```

# macro_use

当使用外部包（ external crate ）时，为了防止无法预料的名字冲突，在导入外部包的同时也必须明确地用这个关键字将宏导入到项目中。下面的例子将所有定义在 serde 包中的宏导入到当前包中：

```
#[macro_use]
extern crate serde;
```


# impl

对一个结构体实现操作它的方法。

```
struct Person {
    name: String,
}

impl Person {
    fn new(n: &str) -> Person {
        Person {
            name: n.to_string(),
        }
    }

    fn greeting(&self) {
        println!("{} say hello .", self.name);
    }
}
```

# mod 模块和文件夹路径

stackoverflow 上有一个不错的解释 [传送门](https://stackoverflow.com/questions/46867652/cannot-import-a-module-in-an-integration-test)

> Each file defines a module. Your lib.rs defines a module of the same name as your crate; a mod.rs defines a module of the same name as the directory it's in; every other file defines a module of the name of the file.

> The root of your library crate must be lib.rs; binary crates may use main.rs.


# deref

解引用操作，可以被自定义。方法是，实现标准库中的std::ops::Deref和std::ops::DerefMut这两个 trait。

Deref的定义如下所示，DerefMut的唯一区别是返回的是&mut型引用。**trait的定义** 见第一小节。

```
pub trait Deref {
    type Target: ?Sized;
    fn deref(&self) -> &Self::Target;
}

pub trait DerefMut: Deref {
    fn deref_mut(&mut self) -> &mut Self::Target;
}
```

以上是标准库中定义好的，用户可以自定义他们的实现，比如 String 向 str 的解引用转换:

```
impl ops::Deref for String {
    type Target = str;

    #[inline]
    fn deref(&self) -> &str {
        unsafe { str::from_utf8_unchecked(&self.vec) }
    }
}
```

# 泛型

[传送门](https://zhuanlan.zhihu.com/p/22682496)

## impl 中的泛型

impl 也可以使用泛型。特别是当我们希望为某一类类型统一 impl 某个 trait 的时候非常有用。有了这个功能，很多时候就没必要单独为每个类型去重复 impl 了。

```
impl<T, U> Into<U> for T where U: From<T>
{
    fn into(self) -> U {
        U::from(self)
    }
}
```
标准库中的 Into 和 From 就是一对功能互逆的 trait。如果 `A: Into<B>` 意味着 `B: From<A>`。因此，标准库中写了这样一段代码，意思是，针对所有类型 T，只要满足 `U: From<T>`，那么就针对此类型 `impl Into<U>`。有了这样的一个 impl 块之后，我们如果想为自己的两个类型提供互相转换的功能，那么只需 impl From 这一个 trait 就够了，因为反过来的 Into trait 标准库已经帮忙实现好了。


# 闭包 ||

闭包的定义以一对竖线（|）开始，在竖线中指定闭包的参数。一个参数 num 可以写成 |num|；如果有多于一个参数，可以使用逗号分隔，比如 |param1, param2|。 
举例：

```
let expensive_closure = |num| {
    println!("calculating slowly...");
    thread::sleep(Duration::from_secs(2));
    num
};
```

# 线程

创建一个新线程，需要调用 thread::spawn 函数并传递一个闭包，其包含希望在新线程运行的代码。
```
fn main() {
    thread::spawn(|| {
        for i in 1..10 {
            println!("hi number {} from the spawned thread!", i);
            thread::sleep(Duration::from_millis(1));
        }
    });
}
```

# 迭代器与 consumer

`let num = (1..101).collect::<Vec<i32>>();` 

collect 作用在一个迭代器上，收集所有的元素。 `::<Vec<i32>>` 的作用是给一个提示，告诉编译器数据类型。

collect() 是一个 common consumer, 还有一个常用的是 find()。

除了 common consumer 之外，迭代器还有一个 Iterator adaptors，它作用在一个迭代器上，然后修改它的值，并且产生一个新迭代器。

最常见的就是 map(), take(), filter()

> filter() is an adapter that takes a closure as an argument. This closure returns true or false. The new iterator filter() produces only the elements that the closure returns true

> take(n) will return an iterator over the next n elements of the original iterator.


[官方文档](https://doc.rust-lang.org/1.8.0/book/iterators.html)


# Learn by examples

```
fn bench_yield(n: i32) {
    let times = MAX/n;
    let ths: Vec<_> = (0..n)
        .into_iter()
        .map(|_| thread::spawn(move || for _ in 0..times {
            unsafe{
                libc::syscall(libc::SYS_sched_yield);
            }
        })).collect();

    for th in ths {
        th.join().unwarp();
    }
}
```

`thread::spawn(move)`  move 闭包，经常与 thread::spawn 一起使用，它允许我们在一个线程中使用另一个线程的数据。在参数列表前使用 move 关键字强制闭包获取其使用的环境值的所有权。这个技巧在将闭包传递给新线程以便将数据移动到新线程中时最为实用。

`0..n` 是从 0 到 n 之间整数的缩写，之后的 into_iter，collect 都是有关[迭代器](https://doc.rust-lang.org/1.8.0/book/iterators.html)的语法。




