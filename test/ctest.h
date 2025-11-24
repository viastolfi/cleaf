/*
------------------------------------------------------------------------------
TEST LIB FOR C - ASTOLFI Vincent 2025

<!> WARNING : this lib is greatly inspired by other c test libs like criterion, you should look at those

Usage:

ct_test(suite, name) 
{
  ct_assert(0, "failing test");
  ct_assert_eq("test", "test", "success string equality test");  
}

API:
  before_each(value_type, value_name, entry_definition) 
    -> allow user to define a function that will run before all test suite
    -> value_type: type of the value you want to use in your test
    -> value_name: name of the value you want to use in your test
    -> entry_definition: definition of the entry you want to use in your before_each function
  ct_test(suite, name, <optional> entry) -> interface for the tests function
    -> entry: value that will be used on the before_each function
  ct_assert(expr, name) -> test for bool expression
  ct_assert_eq(x, y, name) -> test for equality between two objects of the same type
  ct_assert_not_null(expr, name) -> test if expr is NULL or not

EXAMPLE:

  ```c
  #define CTEST_LIB_IMPLEMENTATION
  #include "ctest.h"

  ct_test(test, simple) 
  {
    ct_assert(0, "failing test");
    ct_assert_eq("test", "test", "successfull string equality test"); 
    ct_assert_not_null(1, "successfull not null test");
  }
  ```
  No need of main function in your test files, it'll be handled by the test lib directly

  see (https://github.com/viastolfi/clibs/tree/main/ctest/example) for more example
*/

#ifndef CTEST_H
#define CTEST_H

#ifndef CT_PRINT
#include <stdio.h>
#define CT_PRINT printf
#endif // CT_PRINT

#ifndef CT_STRCMP
#include <string.h>
#define CT_STRCMP strcmp
#endif // CT_STRCMP

#ifndef CT_COLOR_SETUP
#define COLOR_RESET  "\033[0m"
#define COLOR_SUCCESS "\033[32m"
#define COLOR_FAIL "\033[31m"
#define COLOR_HINT "\033[33m"
#define COLOR_BOLD  "\e[1m"
#define BOLD_OFF   "\e[m"
#define COLOR_SUM "\033[36m"
#define CT_COLOR_SETUP
#endif // CT_COLOR_SETUP

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef CTEST_LIB_IMPLEMENTATION

static int total = 0;
static int success = 0;

#define before_each(TYPE, VALUE, ...) \
  typedef TYPE T; \
  static T VALUE; \
  static void before_each_function(__VA_ARGS__)


typedef void (*ct_func)(void);
#ifdef CTEST_BEFORE_EACH
typedef void (*ct_before_each_func)(void);
#endif // CTEST_BEFORE_EACH

typedef struct ct_test ct_test;

struct ct_test
{
  const char* suite;
  const char* name;
  ct_func func;
#ifdef CTEST_BEFORE_EACH
  ct_before_each_func before;
#endif // CTEST_BEFORE_EACH
  ct_test* next;
};

static ct_test* ct_test_head = NULL;

static void add_test(ct_test* t) {
  t->next = ct_test_head;
  ct_test_head = t;
}

#ifndef CTEST_BEFORE_EACH
#define ct_test(SUITE, NAME) \
  static void SUITE##_##NAME##_impl(void); \
  static ct_test ct_test_##SUITE##_##NAME = { \
    .suite = #SUITE, \
    .name = #NAME, \
    .func = SUITE##_##NAME##_impl, \
    .next = NULL \
  }; \
  __attribute__((constructor)) \
  static void register_##SUITE##_##NAME(void) { add_test(&ct_test_##SUITE##_##NAME); } \
  static void SUITE##_##NAME##_impl(void) 
#else
#define ct_test(SUITE, NAME, ...) \
  static void SUITE##_##NAME##_impl(void); \
  static void before_##SUITE##_##NAME(void); \
  static ct_test ct_test_##SUITE##_##NAME = { \
    .suite = #SUITE, \
    .name = #NAME, \
    .func = SUITE##_##NAME##_impl, \
    .before = before_##SUITE##_##NAME, \
    .next = NULL \
  }; \
  __attribute__((constructor)) \
  static void register_##SUITE##_##NAME(void) { add_test(&ct_test_##SUITE##_##NAME); } \
  static void before_##SUITE##_##NAME(void) { before_each_function(__VA_ARGS__); } \
  static void SUITE##_##NAME##_impl(void)
#endif // BEFORE_EACH

#define ct_assert(expr, message) \
  do { \
    total++; \
    if (!expr) { \
      CT_PRINT(COLOR_FAIL "[FAIL] " COLOR_RESET); \
      CT_PRINT("(%s:%d) ", __FILE__, __LINE__); \
      CT_PRINT("%s\n", message); \
      CT_PRINT(COLOR_HINT "[HINT] " COLOR_RESET); \
      CT_PRINT(COLOR_SUM COLOR_BOLD "expected :" COLOR_RESET " true\n"); \
      CT_PRINT(COLOR_HINT "[HINT] " COLOR_RESET); \
      CT_PRINT(COLOR_SUM COLOR_BOLD "result   :" COLOR_RESET " false\n"); \
    } else {\
      CT_PRINT(COLOR_SUCCESS "[PASS] " COLOR_RESET); \
      CT_PRINT("(%s:%d) ", __FILE__, __LINE__); \
      CT_PRINT("%s\n", message); \
      success++; \
    } \
  } while (0) 

