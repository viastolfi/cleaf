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

## Built-in functions

### `alloc`

`alloc` accepts a byte count and returns a `ptr` to heap memory:

```cleaf
var buffer = alloc(16);
```

The size is currently expected to be an integer literal.

### `free`

`free` accepts an address and its size in bytes, then returns an `int` result
from the deallocation syscall. The address can be a `ptr` or a custom struct
value; struct variables are heap-allocated and act as pointers. `free` works as
an expression (to capture its result) or as a standalone statement:

```cleaf
struct Point {
    int x;
    int y;
}

fn main(): int {
    Point point = { .x = 3, .y = 4 };
    int status = free(point, 8);
    return status;
}
```

The size must currently be an integer literal and should match the allocation
size in bytes.

### `len`

`len` accepts exactly one `string` and returns its length as an `int`. The length
counts bytes in the string contents and excludes the trailing null byte used in
the generated data:

```cleaf
fn string_length(string value): int {
    return len(value);
}

fn main(): int {
    string greeting = "Hello";
    int literal_length = len("Cleaf");  // 5
    int greeting_length = len(greeting); // 5
    int parameter_length = string_length(greeting); // 5
    return 0;
}
```

### `sizeof`

`sizeof` accepts the name of a type and returns its size in **bytes** as an
`int`. The result is computed at compile time. Primitive types, structs, and
type aliases are supported:

```cleaf
struct Point {
    int x;
    int y;
}

type int Integer;

fn main(): int {
    int int_size = sizeof(int);       // 4 bytes
    int point_size = sizeof(Point);   // 8 bytes
    int alias_size = sizeof(Integer); // 4 bytes
    return 0;
}
```

Struct sizes are currently the sum of their field sizes; no alignment padding
is added. `sizeof` expects a type name, not a variable or a value.

## Restrictions

- Recursive calls are syntactically valid, but full stack safety is not yet guaranteed.
- Functions must be declared before they are called (single-pass compilation).
- The `main` function is the conventional entry point — it must return `int`.
