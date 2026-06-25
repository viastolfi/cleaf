#ifndef AST_DEFINITION_H
#define AST_DEFINITION_H

#include <stddef.h>
#include <stdbool.h>

#include "types.h"

// ----------------- Enums ------------------

typedef enum 
{
  DECLARATION_VAR,
  DECLARATION_FUNC,
  DECLARATION_STRUCT,
  DECLARATION_MODULE,
  DECLARATION_IMPORT,
} declaration_kind;

typedef enum
{
  STATEMENT_EXPR,
  STATEMENT_RETURN,
  STATEMENT_DECL,
  STATEMENT_IF,
  STATEMENT_WHILE,
  STATEMENT_FOR,
  STATEMENT_ASM,
  STATEMENT_FREE,
} statement_kind;

typedef enum 
{
  EXPRESSION_INT_LIT,
  EXPRESSION_CHAR_LIT,
  EXPRESSION_VAR,
  EXPRESSION_BINARY,
  EXPRESSION_CALL,
  EXPRESSION_ASSIGN,
  EXPRESSION_UNARY,
  EXPRESSION_COMPOSITE_LITERAL,
  EXPRESSION_INDEX,
} expression_kind;

typedef enum
{
  FOR_INIT_DECL,
  FOR_INIT_EXPR
} for_init_kind;

typedef enum 
{
  UNARY_NEGATE,
  UNARY_NOT,
  UNARY_PRE_INC,
  UNARY_PRE_DEC,
  UNARY_POST_INC,
  UNARY_POST_DEC
} unary_op_kind;

typedef enum 
{
  BINARY_ADD,
  BINARY_SUB,
  BINARY_MUL,
  BINARY_DIV,
  BINARY_GT,
  BINARY_GTE,
  BINARY_LT,
  BINARY_LTE,
  BINARY_EQ,
  BINARY_NEQ
} binary_op_kind;

// ----------------- Forward Declarations ------------------

typedef struct declaration_t declaration_t;
typedef struct statement_t statement_t;
typedef struct expression_t expression_t;

// ----------------- Types and Identifiers ------------------

typedef struct {
  char* name;
  size_t size; // in bytes
  size_t element_size;
  size_t array_len;
  types_t kind;
} known_type_t;

typedef struct 
{
  known_type_t type; 
  char* ident_name;
  const char* source_pos;
  bool is_constant;
} typed_identifier_t;

// ----------------- Dynamic arrays ------------------

typedef struct {
  known_type_t* items;
  size_t count;
  size_t capacity; 
} known_type_array;

typedef struct 
{
  typed_identifier_t* items;
  size_t count;
  size_t capacity;
} typed_identifier_array;

typedef struct 
{
  declaration_t** items;
  size_t count;
  size_t capacity;
} declaration_array;

typedef struct
{
  statement_t** items;
  size_t count;
  size_t capacity;
} statement_block_t;

typedef struct 
{
  char** items;
  size_t count;
  size_t capacity;
} import_path_t;

// ----------------- Declarations ------------------

struct declaration_t
{
  declaration_kind type;
  const char* source_pos;

  union {
    struct { 
      typed_identifier_t ident; 
      expression_t* init; 
    } var_decl;

    struct { 
      char* name;  
      known_type_t return_type; 
      typed_identifier_array params; 
      statement_block_t* body;
      bool is_internal;
    } func;

    struct {
      char* name;
      typed_identifier_array members; 
    } struc;

    struct {
      char* name; 
    } module;

    struct {
      import_path_t path;
      char* alias;
    } import;
  };
};

// ----------------- Statements ------------------

struct statement_t 
{
  statement_kind type;
  const char* source_pos;

  union {
    struct { expression_t* value; } ret;
    struct { expression_t* expr; } expr_stmt;
    struct { declaration_t* decl; } decl_stmt;
    struct { expression_t* expr; } free_stmt;
    struct {
      char** instr; 
      expression_t** args;
      size_t instr_count;
      size_t arg_count;
    } asm_stmt;
    struct {
      expression_t* condition;  
      statement_block_t* then_branch;
      statement_block_t* else_branch;
    } if_stmt;
    struct {
      expression_t* condition;
      statement_block_t* body;
    } while_stmt;
    struct {
      for_init_kind init_kind;
      union {
        declaration_t* decl_init; 
        expression_t* expr_init;
      };
      expression_t* condition;
      expression_t* loop; 
      statement_block_t* body;
    } for_stmt;
  };
};

// ----------------- Expressions ------------------

struct expression_t 
{
  expression_kind type;
  const char* source_pos;

  union {
    struct { int value; } int_lit;
    struct { char value; } char_lit;
    struct { 
      typed_identifier_t ident; 
      expression_t* member; 
    } var;
    struct { expression_t* lhs; expression_t* rhs; } assign;
    struct { 
      expression_t* left; 
      expression_t* right; 
      binary_op_kind op;
    } binary;
    struct { 
      char* callee; 
      expression_t** args; 
      size_t arg_count; 
    } call;
    struct {
      unary_op_kind op;
      expression_t* operand;
    } unary;
    struct {
      bool is_initializer; 
      expression_t** values;  
      size_t count;
    } composite_literal;
    struct {
      expression_t* base;
      expression_t* index; 
    } index;
  };
};

#endif // AST_DEFINITION_H
