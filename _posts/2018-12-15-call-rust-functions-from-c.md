---
layout: post
comments: no
title: "从 C 语言调用 Rust 的函数"
category: "blog"
tags: [rust]
---

* table
{:toc}
***

看到 rust 可以编译成动态链接库（.so），想到是不是可以用 C 语言链接到这个库呢？答案是肯定的。Rust 提供了 FFI 接口，即 Foreign Function Interface，目的就是和其他语言交互。

废话不多说，开始干。我们要实现三个例子：

- C 调用 Rust 动态库
- C 调用 Rust 静态库
- Rust 调用 C 函数 (不是库)

# C 调用 Rust 动态库

首先是用 cargo new NAME --lib 创建一个新项目，然后编辑 src/lib.rs

```
#![crate_type = "dylib"]

#[no_mangle]
pub extern fn double_input(input: i32) -> i32 {
    println!("hello --from rust shared library");
    input * 2
}
```
**crate_type = "dylib"** 代表编译成动态链接库。

**no_mangle** 告诉 rust 编译器，不要擅自改变下面这个函数的函数名。一些高级语言比如 c++ 之类，为了防止不同库中的函数名冲突，都会在编译时给每个函数生成独一无二的函数名，比如 `func::h485dee`。


然后编辑  Cargo.toml 文件，在默认的文件基础上加入：
```
[lib]
name = "double_input"
crate-type = ["dylib"]
```

最后 cargo build， 就能在 target/debug/ 目录下找到动态链接库 libdouble_input.so 

然后准备一个 C 文件，也非常简单:
```
extern int32_t double_input(int32_t input);

int main() {
    int input = 4;
    int output = double_input(input);
    printf("%d * 2 = %d\n", input, output);
    return 0;
}
```
注意它用 extern 关键字表明函数在外部。

好了，现在有了.so 文件，也有了 C 文件，现在要把他们编译到一起。

`gcc test.c -L ./target/debug/ -ldouble_input -o test`

其中 -L 选项说明库文件所在的路径，-l 说明库文件的名字。

通过上面的步骤，我们生成了一个 test 可执行文件，现在运行它。

```
$ ./test
./test: error while loading shared libraries: libdouble_input.so: cannot open shared object file: No such file or directory
```

系统无法找到库。虽然我们在编译时(compile time)提供了.so文件的位置，但这个信息并没有写入test可执行文件，所以在程序执行期间的动态链接时，默认的搜索路径下找不到这个库。

有两种方法解决这个问题，一个是设置 LD_LIBRARY_PATH **环境变量**。比如:

`$export LD_LIBRARY_PATH=.`

这样，可执行文件执行时，操作系统将先在LD_LIBRARY_PATH下搜索库文件，再到默认路径中搜索。

环境变量的坏处是，它会影响所有的可执行程序。

另一个解决方案，即提供 `-rpath` 选项，将搜索路径信息写入test文件(rpath代表runtime path)，就不需要设置环境变量了。

`$gcc -g -o test test.c -ldouble_input -L. -Wl,-rpath=.`

其中，`-Wl` 表示 `-rpath`选项是传递给连接器(linker)。

我们可以用 ldd 命令对比一下前后的结果，注意 libdouble_input.so 的地址。
```
$ ldd test
    linux-vdso.so.1 (0x00007ffcdb939000)
    libdouble_input.so => not found
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f2d5972a000)
    /lib64/ld-linux-x86-64.so.2 (0x00007f2d59d1d000)


$ ldd test
    linux-vdso.so.1 (0x00007ffea124d000)
    libdouble_input.so => ./target/debug/libdouble_input.so (0x00007fbb2d783000)
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fbb2d392000)
```

完整的程序代码[在这里]()

# C 调用 Rust 静态库



# Rust 调用 C 源码




参考资料

- [https://blog.rust-lang.org/2015/04/24/Rust-Once-Run-Everywhere.html](https://blog.rust-lang.org/2015/04/24/Rust-Once-Run-Everywhere.html)
- [https://github.com/alexcrichton/rust-ffi-examples](https://github.com/alexcrichton/rust-ffi-examples)
- [https://doc.rust-lang.org/cargo/reference/manifest.html#building-dynamic-or-static-libraries](https://doc.rust-lang.org/cargo/reference/manifest.html#building-dynamic-or-static-libraries)