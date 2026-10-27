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

## Importing modules

```cleaf
// main.clf
module main

import math

fn main(): int {
    return math::add(3, 4);
}
```

An import names a module, not an individual function. Exported functions and types
remain qualified by the local module name:

```cleaf
import math

fn example(): int {
    return math::add(3, 4);
}
```

For a nested module, provide an alias. The alias is the qualifier used for both
functions and types:

```cleaf
module main

import std::io as io

fn main(): int {
    io::print("Hello");
    return 0;
}
```

The full path (`std::io`) selects the module in the project; `io` is the local
name used by this file. Nested module imports without `as` are rejected. An alias
cannot be reused by another import in the same module.

An import does not make functions available as bare names:

```cleaf
import math

fn example(): int {
    math::add(3, 4); // valid
    add(3, 4);       // error: imported functions require the module qualifier
    return 0;
}
```

The same qualification applies to types. A type alias declared in another module
can be used in a variable, parameter, return type, or local alias:

```cleaf
// vector.clf
module vector

type ptr Vector;

fn create(): Vector {
    return alloc(8);
}
```

```cleaf
// main.clf
module main

import vector

type vector::Vector LocalVector;

fn use_vector(value vector::Vector): vector::Vector {
    return value;
}
```

Here `vector::Vector` names the exported type, while `LocalVector` is a local
alias for it. For nested modules, use the import alias in the type name too
(for example, `io::Buffer` after `import std::io as io`).

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

import math

fn main(): int {
    return math::helper(); // error: helper is not exported
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
- an import refers to an unknown module,
- a qualified call or type refers to a name that the module does not export,
- a nested module import has no alias or an alias is reused,
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

:::note
Type aliases can be imported and used by name. Imported `struct` member layouts
are not currently exported, so accessing fields of a struct declared in another
module is not yet supported.
:::
