# Valgrind test case

The goal of this dir is to create cleaf program that valgrind will test to verify that there is no memory leaks.

Here is the strategy:

- maintain some base cleaf program that runs completely to check memory safety in a normal environment
- test every early compiler termination by adding cleaf files that should not compile to check that not running the compiler entirely don't cause memory leak
