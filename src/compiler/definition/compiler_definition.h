#ifndef COMPILER_DEFINITION_H
#define COMPILER_DEFINITION_H

#include <stdlib.h>
#include <stddef.h>

#define DA_LIB_IMPLEMENTATION
#include "thirdparty/da.h"
#include "frontend/ast.h"
#include "middleend/hir.h"
#include "middleend/ir_definition.h"

typedef struct {
  char* text;
  parser_t parser;
  declaration_array program;
  IR_function_array* hir_program;
} compiler_resources_t;

void compiler_resources_free(compiler_resources_t* res);

#endif // COMPILER_DEFINITION_H

