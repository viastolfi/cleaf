---
id: arrays
title: Arrays
sidebar_position: 7
---

# Arrays

Arrays are fixed-size sequences of elements of the same type.

## Declaration

```cleaf
<type>[<length>] <name>;                    // zero-initialized
<type>[<length>] <name> = { <values> };     // with initializer
```

The length must be a compile-time constant integer literal.

### Examples

```cleaf
int[4]  scores;                     // 4 ints, zero-initialized
u8[3]   rgb = { 255, 128, 0 };      // 3 u8 values
int[2]  pair = { 10, 20 };
```

## Initializer

A composite literal `{ v0, v1, ... }` initializes the array in order, from index 0.
The number of values in the literal must match the declared length.

```cleaf
int[3] a = { 1, 2, 3 };
// a[0] == 1, a[1] == 2, a[2] == 3
```

## Element access

Use `[<index>]` to read an element. The index can be a literal or any integer expression.

```cleaf
int[3] a = { 10, 20, 30 };

var first  = a[0];     // 10
var second = a[1];     // 20

int i = 2;
var third  = a[i];     // 30
```

## Element assignment

Use `[<index>]` on the left-hand side of an assignment to write to an element.

```cleaf
int[3] a = { 1, 2, 3 };
a[0] = 99;     // a is now { 99, 2, 3 }
a[2] = a[1];   // a is now { 99, 2, 2 }
```

## Memory model

When an array variable is declared, the compiler allocates its data **on the heap** via `mmap`.
The stack only holds an 8-byte **pointer** to that memory.

Element access is compiled to an offset read or write:
- reading `a[i]` → `mov dst, [base + i * element_size]`
- writing `a[i] = v` → `mov [base + i * element_size], v`

Because the stack only stores a pointer, passing an array to a function passes that pointer —
not a copy of the data. Modifications inside the callee are visible to the caller.

## Iteration

Arrays are commonly iterated with a `for` loop:

```cleaf
fn sum_array(): int {
    int[5] nums = { 1, 2, 3, 4, 5 };
    var total = 0;
    for (var i = 0; i < 5; ++i) {
        total = total + nums[i];
    }
    return total;
}
```

## Supported element types

All primitive types can be used as the element type:

| Element type | Element size |
|--------------|-------------|
| `int`        | 4 bytes     |
| `u8`         | 1 byte      |
| `u16`        | 2 bytes     |
| `u64`        | 8 bytes     |

```cleaf
u8[8]  bytes  = { 0, 1, 2, 3, 4, 5, 6, 7 };
u64[2] big    = { 1000000, 2000000 };
```

## Semantic rules

- The length must be a positive integer literal — expressions or variables are not allowed.
- The initializer must provide exactly as many values as the declared length.
- Indices are not bounds-checked at runtime.
- Array variables are always pointer-sized (8 bytes) on the stack regardless of element type or length.
- Arrays cannot be assigned to each other with `=` — element-by-element assignment must be done manually.
