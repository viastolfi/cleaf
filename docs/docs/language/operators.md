---
id: operators
title: Operators
sidebar_position: 4
---

# Operators

## Arithmetic operators

| Operator | Description |
|----------|-------------|
| `+`      | Addition |
| `-`      | Subtraction |
| `*`      | Multiplication |
| `/`      | Division |

```cleaf
var a = 10 + 3;   // 13
var b = 10 - 3;   // 7
var c = 10 * 3;   // 30
var d = 10 / 3;   // 3  (integer division)
```

## Comparison operators

Comparison operators are used in conditions. They produce a boolean-like result used by control-flow statements.

| Operator | Description |
|----------|-------------|
| `==`     | Equal |
| `!=`     | Not equal |
| `<`      | Less than |
| `<=`     | Less than or equal |
| `>`      | Greater than |
| `>=`     | Greater than or equal |

```cleaf
if (a == b) { ... }
if (x < 10) { ... }
```

## Unary operators

| Operator | Syntax | Description |
|----------|--------|-------------|
| `-`      | `-x`   | Arithmetic negation |
| `!`      | `!x`   | Logical NOT |
| `++`     | `++x`  | Pre-increment (increment then use) |
| `--`     | `--x`  | Pre-decrement (decrement then use) |
| `++`     | `x++`  | Post-increment (use then increment) |
| `--`     | `x--`  | Post-decrement (use then decrement) |

```cleaf
var a = 5;
++a;    // a is now 6
a++;    // a is now 7
--a;    // a is now 6
var b = -a;   // b is -6
```

## Assignment operator

```cleaf
x = expression;
```

Assignment is an expression and can be used in expression contexts.

## Operator precedence

From highest to lowest:

1. Unary: `-`, `!`, `++`, `--`
2. Multiplicative: `*`, `/`
3. Additive: `+`, `-`
4. Comparison: `<`, `<=`, `>`, `>=`
5. Equality: `==`, `!=`
6. Assignment: `=`

Use parentheses to override precedence:

```cleaf
var x = (a + b) * c;
```