#define ct_assert_not_null(expr, message) \
  do { \
    total++; \
    if (!(void*)(expr)) { \
      CT_PRINT(COLOR_FAIL "[FAIL] " COLOR_RESET); \
      CT_PRINT("(%s:%d) ", __FILE__, __LINE__); \
      CT_PRINT("%s\n", message); \
      CT_PRINT(COLOR_HINT "[HINT] " COLOR_RESET); \
      CT_PRINT(COLOR_SUM COLOR_BOLD "expected :" COLOR_RESET " not NULL\n"); \
      CT_PRINT(COLOR_HINT "[HINT] " COLOR_RESET); \
      CT_PRINT(COLOR_SUM COLOR_BOLD "result   :" COLOR_RESET " NULL\n"); \
    } else { \
      CT_PRINT(COLOR_SUCCESS "[PASS] " COLOR_RESET); \
      CT_PRINT("(%s:%d) ", __FILE__, __LINE__); \
      CT_PRINT("%s\n", message); \
      success++; \
    } \
  } while(0)

static void ct_assert_eq_core(long long x, long long y, const char* fmt, const char* message, const char* file, int line)
{
  total++;
  if (x == y) {
    CT_PRINT(COLOR_SUCCESS "[PASS] " COLOR_RESET);
    CT_PRINT("(%s:%d) %s\n", file, line, message);
    success++;
  } else {
    CT_PRINT(COLOR_FAIL "[FAIL] " COLOR_RESET);
    CT_PRINT("(%s:%d) %s", file, line, message);
    CT_PRINT(COLOR_HINT "\n[HINT] " COLOR_RESET);
    CT_PRINT(COLOR_BOLD COLOR_SUM "expected : " COLOR_RESET BOLD_OFF);
    CT_PRINT(fmt, y);
    CT_PRINT("\n" COLOR_HINT "[HINT] " COLOR_RESET);
    CT_PRINT(COLOR_BOLD COLOR_SUM "result   : " COLOR_RESET BOLD_OFF);
    CT_PRINT(fmt, x);
    CT_PRINT("\n");
  }
}

static void ct_assert_eq_string(const char* x, const char* y, const char* message, const char* file, int line)
{
  total++;
  if (CT_STRCMP(x, y) == 0) {
    CT_PRINT(COLOR_SUCCESS "[PASS] " COLOR_RESET);
    CT_PRINT("(%s:%d) ", file, line);
    CT_PRINT("%s\n", message);
    success++; 
  }
  else {
    CT_PRINT(COLOR_FAIL "[FAIL] " COLOR_RESET);
    CT_PRINT("(%s:%d) ", file, line); 
    CT_PRINT("%s", message);
    CT_PRINT(COLOR_HINT "\n[HINT] " COLOR_RESET);
    CT_PRINT(COLOR_BOLD COLOR_SUM "expected : " COLOR_RESET BOLD_OFF "\"%s\"\n", y);
    CT_PRINT(COLOR_HINT "[HINT] " COLOR_RESET);
    CT_PRINT(COLOR_BOLD COLOR_SUM "result   : " COLOR_RESET BOLD_OFF "\"%s\"\n", x);
  }
}

static void ct_assert_eq_int(int x, int y, const char* message, const char* file, int line)
{
  ct_assert_eq_core((long long)x, (long long)y, "%d", message, file, line);
}

static void ct_assert_eq_unsigned_int(unsigned int x, unsigned int y, const char* message, const char* file, int line)
{
  ct_assert_eq_core((long long)x, (long long)y, "%u", message, file, line);
}

static void ct_assert_eq_long(long x, long y, const char* message, const char* file, int line)
{
  ct_assert_eq_core((long long)x, (long long)y, "%ld", message, file, line);
}

#define ct_assert_eq(x, y, message) \
  _Generic((x), \
    int: ct_assert_eq_int, \
    char*: ct_assert_eq_string, \
    const char*: ct_assert_eq_string, \
    unsigned int: ct_assert_eq_unsigned_int, \
    long: ct_assert_eq_long, \
    default: ct_assert_eq_int \
  )(x, y, message, __FILE__, __LINE__)

int main(void) 
{
  ct_test* t = ct_test_head;
  if (!t) {
    CT_PRINT(COLOR_HINT "No tests registered!\n" COLOR_RESET);
    return 1;
  }

  ct_test* reversed = NULL;
  while (t) {
    ct_test* next = t->next;
    t->next = reversed;
    reversed = t;
    t = next;
  }

  t = reversed;
  while (t) {
    CT_PRINT("[" COLOR_SUM "----" COLOR_RESET "] " COLOR_BOLD "%s::%s" BOLD_OFF"\n", t->suite, t->name);
#ifdef CTEST_BEFORE_EACH
    t->before();
#endif // CTEST_BEFORE_EACH
    t->func();
    t = t->next;
  }

  CT_PRINT("[" COLOR_SUM "====" COLOR_RESET "] " COLOR_BOLD "Summary : " COLOR_HINT "%d " COLOR_RESET COLOR_BOLD "Test | " COLOR_SUCCESS "%d " COLOR_RESET COLOR_BOLD "Success | " COLOR_FAIL "%d " COLOR_RESET COLOR_BOLD "Fail\n" BOLD_OFF, total, success, total - success);

  return total - success;
}

#endif // CT_TEST_IMPLEMENTATION

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CTEST_H
