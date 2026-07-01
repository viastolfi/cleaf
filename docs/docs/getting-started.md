---
id: getting-started
title: Getting Started
sidebar_position: 2
---

# Getting Started

## Dependencies

To build the Cleaf compiler:

- `gcc`

To compile `.clf` source files with the resulting binary:

- `nasm`
- `ld` (from binutils)

## Building the compiler

```sh
git clone https://github.com/viastolfi/cleaf.git
cd cleaf
make
```

This produces `./build/cleaf`.

## Compiling a Cleaf program

```sh
./build/cleaf <source.clf>           # compile to ./build/a.out
./build/cleaf <source.clf> -o <out>  # compile with a custom output name (./build/<out>)
```

### Debug flags

```sh
./build/cleaf <source.clf> -v        # show each compilation phase and its result
./build/cleaf <source.clf> -V        # same as -v, and dump AST, HIR, and generated assembly
```

## Your first program

Create a file called `hello.clf`:

```cleaf
fn main(): int {
    return 0;
}
```

Compile and run it:

```sh
./build/cleaf hello.clf -o hello
./build/hello
echo $?   # prints 0
```

## A more complete example

```cleaf
fn add(int a, int b): int {
    return a + b;
}

fn main(): int {
    var x = add(3, 4);
    return x;
}
```

```sh
./build/cleaf example.clf -o example
./build/example
echo $?   # prints 7
```

## Multi-file projects

Cleaf can also compile a project spread across several `.clf` files, using a
Go/Rust-inspired module system (`module`/`import` declarations). Running
`cleaf build` in a directory scans every `.clf` file, resolves dependencies between
modules, and produces a single executable at `build/a.out`. See
[Modules and Imports](/docs/language/modules) for the full syntax and rules.

