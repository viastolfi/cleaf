---
id: inline-asm
title: Inline Assembly
sidebar_position: 8
---

# Inline Assembly

Cleaf provides an `asm` statement for embedding raw x86-64 assembly instructions (NASM syntax) directly in your code.

:::caution
Inline assembly bypasses all safety guarantees of the language. Use with care.
:::

## Syntax

```cleaf
asm(
    "<instruction>",
    "<instruction>",
    ...
);
```

Instructions are given as string literals and emitted verbatim into the generated assembly.

## Variable substitution

You can inject a Cleaf variable into an instruction using `%` as a placeholder.
The variable is provided as an additional argument after the instruction string that contains `%`.

```cleaf
asm(
    "<instruction with %>", <variable>,
    ...
);
```

The `%` is replaced by the register or memory location holding the variable's value.

## Examples

### Exit syscall

```cleaf
fn main(): int {
    asm(
        "mov rax, 60",
        "mov rdi, 0",
        "syscall"
    );
}
```

### Passing a variable to a syscall

```cleaf
fn main(): int {
    var exit_code = 42;
    asm(
        "mov rax, 60",
        "mov rdi, %", exit_code,
        "syscall"
    );
}
```

### Accessing a struct field

```cleaf
struct v2 {
    int x;
    int y;
}

fn main(): int {
    v2 pos = { .x = 5, .y = 10 };
    asm(
        "mov rax, 60",
        "mov rdi, %", pos.x,
        "syscall"
    );
}
```

## Semantic rules

- Each `%` placeholder requires exactly one matching variable argument.
- The referenced variable must be defined in the current scope.
- The number of `%` placeholders across all instructions must match the number of variable arguments.
