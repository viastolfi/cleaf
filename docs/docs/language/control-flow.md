---
id: control-flow
title: Control Flow
sidebar_position: 5
---

# Control Flow

## if / else

```cleaf
if (<condition>) {
    // executed when condition is true
}

if (<condition>) {
    // then branch
} else {
    // else branch
}
```

### Example

```cleaf
fn classify(int n): int {
    if (n > 0) {
        return 1;
    } else {
        return 0;
    }
}
```

Variables declared inside an `if` block are scoped to that block.

## while

Executes the body repeatedly as long as the condition is true.

```cleaf
while (<condition>) {
    // body
}
```

### Example

```cleaf
fn count_down(int n): int {
    while (n > 0) {
        n = n - 1;
    }
    return n;
}
```

## for

The `for` loop has three parts: an initializer, a condition, and a loop expression (typically an increment).

```cleaf
for (<init>; <condition>; <loop>) {
    // body
}
```

- `<init>` can be a variable declaration (`var i = 0` or `int i = 0`) or an expression.
- `<condition>` is evaluated before each iteration; the loop exits when it becomes false.
- `<loop>` is evaluated after each iteration (before re-checking the condition).

### Example

```cleaf
fn sum(int n): int {
    var total = 0;
    for (var i = 0; i < n; ++i) {
        total = total + i;
    }
    return total;
}
```

Variables declared in the init expression are scoped to the loop body.

## Nested control flow

Control flow statements can be nested freely:

```cleaf
fn main(): int {
    var result = 0;

    for (var i = 0; i < 10; ++i) {
        if (i == 5) {
            result = result + 1;
        }
    }

    while (result > 0) {
        result = result - 1;
    }

    return result;
}
```
