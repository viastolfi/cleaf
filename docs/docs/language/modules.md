---
id: modules
title: Modules and Imports
sidebar_position: 10
---

# Modules and Imports

Cleaf supports splitting a program across multiple `.clf` files using a Go/Rust-inspired
module system, compiled with the `cleaf build` command.

## Declaring a module

Every file that is part of a multi-file project starts with a `module` declaration.
Module paths can be nested with `::`:

```cleaf
// math.clf
module math

fn add(int a, int b): int {
    return a + b;
}
```

```cleaf
// io/console.clf
module std::io
```

Several files may declare the same module name — they are merged together, Go-style.

## The `main` module

Exactly one module named `main` must exist in the project, and it must contain a
`fn main(): int` — this is the program's entry point. `cleaf build` fails with an error
if no `main` module is found.

## Importing symbols

```cleaf
// main.clf
module main

import math::add           // call as: add(3, 4)  or  math::add(3, 4)
import math::add as madd   // call as: madd(3, 4)

fn main(): int {
    var x = add(3, 4);
    var y = math::add(1, 2);
    return x;
}
```

- `import <module>::<symbol>` brings a single function into scope.
- An optional `as <name>` renames it locally.
- At the call site, a qualifier (`math::add(...)`) is allowed and must match the last
  segment of the imported module's path — at most one level of qualification is
  supported (`io::print()`, not `std::io::print()`).

## Restricting visibility with `internal`

A function marked `internal` is only visible within its own module and cannot be
imported from anywhere else:

```cleaf
module math

internal fn helper(): int {
    return 41;
}

fn add(): int {
    return helper(); // OK: same module
}
```

```cleaf
module main

import math::helper // error: cannot import an internal function

fn main(): int {
    return helper();
}
```

## Building a multi-module project

```sh
cleaf build
```

`cleaf build` scans every `.clf` file in the current directory (recursively), resolves
the dependency graph between modules, and compiles them in dependency order (leaves
first). Compilation fails with an error if:

- no `main` module is found,
- an import refers to an unknown module or symbol,
- an import targets an `internal` function from another module,
- the dependency graph contains a cycle.

Each module is compiled to its own object file under `build/` (e.g. `build/math.o`,
`build/main.o`) and kept on disk after linking. The final executable is also written to
`build/` (`build/a.out` by default).

## How module boundaries are erased

Semantic analysis is the only compiler pass aware of module boundaries. Once a program
passes semantic analysis, function names are mangled into flat, globally unique symbols
before HIR lowering:

```
"main" in module "main"    ->  "start"          (maps to the `_start` entry point)
"foo"  in module "main"    ->  "main__foo"
"add"  in module "math"    ->  "math__add"
"add"  in module "std::io" ->  "std__io__add"
```

From HIR lowering onward (codegen, assembly, linking), Cleaf only ever deals with these
mangled names and plain `global`/`extern` directives — there is no notion of "module"
below the semantic pass.

Single-file compilation (`cleaf <file.clf>`, no `module` declaration) is unaffected:
function names are left untouched, exactly as before the module system was introduced.
