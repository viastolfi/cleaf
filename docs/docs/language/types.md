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
| `string` | 8 bytes | Address of a string literal |

:::note
`u32`, floats, and booleans are planned but not yet implemented.
:::

## Strings

String literals use double quotes and can initialize explicitly typed or
inferred variables:

```cleaf
string greeting = "Hello";
var language = "Cleaf";  // inferred as string
```

The compiler emits string literal data as null-terminated data in the program's
`.data` section. A string variable stores the address of that data. String
binary operations are not supported yet; see [Operators](./operators).

## Type inference

Use `var` to let the compiler infer the type from the initializer expression:

```cleaf
var x = 42;       // inferred as int
var y = x + 10;   // inferred from expression result
```

`var` requires an initializer — it cannot be used to declare an uninitialized variable.

## Type aliases

Use `type <existing-type> <new-name>;` to give an existing type another name:

```cleaf
type ptr Vector;

fn make_buffer(): Vector {
    Vector buffer = alloc(64);
    return buffer;
}
```

The alias has the same underlying type as its target. A type alias declared in
another module can be named through that module's import qualifier:

```cleaf
module main

import vector

type vector::Vector LocalVector;

fn make_vector(): vector::Vector {
    LocalVector value = vector::create();
    return value;
}
```

For a nested module, import it with an alias and use that alias in the type name:

```cleaf
module main

import std::geometry as geometry

fn use_point(): int {
    geometry::Point point;
    return 0;
}
```

Imported type names can be used in variable declarations, function parameters,
function return types, and local type aliases. See
[Modules and Imports](./modules) for module import and visibility rules.

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
