#include "ast.h"

#define DA_LIB_IMPLEMENTATION
#include "../thirdparty/da.h"

void free_statement(statement_t* s) 
{
  if (s->next) 
    free_statement(s->next);

  if (s->type == STATEMENT_RETURN) {
    if (s->ret.type)
      free(s->ret.type);

    if (s->ret.string_value)
      free(s->ret.string_value);
  }

  if (s->type == STATEMENT_DECL) {
    if (s->decl.var)
      free_declaration(s->decl.var);
  }

  free(s);
}

void free_declaration(declaration_t* d) 
{
  if (d->next)
    free_declaration(d->next);

  if (d->type == DECLARATION_FUNC) {
    if (d->func.name)
      free(d->func.name);

    // TODO: create function that free function params completely
    // if (d->func.params)

    if (d->func.return_type)
      free(d->func.return_type);

    if (d->func.body)
      free_statement(d->func.body);
  }

  if (d->type == DECLARATION_VAR) {
    if (d->var.name)
      free(d->var.name);

    if(d->var.type)
      free(d->var.type);

    // TODO: add a free_expression function
  }
  
  free(d);
}

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

bool check_is_type(parser_t* p) 
{
  if (!peek(p)->string_value) {
    fprintf(stderr, "ERROR - unknown token, looking for type declaration");
    return false;
  }

  // atm, we only handle int type
  // TODO: add other types
  if (strcmp(peek(p)->string_value, "int") == 0)
    return true;

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
      if (!decl->func.name) { 
        fprintf(stderr,"oom\n"); 
        free_declaration(decl); 
        return NULL; 
      }
    }
  } else {
    fprintf(stderr, "ERROR - specify function name\n");
    free_declaration(decl);
    return NULL;
  }

  if (!expect(p, '(', "missing token : '(' after function name")) {
    free_declaration(decl);
    return NULL;
  }

  decl->func.params = NULL;
  while (!check(p, ')')) {
    // TODO: parse function params
    if ((size_t) p->pos >= p->count) {
      fprintf(stderr, "Unexpected EOF in function params parsing\n");
      free_declaration(decl);
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
          free_declaration(decl);
          return NULL;
        }
      } else {
        fprintf(stderr, "ERROR - no return type specified after ':'");
        free_declaration(decl);
        return NULL;
      }
    } else {
      fprintf(stderr, "ERROR - specify return type");
      free_declaration(decl);
    }
  } else {
    decl->func.return_type = NULL;
  }

  if (!expect(p, '{', "ERROR - missing token '{'")) {
    free_declaration(decl);
    return NULL;
  }

  decl->func.body = parse_statement(p); 
  // TODO: add error handling if body = NULL

  while (!check(p, '}')) {
    if ((size_t) p->pos >= p->count) {
      fprintf(stderr, "ERROR - early EOF / missing '}' token\n");
    }

    statement_t* stmt = parse_statement(p);
    if (!stmt) {
      fprintf(stderr, "ERROR - parse statement didn't work");
      free_declaration(decl);
      return NULL;
    }

    statement_t** p = &decl->func.body;
    if (!*p) {
      *p = stmt;
    } else {
      while ((*p)->next) {
        p = &(*p)->next;
      }
      (*p)->next = stmt;
    }
  }

  return decl;
}

declaration_t* ast_parse_var_decl(parser_t* p)
{
  declaration_t* d = (declaration_t*) malloc(sizeof(declaration_t));
  if (!d) {
    fprintf(stderr, "ERROR - oom while var decl\n");
    return NULL;
  }

  d->type = DECLARATION_VAR;
  d->next = NULL;

  // For now, we assume that we can only fall back here if the newt token is a typedef
  // Which mean that next token is an token_id with a string_value
  // This Might cause error later on
  token_t* type_tok = advance(p);
  d->var.type = strdup(type_tok->string_value);
  if (!d->var.type) {
    fprintf(stderr, "ERROR - can't get type\n");
    free_declaration(d);
    return NULL;
  }

  if (!check(p, LEXER_token_id)) {
    fprintf(stderr, "ERROR - please provide var name\n");
    free_declaration(d);
    return NULL;
  }

  token_t* name_tok = advance(p);
  if (!name_tok->string_value) {
    fprintf(stderr, "ERROR - var name not found\n");
    free_declaration(d);
    return NULL;
  }

  d->var.name = strdup(name_tok->string_value);
  if (!d->var.name) {
    fprintf(stderr, "ERROR - oom\n");
    free_declaration(d);
    return NULL;
  }

  if (!expect(p, '=', "ERROR - missing '=' token")) {
    free_declaration(d);
    return NULL;
  }

  // TODO: Implement that
  // expression_t* e = ast_parse_value(p);
  advance(p);

  if (!expect(p, ';', "ERROR - missing ';' token")) {
    free_declaration(d);
    return NULL;
  }

  return d;
}

declaration_t* parse_declaration(parser_t* p)
{
  if (check(p, LEXER_token_id) && strcmp(peek(p)->string_value, "fn") == 0) {
    advance(p);
    return ast_parse_function(p);
  }

  if (check(p, LEXER_token_id) && check_is_type(p)) {
    return ast_parse_var_decl(p);
  }
  // Do we risk to fall here ?
  return NULL;
}

statement_t* ast_parse_return_stmt(parser_t* p) 
{
  statement_t* s = (statement_t*) malloc(sizeof(statement_t));
  if (s == NULL) {
    fprintf(stderr, "ERROR - oom\n");
    return NULL;
  }

  s->type = STATEMENT_RETURN;
  if (check(p, LEXER_token_id)) {
    token_t* name_tok = advance(p);
    if (!name_tok->string_value) {
      fprintf(stderr, "ERROR - id has no name\n");
      free_statement(s);
      return NULL;
    }
    s->ret.id_name = strdup(name_tok->string_value);
    if (!s->ret.id_name) {
      free_statement(s);
      fprintf(stderr, "oom\n");
      return NULL;
    }
  } else if (check(p, '"')) {
    // TODO: handle lit string return 
  } else {
    // return lit int
    token_t* val_token = advance(p);
    s->ret.type = malloc(strlen("int"));
    s->ret.type = strdup("int");
    s->ret.int_value = val_token->int_value;
  }

  if (!expect(p, ';', "ERROR - missing token ';' at end of line")) {
    free_statement(s);
    return NULL;
  }

  return s;
}

statement_t* ast_parse_decl_stmt(parser_t* p) 
{
  statement_t* s = (statement_t*) malloc(sizeof(statement_t));
  if (s == NULL) {
    fprintf(stderr, "ERROR - oom\n");
    return NULL;
  }
  
  s->type = STATEMENT_DECL;
  s->next = NULL;
  
  s->decl.var = ast_parse_var_decl(p);
  if (!s->decl.var) {
    fprintf(stderr, "ERROR - parsing var declaration\n");
    free_statement(s);
    return NULL;
  }

  return s;
}

statement_t* parse_statement(parser_t* p) 
{
  if (check(p, LEXER_token_id) && strcmp(peek(p)->string_value, "return") == 0) {
    advance(p);
    return ast_parse_return_stmt(p);
  } else if (check(p, LEXER_token_id) && check_is_type(p)) {
    return ast_parse_decl_stmt(p);
  }

  return NULL;
}
