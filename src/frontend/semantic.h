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
  char** items;
  size_t count;
  size_t capacity;
} function_params_name_t;

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

typedef struct 
{
  error_context_t* error_ctx;

  declaration_array* ast;

  function_symbol_table_t* fst;
} semantic_analyzer_t;

int is_param_name_declared(function_params_name_t* fpn, const char* name);

void semantic_analyze(semantic_analyzer_t* analyzer);
void semantic_load_function_definition(semantic_analyzer_t* analyzer);
void semantic_free_function_definition(semantic_analyzer_t* analyzer);

#endif // SEMANTIC_H
