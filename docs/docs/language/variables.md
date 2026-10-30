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
string name = "Cleaf";
var greeting = "Hello"; // inferred as string
```

String variables can be initialized from string literals. The variable holds
the address of the literal's generated data in the program's `.data` section.
String-returning functions also preserve the byte length used by `len`.

## Multiple declarations

Separate names with commas to declare several variables in one statement:

```cleaf
fn coordinates(): (int, int) {
    return 10, 20;
}

fn main(): int {
    var x, y = coordinates();
    return x + y;
}
```

There is one shared initializer, evaluated **once**, not once per variable.
Results are associated with names from left to right. The initializer must
produce exactly as many values as there are names; a single value is not
broadcast to every variable.

With `var`, each binding is inferred separately using the usual inference rules.
The results can have different types:

```cleaf
fn describe(): (int, string) {
    return 7, "ready";
}

fn main(): int {
    var code, message = describe();
    return code + len(message);
}
```

An explicit type and the optional `!` modifier apply to every name:

```cleaf
int x, y = coordinates();   // both bindings have type int
var! a, b = coordinates();  // both bindings are constant
int first, second;         // two declarations without an initializer
```

Every name must be valid and distinct; duplicate names and redefinitions are
errors. The same count and type checks apply when initializing several variables
in a `for` loop.

These are declarations, not general multiple assignment or tuple unpacking:

```cleaf
// var x, y = 1;       // error: one value for two variables
// var x = coordinates(); // error: two values for one variable
// var x, y = 1, 2;    // unsupported: use a multiple-result call
// x, y = coordinates(); // unsupported: reassignment needs separate statements
```

See [Multiple return values](./functions#multiple-return-values) for forwarding
results and the current register limit.

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

`int`, `u8`, `u16`, `u64`, `string`, `var`, `fn`, `return`, `if`, `else`, `while`, `for`, `struct`, `asm`

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
