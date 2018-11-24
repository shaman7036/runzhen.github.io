---
layout: post
comments: no
title: "Rust 编程语言"
category: "rust"
tags: [rust]
---

* table
{:toc}
***

Rust 编程语言知识点笔记。

# trait 关键字

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

Rust提供了一个属性derive来自动实现一些trait，这样可以避免重复繁琐地实现他们，能被derive使用的trait包括：Clone, Copy, Debug, Default, Eq, Hash, Ord, PartialEq, PartialOrd

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

Rust没有继承，它和Golang不约而同的选择了trait(Golang叫Interface)作为其实现多态的基础。
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

stackoverflow 上有一个不错的解释 [https://stackoverflow.com/questions/46867652/cannot-import-a-module-in-an-integration-test](https://stackoverflow.com/questions/46867652/cannot-import-a-module-in-an-integration-test)

> Each file defines a module. Your lib.rs defines a module of the same name as your crate; a mod.rs defines a module of the same name as the directory it's in; every other file defines a module of the name of the file.

> The root of your library crate must be lib.rs; binary crates may use main.rs.






