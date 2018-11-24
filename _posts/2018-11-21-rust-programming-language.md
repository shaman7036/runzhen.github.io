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


