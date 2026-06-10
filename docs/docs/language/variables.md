---
id: variables
title: Variables
sidebar_position: 2
---

# Variables

## Declaration

Variables are declared with either an explicit type or `var` for inference.

```cleaf
int x = 10;      // explicitly typed, initialized
var y = x + 5;   // type inferred from the expression
int z;           // explicitly typed, zero-initialized
```

## Constant variables

Adding `!` after the type marks a variable as **constant** — it cannot be reassigned after initialization.

```cleaf
int! max = 100;
max = 200;  // error: cannot assign to constant variable
```

The `!` qualifier is also valid with type inference:

```cleaf
var! pi = 314;
```

## Scope

Variables follow standard block scoping. A variable declared inside a block (`{ }`) is not visible outside of it.

```cleaf
fn example(): int {
    int x = 1;
    if (x == 1) {
        int y = 2;  // y is only visible inside this block
    }
    // y is not accessible here
    return x;
}
```

For-loop init variables are scoped to the loop body:

```cleaf
for (var i = 0; i < 10; ++i) {
    // i is visible here
}
// i is not accessible here
```

## Reserved names

The following identifiers are reserved and cannot be used as variable names:

`int`, `u8`, `u16`, `u64`, `var`, `fn`, `return`, `if`, `else`, `while`, `for`, `struct`, `asm`

## Examples

```cleaf
fn main(): int {
    int a = 10;
    var b = a + 5;
    int! constant = 42;

    a = 20;         // ok
    b = 30;         // ok
    // constant = 0; // error

    return a + b;
}
```
