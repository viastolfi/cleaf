#ifndef AST_DEFINITION_H
#define AST_DEFINITION_H

// ----------------- Enums ------------------

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
  EXPRESSION_INT_LIT,
  EXPRESSION_STRING_LIT,
  EXPRESSION_VAR,
  EXPRESSION_BINARY,
  EXPRESSION_CALL,
  EXPRESSION_ASSIGN
} expression_kind;

typedef enum
{
  TYPE_INT,
  TYPE_STRING
} type_kind;

// ----------------- Forward Declarations ------------------

typedef struct declaration_t declaration_t;
typedef struct statement_t statement_t;
typedef struct expression_t expression_t;

// ----------------- Types and Identifiers ------------------

typedef struct 
{
  type_kind kind;
  char* name;
} type_t;

typedef struct 
{
  char* name;
  type_t type;
} typed_identifier_t;

// ----------------- Dynamic arrays ------------------

typedef struct 
{
  typed_identifier_t* items;
  size_t count;
  size_t capacity;
} function_param_array;


typedef struct 
{
  declaration_t* items;
  size_t count;
  size_t capacity;
} declaration_array;

// ----------------- Declarations ------------------

struct declaration_t
{
  declaration_kind type;
  declaration_t* next;
  union {
    struct { 
      typed_identifier_t ident; 
      expression_t* init; 
    } var_decl;

    struct { 
      char* name;  
      type_t return_type; 
      function_param_array params; 
      statement_t* body; 
    } func;
  };
};

// ----------------- Statements ------------------

struct statement_t 
{
  statement_kind type;
  statement_t* next;

  union {
    struct { expression_t* value; } ret;
    struct { expression_t* expr; } expr_stmt;
    struct { declaration_t* decl; } decl_stmt;
  };
};

// ----------------- Expressions ------------------

struct expression_t 
{
  expression_kind type;

  union {
    struct { int value; } int_lit;
    struct { char* value; } string_lit;
    struct { char* name; } var;
    struct { expression_t* lhs; expression_t* rhs; } assign;
    struct { 
      expression_t* left; 
      expression_t* right; 
      char op; 
    } binary;
  };
};

#endif // AST_DEFINITION_H
