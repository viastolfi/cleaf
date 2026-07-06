#ifndef ASSERTION_H
#define ASSERTION_H

#define CLEAF_ASSERT(expr, msg)             \
  do {                                      \
    if (!(expr)) {                          \
      fprintf(stderr,                       \
          "Assertion failed: %s\n"          \
          "Reason: %s\n"                    \
          " File: %s\n"                     \
          " Line: %d\n",                    \
          #expr, msg,  __FILE__, __LINE__); \
      abort();                              \
    }                                       \
  } while(0)                                \

#endif // ASSERTION_H
