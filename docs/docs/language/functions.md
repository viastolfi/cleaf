---
id: functions
title: Functions
sidebar_position: 3
---

# Functions

## Syntax

```cleaf
fn <name>(<params>): <return_type> {
    <body>
}
```

A function without a return type can omit the `: <return_type>` part.

## Examples

```cleaf
// No parameters, no return type
fn greet() {
    asm("mov rax, 60", "mov rdi, 0", "syscall");
}

// Parameters and return type
fn add(int a, int b): int {
    return a + b;
}

// Type inference for the return value
fn compute(int x): int {
    var result = x * 2;
    return result;
}
```

## Parameters

Parameters are declared as `<type> <name>` and separated by commas.
All parameters are passed by value.

```cleaf
fn multiply(int a, int b): int {
    return a * b;
}
```

Multiple parameter types are supported:

```cleaf
fn pack(u8 low, u16 mid, u64 high): u64 {
    return high;
}
```

## Return statement

Use `return` to return a value from a function:

```cleaf
fn double(int n): int {
    return n * 2;
}
```

The compiler checks that the returned type matches the declared return type.
Returning a narrower type into a wider return type is allowed (promotion).

## Function calls

```cleaf
fn main(): int {
    var x = add(3, 4);
    return x;
}
```

Function calls can appear as expressions (capturing the return value) or as standalone statements (discarding it).

## Restrictions

- Recursive calls are syntactically valid, but full stack safety is not yet guaranteed.
- Functions must be declared before they are called (single-pass compilation).
- The `main` function is the conventional entry point — it must return `int`.
