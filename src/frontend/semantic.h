#ifndef SEMANTIC_H
#define SEMANTIC_H

#define DA_LIB_IMPLEMENTATION
#include "../thirdparty/da.h"
#include "ast_definition.h"

#include <string.h>
#include <stdlib.h>

typedef struct 
{
  char* name;
  type_t return_type;
  type_t* params_type;
  // TODO: add var for error logging info
} function_symbol_t;

typedef struct
{
  function_symbol_t* items;
  size_t count;
  size_t capacity;
} function_symbol_table_t;

void semantic_analyze(declaration_array* ast);
function_symbol_table_t* semantic_load_function_definition(declaration_array* ast);

#endif // SEMANTIC_H
