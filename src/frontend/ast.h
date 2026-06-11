#ifndef AST_H
#define AST_H

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "lexer.h"
#include "ast_definition.h"
#include "../thirdparty/error.h"
#include "types.h"

typedef struct 
{
  token_t* items;
  size_t count;
  size_t capacity;

  int pos;

  known_type_array* types;
  
  error_context_t* error_ctx;
} parser_t;

// Get the actual token at pos
token_t* peek(parser_t* p);
token_t* peek_next(parser_t* p, int range);

// Get the actual token at pos and advance pos
token_t* advance(parser_t* p);

// Check if the actual token is of kind 
bool check(parser_t* p, long kind);
bool check_next(parser_t* p, long kind, int range);
bool check_is_type(parser_t* p);

// Check if the actual token is of kind
// If yes, advance pos
bool expect(parser_t* p, long kind, char* err);

known_type_t* get_type_info_from_string(
    parser_t* p,
    const char* type_string);

declaration_t* ast_parse_function(parser_t* p);
declaration_t* ast_parse_var_decl(parser_t* p);
declaration_t* ast_parse_untype_var_decl(parser_t* p);
declaration_t* ast_parse_struct_decl(parser_t* p);
declaration_t* parse_declaration(parser_t* p);

statement_t*   ast_parse_return_stmt(parser_t* p);
statement_t*   ast_parse_decl_stmt(parser_t* p);
statement_t*   ast_parse_expr_stmt(parser_t* p);
statement_t*   ast_parse_if_stmt(parser_t* p);
statement_t*   ast_parse_while_stmt(parser_t* p);
statement_t*   ast_parse_for_stmt(parser_t* p);
statement_t*   ast_parse_asm_stmt(parser_t* p);
statement_t*   ast_parse_free_stmt(parser_t*p);
statement_t*   parse_statement(parser_t* p);

expression_t*  parse_expression(parser_t* p);
expression_t*  parse_primary(parser_t* p);
expression_t*  ast_parse_expr_int_lit(parser_t* p);
expression_t*  ast_parse_expr_string_lit(parser_t* p);
expression_t*  ast_parse_expr_var(parser_t* p);
expression_t*  ast_parse_expr_assign(parser_t* p);
expression_t*  ast_parse_expr_binary(parser_t* p, 
                                     int bp);
expression_t*  ast_parse_expr_comparison_binary(parser_t* p);
expression_t*  ast_parse_expr_call(parser_t* p);
expression_t*  ast_parse_expr_unary(parser_t* p);
expression_t*  ast_parse_expr_composite_literal(parser_t* p);
expression_t*  ast_parse_expr_char_lit(parser_t* p);

void populate_parser_known_type(known_type_array* types);

void free_declaration(declaration_t* d);
void free_statement(statement_t* s);
void free_expression(expression_t* e);

#endif // AST_H
