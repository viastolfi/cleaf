#ifndef SEMANTIC_H
#define SEMANTIC_H

#define DA_LIB_IMPLEMENTATION
#include "../thirdparty/da.h"
#include "ast_definition.h"
#include "../frontend/error.h"

#include <string.h>
#include <stdlib.h>

typedef struct 
{
  char* name;
  type_kind return_type;
  type_kind* params_type;
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
void semantic_free_function_definition(function_symbol_table_t* fst);

#endif // SEMANTIC_H
