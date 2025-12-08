#ifndef SEMANTIC_H
#define SEMANTIC_H

#define DA_LIB_IMPLEMENTATION
#include "../thirdparty/da.h"
#include "ast_definition.h"
#include "error.h"
#include "../thirdparty/hashmap.h"
#include "scope.h"    

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct
{
  const char* message;
  const char* position;
} diagnostic_t;

typedef struct 
{
  diagnostic_t* items;
  size_t count;
  size_t capacity;
} diagnostics_t;

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
  diagnostics_t semantic_errors;
  int error_count;

  declaration_array* ast;

  hashmap_t* function_symbols;

  const char* current_analyzed_function;
} semantic_analyzer_t;

int string_array_contains(char** source, size_t source_len, const char* name);

int analyze_declaration(semantic_analyzer_t* analyzer,
                        declaration_t* decl,
                        scope_t* scope);
type_kind semantic_check_expression(semantic_analyzer_t* analyzer,
                       expression_t* expr,
                       scope_t* scope);
void semantic_analyze(semantic_analyzer_t* analyzer);
void semantic_check_for_statement(semantic_analyzer_t* analyzer,
                                  statement_t* stmt,
                                  scope_t* scope);
void semantic_check_return_statement(semantic_analyzer_t* analyzer,
                                     statement_t* stmt,
                                     scope_t* scope);
void semantic_check_scope(semantic_analyzer_t* analyzer, 
                          statement_block_t* func, 
                          scope_t* scope);
void semantic_load_function_definition(semantic_analyzer_t* analyzer);
void semantic_free_function_definition(semantic_analyzer_t* analyzer);

void semantic_error_register(semantic_analyzer_t* analyzer,
                             const char* pos, 
                             const char* msg);
void semantic_error_display(semantic_analyzer_t* analyzer);
int semantic_check_name_not_reserved(semantic_analyzer_t* analyzer, 
                                     const char* name);

#endif // SEMANTIC_H
