---
id: comments
title: Comments
sidebar_position: 9
---

# Comments

Cleaf supports two comment styles, identical to C.

## Single-line comments

Start with `//` and extend to the end of the line.

```cleaf
// This is a single-line comment
fn main(): int {
    var x = 10;  // inline comment
    return x;
}
```

## Multi-line comments

Delimited by `/*` and `*/`. Can span multiple lines.

```cleaf
/*
 * This function computes
 * the sum of two integers.
 */
fn add(int a, int b): int {
    return a + b;
}
```

Multi-line comments do **not** nest:

```cleaf
/* outer /* inner */ this is still inside the outer comment */
```
