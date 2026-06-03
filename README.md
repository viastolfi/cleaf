# Cleaf

> Work in progress. The language and compiler are not feature-complete. A number of constructs described here
> may not yet produce correct output, and breaking changes to the syntax or pipeline are expected.

Cleaf is a small, statically typed compiled language targeting x86-64 Linux. Source files use the `.clf` extension.
The compiler is written in C and produces native executables via NASM and the system linker.

## Motivation

Cleaf is a personal project to explore compiler construction from scratch, covering the full pipeline
from lexing to native code generation without relying on LLVM or any other backend framework.

The language itself is intentionally simple and C-like, with a focus on having a clean, readable syntax
while still being compiled to native machine code.

## Dependencies

To build the compiler itself:
- `gcc`

To compile `.clf` source files with the resulting binary:
- `nasm`
- `ld` (from binutils)

## Build and Run

```sh
make              # build ./build/cleaf
./build/cleaf <source.clf>           # compile to a.out
./build/cleaf <source.clf> -o <out>  # compile with a custom output name
./build/cleaf <source.clf> -v        # show each compilation phase and its result
./build/cleaf <source.clf> -V        # same as -v, and dump AST, HIR, and generated assembly
```

## Examples

### Variables

Cleaf supports explicit typing and type inference via `var`:

```
fn main(): int {
    int a = 10;
    var b = a + 5;
    return b;
}
```

### Control flow

```
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

### Functions

```
fn add(int a, int b): int {
    return a + b;
}

fn main(): int {
    var x = add(3, 4);
    return x;
}
```

### Structs

```
struct v2 {
    int x;
    int y;
}

fn main(): int {
    v2 point = { .x = 3, .y = 4 };
    var val = point.x;
    return val;
}
```

## Current state

- [x] Lexer
- [x] Parser
  - [x] Functions with typed parameters and return types
  - [x] Typed variable declarations and `var` inference
  - [x] Struct definitions
  - [x] Composite literal initialization (`{ .field = value }`)
  - [x] Member access
  - [x] Control flow: `if`/`else`, `while`, `for`
  - [x] Arithmetic and comparison expressions
  - [x] Unary operators (`++`, `--`, `-`, `!`)
  - [x] Function calls
  - [ ] Arrays
  - [ ] Additional primitive types (floats, strings, booleans)
- [x] Semantic analysis
  - [x] Type checking
  - [x] Symbol resolution (undefined variables and functions)
  - [x] Function signature validation (argument count and types)
  - [x] Variable redefinition detection
  - [x] Return type checking
  - [x] Reserved keyword enforcement
  - [x] Struct type resolution
  - [ ] Arrays
  - [ ] Additional primitive types
- [x] HIR lowering
  - [x] Arithmetic and comparisons
  - [x] Control flow
  - [x] Function calls
  - [x] Struct field access
- [x] Code generation (x86-64 Linux, NASM syntax)
  - [x] Arithmetic
  - [x] Control flow
  - [x] Function calls and stack management
  - [x] Struct field access
- [ ] Memory safety (garbage collection or ownership model, not yet decided)
- [ ] Standard library
- [ ] Multiple source files
- [ ] Arrays
- [ ] Additional primitive types

## Test coverage

The test suite contains 120 test cases totalling 248 assertions spread across the four compiler passes,
plus around 20 additional fixtures used for memory safety validation with Valgrind.

| Suite    | Test cases | Assertions |
|----------|-----------|------------|
| Parser   | 26        | 129        |
| Semantic | 55        | 80         |
| HIR      | 20        | 20         |
| Codegen  | 19        | 19         |
| **Total**| **120**   | **248**    |

The semantic pass has the most coverage, reflecting the variety of error cases it handles.
The parser and HIR passes cover the main language constructs. The codegen tests compare
the full generated assembly output against expected fixtures for each construct.

Note that these numbers give a rough indication of coverage — there is no formal coverage
measurement tool in place yet.

## Running tests

```sh
make test           # run all test suites
make ast-test       # parser tests only
make semantic-test  # semantic analysis tests only
make hir-test       # HIR lowering tests only
make codegen-test   # code generation tests only
make asan-test      # all tests with AddressSanitizer and UBSan
make valgrind-test  # memory checks on a suite of ~20 .clf fixtures
```
