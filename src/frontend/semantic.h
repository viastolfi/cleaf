#ifndef SEMANTIC_H
#define SEMANTIC_H

#define DA_LIB_IMPLEMENTATION
#include "../thirdparty/da.h"
#include "ast_definition.h"
#include "error.h"
#include "../thirdparty/hashmap.h"

#include <string.h>
#include <stdlib.h>

typedef struct 
{
  type_kind return_type;

  char** params_name;
  type_kind* params_type;
  size_t params_count;
} function_symbol_t;

typedef struct 
{
  error_context_t* error_ctx;

  declaration_array* ast;

  hashmap_t* function_symbols;
} semantic_analyzer_t;

int string_array_contains(char** source, size_t source_len, const char* name);

void semantic_analyze(semantic_analyzer_t* analyzer);
void semantic_check_scope(semantic_analyzer_t* analyzer, statement_block_t* func, hashmap_t* known_symbols);
void semantic_load_function_definition(semantic_analyzer_t* analyzer);
void semantic_free_function_definition(semantic_analyzer_t* analyzer);

#endif // SEMANTIC_H
