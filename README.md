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
./build/cleaf <source.clf>           # compile to build/a.out
./build/cleaf <source.clf> -o <out>  # compile with a custom output name (build/<out>)
./build/cleaf <source.clf> -v        # show each compilation phase and its result
./build/cleaf <source.clf> -V        # same as -v, and dump AST, HIR, and generated assembly
./build/cleaf build                  # compile a multi-file module project (see below)
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

### Modules and imports

Multi-file projects use a Go/Rust-inspired module system, compiled with `cleaf build`:

```
// math.clf
module math

internal fn helper(): int { return 41; }
fn add(): int { return helper(); }
```

```
// main.clf
module main

import math::add

fn main(): int {
    return add();
}
```

`cleaf build` scans every `.clf` file in the current directory, resolves the module
dependency graph, and links everything into a single executable under `build/`.
`internal` functions are only visible within their own module.

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
- [x] Multiple source files
  - [x] `module`/`import` declarations (nested module paths via `::`)
  - [x] `internal` visibility restriction
  - [x] `cleaf build` — project-wide scan, dependency graph, topological compilation
  - [x] Cross-module name mangling + multi-object codegen/link
- [ ] Arrays
- [ ] Additional primitive types

## Test coverage

The test suite contains 249 test cases totalling 563 assertions spread across the compiler
passes and the module build pipeline, plus a set of end-to-end integration tests and
around 20 additional fixtures used for memory safety validation with Valgrind.

| Suite               | Test cases | Assertions |
|---------------------|-----------|------------|
| Parser (AST)        | 64        | 324        |
| Semantic            | 106       | 160        |
| HIR                 | 35        | 35         |
| HIR name mangling   | 3         | 3          |
| Codegen             | 34        | 34         |
| Build (imports)     | 7         | 7          |
| **Total**           | **249**   | **563**    |

The semantic pass has the most coverage, reflecting the variety of error cases it handles.
The parser and HIR passes cover the main language constructs. The codegen tests compare
the full generated assembly output against expected fixtures for each construct.

On top of the suites above, `test/integration_case/` holds end-to-end multi-module
projects exercised via `make integration-test`: a correct 2-module build whose executable
is run and checked for the expected exit code, plus three failure scenarios (`internal`
violation, import cycle, missing `main` module).

Note that these numbers give a rough indication of coverage — there is no formal coverage
measurement tool in place yet.

## Running tests

```sh
make test              # run all test suites, including integration tests
make ast-test           # parser tests only
make semantic-test      # semantic analysis tests only
make hir-test           # HIR lowering tests only
make hir-module-test    # HIR name mangling tests only
make codegen-test       # code generation tests only
make build-test         # multi-module import/semantic tests only
make integration-test   # end-to-end `cleaf build` tests (requires nasm/ld)
make asan-test          # all tests with AddressSanitizer and UBSan
make valgrind-test      # memory checks on single-file and multi-module fixtures
```
