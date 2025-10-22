#ifndef AST_DEFINITION_H
#define AST_DEFINITION_H

typedef enum 
{
  DECLARATION_VAR,
  DECLARATION_FUNC
} declaration_kind;

typedef enum
{
  STATEMENT_EXPR,
  STATEMENT_RETURN,
  STATEMENT_DECL
} statement_kind;

typedef enum 
{
  EXPRESSION_LIT,
  EXPRESSION_VAR,
  EXPRESSION_BINARY,
  EXPRESSION_CALL,
  EXPRESSION_ASSIGN
} expression_kind;

typedef struct 
{
  char* name;
  char* type;

  int int_value;

  char* string_value;
  int string_len;
} function_parameter_t;

typedef struct declaration_t declaration_t;
typedef struct statement_t statement_t;
typedef struct expression_t expression_t;

typedef struct 
{
  declaration_t* items;
  size_t count;
  size_t capacity;
} declaration_array;

struct declaration_t
{
  declaration_kind type;
  declaration_t* next;
  union {
    struct { char* name; char* type; expression_t* init; } var;
    struct { char* name; char* return_type; function_parameter_t* params; statement_t* body; } func;
  };
};

struct statement_t 
{
  statement_kind type;
  statement_t* next;

  union {
    struct { char* type; char* id_name; int int_value; char* string_value; int string_len; } ret;
    struct { expression_t* e; } expr;
    struct { declaration_t* var; } decl;
  };
};

struct expression_t 
{
  expression_kind type;

  int int_value;

  char* string_value;
  int string_len;
};

#endif // AST_DEFINITION_H
