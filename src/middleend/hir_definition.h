#ifndef HIR_DEFINITION_H
#define HIR_DEFINITION_H

#include "../frontend/ast_definition.h"
#include "../thirdparty/rand.h"

typedef void (*chunk_name_gen_t)(void* ctx, char* out);

#define HIR_PARSER_USE_RNG(parser, rng_ptr) \
  do { \
    (parser).gen_chunk = rand_chunk_gen; \
    (parser).chunk_ctx = (void*)(rng_ptr); \
  } while(0)

typedef enum 
{
  HIR_NOP,

  HIR_MOV,

  HIR_CHUNK,

  HIR_INT_CONST,
  HIR_STRING_CONST,

  HIR_BINARY,

  HIR_INC,
  HIR_DEC,

  HIR_LOAD_VAR,
  HIR_STORE_VAR,

  HIR_JMP,
  HIR_JMP_EQUAL,
  HIR_JMP_NOT_EQUAL,

  HIR_RETURN,
  HIR_EXIT,

  HIR_CALL
} HIR_instruction_kind;

typedef enum 
{
  HIR_BINARY_ADD,
  HIR_BINARY_MINUS,
  HIR_BINARY_MUL,
  HIR_BINARY_CMP,
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

    struct {
      char* name;
      int is_init;
    } var;

    HIR_binary_kind binary_op;

    char* chunk_name;

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

  chunk_name_gen_t gen_chunk;
  void* chunk_ctx;

  HIR_function_array* hir_program;
} HIR_parser_t;

#endif // HIR_DEFINITION_H
