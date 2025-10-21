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

token_t* peek(parser_t* p);
token_t* advance(parser_t* p);
bool match(parser_t* p, long kind);
declaration_t* ast_parse_function(parser_t* parser);
declaration_t* parse_declaration(parser_t* parser);

void print_declaration(declaration_t* d);

#endif // AST_H
