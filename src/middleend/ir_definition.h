#ifndef IR_DEFINITION_H
#define IR_DEFINITION_H

#include "../frontend/ast_definition.h"
#include "../thirdparty/rand.h"
#include "../thirdparty/error.h"
#include "../thirdparty/hashmap.h"
#include "../frontend/symbols.h"

typedef void (*chunk_name_gen_t)(void* ctx, char* out);

#define HIR_PARSER_USE_RNG(parser, rng_ptr) \
  do { \
    (parser).gen_chunk = rand_chunk_gen; \
    (parser).chunk_ctx = (void*)(rng_ptr); \
  } while(0)

typedef enum 
{
  IR_NOP,

  IR_MOV,
  IR_MOV_OFFSET,

  IR_CHUNK,

  IR_INT_CONST,

  IR_BINARY,
  IR_DIRECT_MUL,

  IR_INC,
  IR_DEC,

  IR_LOAD_VAR,
  IR_STORE_VAR,

  IR_LOAD_ELEM,
  IR_STORE_ELEM,

  IR_JMP,
  IR_JMP_EQUAL,
  IR_JMP_NOT_EQUAL,
  IR_JMP_GREATER_THAN,
  IR_JMP_GREATER_THAN_EQUAL,
  IR_JMP_LOWER_THAN,
  IR_JMP_LOWER_THAN_EQUAL,

  IR_RETURN,
  IR_EXIT,

  IR_ALLOC,
  IR_DEALLOC,

  IR_CALL,

  IR_ASM
} IR_instruction_kind;

typedef enum 
{
  IR_BINARY_ADD,
  IR_BINARY_SUB,
  IR_BINARY_MUL,
  IR_BINARY_CMP,
} IR_binary_kind;

typedef enum 
{
  IR_PRE_OFFSET,
  IR_POST_OFFSET,
} IR_offset_timing;

typedef struct {
  int id;
  size_t size;
} IR_temp_id;

typedef struct 
{
  IR_instruction_kind kind;

  IR_temp_id dest, src;

  union {
    int int_value;
    size_t alloc_size;

    IR_temp_id index;

    struct {
      char* name;
      int is_init;
    } var;

    struct {
      IR_offset_timing timing;
      size_t size;
    } offset;

    IR_binary_kind binary_op;

    char* chunk_name;
    char* func_name;

    struct {
      char** strings;
      size_t string_count;
      IR_temp_id* args;
      size_t arg_count;
    } asm_data;
  };
} IR_instruction_t;

typedef struct 
{
  IR_instruction_t** items;
  size_t count;
  size_t capacity;
} IR_instruction_block;

typedef struct 
{
  char* name;

  IR_instruction_block* code;

  int next_temp_id;

  size_t stack_reserve_size;
} IR_function_t;

typedef struct 
{
  IR_function_t** items;
  size_t count;
  size_t capacity;
} IR_function_array;

#endif // IR_DEFINITION_H
