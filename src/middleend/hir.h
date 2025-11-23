#ifndef HIR_H
#define HIR_H

#include <string.h>

#define DA_LIB_IMPLEMENTATION
#include "../thirdparty/da.h"
#include "../thirdparty/error.h"
#include "../frontend/ast_definition.h"

typedef enum 
{
  HIR_NOP,

  HIR_INT_CONST,
  HIR_STRING_CONST,

  HIR_BINARY,
  HIR_UNARY,

  HIR_LOAD_VAR,
  HIR_STORE_VAR,

  HIR_JUMP,
  HIR_JUMP_IF_FALSE,
  HIR_RETURN,

  HIR_CALL
} HIR_instruction_kind;

typedef int HIR_temp_id;

typedef struct 
{
  HIR_instruction_kind kind;

  HIR_temp_id dest;
  HIR_temp_id a, b;

  union {
    int int_value;
    char* string_value; 

    HIR_temp_id var;

    struct {
      char* callee;
      HIR_temp_id* args;
      size_t arg_count; 
    } call;
  };
} HIR_instruction_t;

typedef struct 
{
  HIR_instruction_t** items;
  size_t count;
  size_t capacity;
} HIR_instruction_block;

typedef struct 
{
  char* name;
  type_kind return_type;

  typed_identifier_t* params;
  size_t param_count;

  HIR_instruction_block* code;

  HIR_temp_id next_temp_id;
} HIR_function_t;

typedef struct 
{
  HIR_function_t** items;
  size_t count;
  size_t capacity;
} HIR_function_array;

typedef struct 
{
  error_context_t* error_ctx;
  int error_count;

  HIR_function_array* hir_program;
} HIR_parser_t;

int HIR_lower_function(HIR_parser_t* hir, 
    declaration_t* function);
int HIR_lower_statement(HIR_parser_t* hir, 
    statement_t* stmt,
    HIR_function_t* func);
int HIR_lower_expression(HIR_parser_t* hir,
    expression_t* expr,
    HIR_function_t* func);
void HIR_display_function(HIR_function_t* function);
void HIR_free_function(HIR_function_t* func);
void HIR_free_instruction(HIR_instruction_t* instr);
#endif // HIR_H
