#include "ast.h"

#define DA_LIB_IMPLEMENTATION
#include "../thirdparty/da.h"

token_t* peek(parser_t* p)
{
  return (size_t) p->pos < p->count ? &p->items[p->pos] : NULL;
}

token_t* advance(parser_t* p)
{
  return (size_t) p->pos < p->count ? &p->items[p->pos++] : NULL;
}

bool match(parser_t* p, long kind)
{
  if(peek(p) && peek(p)->type == kind) return true;
  return false;
}

declaration_t* ast_parse_function(parser_t* p)
{
  declaration_t* decl = (declaration_t*) malloc(sizeof(declaration_t));
  decl->type = DECLARATION_FUNC;

  if (match(p, LEXER_token_id)) {
    decl->func.name = advance(p)->string_value;
  } else {
    fprintf(stderr, "ERROR - specify function name\n");
  }
   
  return decl;
}

declaration_t* parse_declaration(parser_t* p)
{
  if (match(p, LEXER_token_id) && strcmp(peek(p)->string_value, "fn") == 0) {
    advance(p);
    return ast_parse_function(p);
  }

  // Do we risk to fall here ?
  return NULL;
}

void print_declaration(declaration_t* d) 
{
  if (d->type == (declaration_kind) DECLARATION_FUNC) {
    printf("\033[0;34m`-\033[0m\033[0;1;32mFUNCTION_DECLARATION\033[0m\033[0;33m %p\033[0m %s\n", d, d->func.name);
  }
}
