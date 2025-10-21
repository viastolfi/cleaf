#ifndef AST_H
#define AST_H

typedef enum 
{
  DECLARATION_VAR,
  DECLARATION_FUNC
} declaration_kind;

typedef enum
{
  STATEMENT_EXPR,
  STATEMENT_RETURN
} statement_kind;

typedef enum 
{
  EXPRESSION_LIT,
  EXPRESSION_VAR,
  EXPRESSION_BINARY,
  EXPRESSION_CALL,
  EXPRESSION_ASSIGN
} expression_kind;

#endif // AST_H
