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

bool check(parser_t* p, long kind)
{
  if(peek(p) && peek(p)->type == kind) return true;
  return false;
}

bool expect(parser_t* p, long kind, char* err) 
{
  if (check(p, kind)) {
    p->pos++;
    return true;
  }

  fprintf(stderr, "ERROR - parsing : %s\n", err);
  return false;
}

declaration_t* ast_parse_function(parser_t* p)
{
  declaration_t* decl = (declaration_t*) malloc(sizeof(declaration_t));
  if (!decl) { fprintf(stderr, "Out of memory\n"); return NULL; }

  decl->type = DECLARATION_FUNC;
  decl->next = NULL;

  if (check(p, LEXER_token_id)) {
    token_t * name_tok = advance(p);
    if (name_tok->string_value) {
      decl->func.name = strdup(name_tok->string_value);
      if (!decl->func.name) { fprintf(stderr,"oom\n"); free(decl); return NULL; }
    }
  } else {
    fprintf(stderr, "ERROR - specify function name\n");
    free(decl);
    return NULL;
  }
   
  if (!expect(p, '(', "missing token : '(' after function name")) {
    free(decl->func.name); free(decl);
    return NULL;
  }

  decl->func.params = NULL;
  while (!check(p, ')')) {
    // TODO: parse function params
    if ((size_t) p->pos >= p->count) {
      fprintf(stderr, "Unexpected EOF in function params parsing\n");
      free(decl->func.name); free(decl);
    }

    advance(p);
  }

  // consume ')'
  advance(p);

  // we don't use expect here cause function can have no return type
  if (check(p, ':')) {
    advance(p);
    if (check(p, LEXER_token_id)) {
      token_t* ret_tok = advance(p); 
      if (ret_tok->string_value) {
        decl->func.return_type = strdup(ret_tok->string_value);
        if (!decl->func.return_type) {
          fprintf(stderr, "ERROR - oom\n");
          free(decl->func.name); free(decl);
          return NULL;
        }
      } else {
        fprintf(stderr, "ERROR - no return type specified after ':'");
        free(decl->func.name); free(decl);
        return NULL;
      }
    } else {
      fprintf(stderr, "ERROR - specify return type");
      free(decl->func.name); free(decl);
    }
  } else {
    decl->func.return_type = NULL;
  }

  if (!expect(p, '{', "ERROR - missing token '{'")) {
        free(decl->func.return_type);
        free(decl->func.name);
        free(decl);
        return NULL;
    }

    decl->func.body = NULL; // TODO: implement parse_block
                            
  return decl;
}

declaration_t* parse_declaration(parser_t* p)
{
  if (check(p, LEXER_token_id) && strcmp(peek(p)->string_value, "fn") == 0) {
    advance(p);
    return ast_parse_function(p);
  }

  // Do we risk to fall here ?
  return NULL;
}

void print_declaration(declaration_t* d) 
{
  if (d->type == (declaration_kind) DECLARATION_FUNC) {
    printf("\033[0;34m`-\033[0m\033[0;1;32mFUNCTION_DECLARATION\033[0m\033[0;33m %p\033[0m %s\033[0;32m %s (%s)\033[0m\n", d, d->func.name, d->func.return_type ? d->func.return_type : "void", d->func.params ? d->func.params->type : "");
  }
}
