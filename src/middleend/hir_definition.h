#ifndef HIR_DEFINITION_H
#define HIR_DEFINITION_H

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

typedef enum 
{
  HIR_BINARY_ADD,
  HIR_BINARY_MINUS,
  HIR_BINARY_MUL
} HIR_binary_kind;

typedef int HIR_temp_id;

typedef struct 
{
  HIR_instruction_kind kind;

  HIR_temp_id dest;
  HIR_temp_id a, b;

  union {
    int int_value;
    char* string_value; 

    char* var;

    HIR_binary_kind op;

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

#endif // HIR_DEFINITION_H
