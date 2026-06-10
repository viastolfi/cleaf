---
id: structs
title: Structs
sidebar_position: 6
---

# Structs

Structs are user-defined aggregate types that group multiple fields together.

## Definition

```cleaf
struct <Name> {
    <type> <field>;
    ...
}
```

### Example

```cleaf
struct Point {
    int x;
    int y;
}
```

## Declaration

A struct variable is declared using the struct name as the type.

```cleaf
Point p;                            // zero-initialized
Point q = { .x = 3, .y = 4 };      // designated initializer
```

### Memory model

When a struct variable is declared, the compiler emits a `mmap` call to allocate the struct's
data **on the heap**. The stack only holds a **pointer** to that memory.
As a result, loading a struct variable into an expression gives you its pointer,
and all field accesses go through that pointer with a computed offset.

### Designated initializer

Fields are initialized by name using the `.field = value` syntax.
Fields can appear in any order — the compiler maps each to the correct memory offset.

```cleaf
struct Color {
    u8  r;
    u8  g;
    u8  b;
}

Color red  = { .r = 255, .g = 0, .b = 0 };
Color blue = { .b = 255, .r = 0, .g = 0 };  // order doesn't matter
```

## Member access

Use `.` to read a field of a struct variable.
Internally this compiles to a pointer-offset read (`mov dst, [ptr + offset]`).

```cleaf
struct Point {
    int x;
    int y;
}

fn main(): int {
    Point p = { .x = 10, .y = 20 };
    var val = p.x;   // val == 10
    return val;
}
```

## Passing structs to functions

Because a struct variable is internally a pointer, passing it to a function passes that
**pointer** — not a copy of the data. Modifications to struct fields inside the callee
are visible to the caller.

```cleaf
struct Counter {
    int value;
}

fn increment(Counter c): int {
    // c is a pointer; c.value modifies the original struct
    return c.value;
}
```

## Constant struct variables

You can declare a struct variable as constant with `!`:

```cleaf
Point! origin = { .x = 0, .y = 0 };
// origin.x = 1;  // error — cannot assign to constant variable
```

## Semantic rules

- All fields referenced in a designated initializer must exist in the struct definition.
- The number of initializer entries must match the number of fields.
- Duplicate field names in the initializer are rejected.
- Assigning a value that overflows a field's type is an error.
