#ifndef AST_H
#define AST_H

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "lexer.h"
#include "ast_definition.h"

typedef struct 
{
  token_t* items;
  size_t count;
  size_t capacity;

  int pos;
} parser_t;

// Get the actual token at pos
token_t* peek(parser_t* p);

// Get the actual token at pos and advance pos
token_t* advance(parser_t* p);

// Check if the actual token is of kind 
bool check(parser_t* p, long kind);

// Check if the actual token is of kind
// If yes, advance pos
bool expect(parser_t* p, long kind, char* err);

declaration_t* ast_parse_function(parser_t* p);
declaration_t* parse_declaration(parser_t* p);
statement_t*   ast_parse_return_stmt(parser_t* p);
statement_t*   parse_statement(parser_t* p);

void free_declaration(declaration_t* d);
void free_statement(statement_t* s);
void print_declaration(declaration_t* d);

#endif // AST_H
