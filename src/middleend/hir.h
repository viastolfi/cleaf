#ifndef HIR_H
#define HIR_H

#define DA_LIB_IMPLEMENTATION
#include "../thirdparty/da.h"
#include "../thirdparty/error.h"
#include "../frontend/ast_definition.h"

typedef enum {
  HIT_INT_CONST,
  HIT_STRING_CONST,
  HIR_VAR,
  HIR_BINARY_OP,
  HIR_UNARY_OP,
  HIR_CALL
} hir_expr_kind;

typedef enum {
  HIR_PLUS,
  HIR_MINUS,
  HIR_MUL,
  HIR_DIV,
  HIR_GT,
  HIR_GTE,
  HIR_LT,
  HIR_LTE,
  HIR_EQ,
  HIR_NEQ
} hir_binary_op;

typedef enum {
  HIR_NEGATE,
  HIR_NOT,
  HIR_PRE_INC,
  HIR_PRE_DEC,
  HIR_POST_INC,
  HIR_POST_DEC
} hir_unary_op;

typedef enum {
  HIR_STMT_EXPR,
  HIR_STMT_ASSIGN,
  HIR_STMT_RETURN,
  HIR_STMT_IF,
  HIR_STMT_FOR,
  HIR_STMT_WHILE
} hir_stmt_kind;

typedef struct hir_expr_t hir_expr_t;
typedef struct hir_stmt_t hir_stmt_t;

typedef struct 
{
  hir_expr_t** items;
  size_t count;
  size_t capacity;
} hir_block_t;

typedef struct 
{
  char* name;
  type_kind return_type;
  typed_identifier_t* params;
  size_t param_count;

  hir_block_t* body;

  int next_temp_id;
} hir_funcion_t;

struct hir_stmt_t 
{
  hir_stmt_kind kind;

  union {
    hir_expr_t* expr;    
    
    struct {
      char* target;
      hir_expr_t* value; 
    } assign;

    hir_expr_t* ret_value;

    struct {
      hir_expr_t* condition;
      hir_block_t* then_branch; 
      hir_block_t* else_branch;
    } if_stmt;

    struct {
      hir_expr_t* condition; 
      hir_block_t* body;
    } while_stmt;

    struct {
      char* iter_init;
      hir_expr_t* init; 
      hir_expr_t* condition;
      hir_expr_t* increment;
      hir_block_t* body;
    } for_stmt;
  };
};

struct hir_expr_t 
{
  hir_expr_kind kind;
  type_kind type;
  int temp_id;

  union {
    int int_value;
    char* string_value;
    char* var_name; 

    struct {
      hir_expr_t* lhs;
      hir_expr_t* rhs;
      hir_binary_op op;  
    } binary;

    struct {
      hir_expr_t* operand;
      hir_unary_op op;
    } unary;

    struct {
      char* callee; 
      hir_expr_t** args;
      size_t arg_count;
    } call;
  };
};

typedef struct 
{
  hir_funcion_t** items;
  size_t count;
  size_t capacity;
} hir_function_array;

#endif // HIT_H
