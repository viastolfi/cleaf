#ifndef STRING_BUILDER_H
#define STRING_BUILDER_H

#define DA_LIB_IMPLEMENTATION
#include "da.h"

#include <stdarg.h>
#include <stdio.h>

typedef struct 
{
  char* items;
  size_t count;
  size_t capacity;
} string_builder_t;

inline static void sb_append_fmt(string_builder_t* sb, const char* fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  int required = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  DA_ASSERT(required >= 0);

  da_reserve(sb, sb->count + required);

  va_start(args, fmt);
  vsnprintf(&sb->items[sb->count], 
      required + 1,
      fmt,
      args);
  va_end(args);

  sb->count += required;
}

#endif // STRING_BUILDEr_H
