---
id: types
title: Types
sidebar_position: 1
---

# Types

Cleaf is **statically typed**. Every variable and function parameter has a type known at compile time.

## Primitive types

| Type  | Size    | Description |
|-------|---------|-------------|
| `int` | 4 bytes | Signed 32-bit integer |
| `u8`  | 1 byte  | Unsigned 8-bit integer (0–255) |
| `u16` | 2 bytes | Unsigned 16-bit integer (0–65535) |
| `u64` | 8 bytes | Unsigned 64-bit integer |

:::note
`u32`, floats, booleans, and strings are planned but not yet implemented.
:::

## Type inference

Use `var` to let the compiler infer the type from the initializer expression:

```cleaf
var x = 42;       // inferred as int
var y = x + 10;   // inferred from expression result
```

`var` requires an initializer — it cannot be used to declare an uninitialized variable.

## Type narrowing (promotion)

When assigning a value of a wider type to a narrower variable, the compiler reports an error if the value exceeds the target type's range.
Assigning a narrower type to a wider variable is always allowed.

```cleaf
u8  a = 200;    // ok  — 200 fits in u8
u8  b = 300;    // error — 300 overflows u8
u16 c = 200;    // ok  — u8 value fits in u16
```

## Custom types (structs)

User-defined aggregate types are defined with the `struct` keyword.
See [Structs](./structs) for details.

```cleaf
struct Point {
    int x;
    int y;
}
```
