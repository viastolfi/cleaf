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
./build/cleaf <source.clf>           # compile to ./a.out
./build/cleaf <source.clf> -o <out>  # compile with a custom output name
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
./hello
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
./example
echo $?   # prints 7
```
