#ifndef COMPILER_DEFINITION_H
#define COMPILER_DEFINITION_H

#include <stdlib.h>
#include <stddef.h>

#define DA_LIB_IMPLEMENTATION
#include "thirdparty/da.h"
#include "thirdparty/error.h"
#include "frontend/ast.h"
#include "middleend/hir.h"
#include "middleend/ir_definition.h"

typedef struct {
  char** items;
  size_t count;
  size_t capacity;
} compiled_files_array;

typedef struct {
  char*             file_path;   // not owned (points into files array)
  char*             source;      // owned
  int               source_len;
  error_context_t   error_ctx;
  parser_t          parser;
  declaration_array program;
} module_unit_t;

typedef struct {
  module_unit_t** items;
  size_t count;
  size_t capacity;
} module_unit_array;

typedef struct {
  compiled_files_array files;
  module_unit_array    units;
  IR_function_array*   hir_program;
  const char*          output;
} compiler_resources_t;

void module_unit_free(module_unit_t* unit);
void compiler_resources_free(compiler_resources_t* res);

#endif // COMPILER_DEFINITION_H

