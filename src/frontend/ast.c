#include "ast.h"

#define DA_LIB_IMPLEMENTATION
#include "../thirdparty/da.h"

void free_expression(expression_t* e) 
{
  if (!e)
    return;

  if (e->type == EXPRESSION_STRING_LIT && e->string_lit.value) 
      free(e->string_lit.value);

  if (e->type == EXPRESSION_VAR) {
    if (e->var.name)
      free(e->var.name);
  }

  if (e->type == EXPRESSION_ASSIGN) {
    if (e->assign.lhs)
      free_expression(e->assign.lhs);

    if (e->assign.rhs)
      free_expression(e->assign.rhs);
  }

  free(e);
}

void free_statement(statement_t* s) 
{
  if (!s)
    return;

  if (s->next) 
    free_statement(s->next);

  if (s->type == STATEMENT_RETURN) 
    free_expression(s->ret.value);

  if (s->type == STATEMENT_DECL)
      free_declaration(s->decl_stmt.decl);

  if (s->type == STATEMENT_EXPR)
    free_expression(s->expr_stmt.expr);

  free(s);
}

void free_declaration(declaration_t* d) 
{
  if (!d)
    return;

  if (d->next)
    free_declaration(d->next);

  if (d->type == DECLARATION_FUNC) {
    if (d->func.name)
      free(d->func.name);

    da_foreach(typed_identifier_t, it, &(d->func.params)) {
      if (it->name)
        free(it->name);
      if (it->type.name)
        free(it->type.name);
    }

    da_free(&(d->func.params));

    if (d->func.return_type.name)
      free(d->func.return_type.name);

    if (d->func.body)
      free_statement(d->func.body);
  }

  if (d->type == DECLARATION_VAR) {
    if (d->var_decl.ident.name) 
      free(d->var_decl.ident.name);

    if (d->var_decl.ident.type.name)
      free(d->var_decl.ident.type.name);

    if(d->var_decl.init)
      free_expression(d->var_decl.init);
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

bool check_next(parser_t* p, long kind, int range) 
{
  if ((size_t) (p->pos + range) > p->count)
    return false;

  return p->items[p->pos + range].type == kind;
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
  if (strcmp(peek(p)->string_value, "string") == 0)
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

type_kind get_type_kind_from_string(char* type_string) 
{
  if (strcmp(type_string, "int") == 0) 
    return TYPE_INT;

  if (strcmp(type_string, "string") == 0)
    return TYPE_STRING;


  // Some unexpected fallback
  // TODO: implement this in a better way
  return TYPE_INT;
}

expression_t*  ast_parse_expr_int_lit(parser_t* p) {
  expression_t* e = (expression_t*) malloc(sizeof(expression_t));
  
  if (!e) {
    fprintf(stderr, "ERROR - oom while parse_expr_int_lit\n");
    return NULL;
  }

  memset(e, 0, sizeof(expression_t));
  e->type = EXPRESSION_INT_LIT;
  token_t* t = advance(p);

  e->int_lit.value = t->int_value;
  return e;
}

expression_t* ast_parse_expr_string_lit(parser_t* p) 
{
  expression_t* e = (expression_t*) malloc(sizeof(expression_t));
  if (!e) {
    fprintf(stderr, "ERROR - oom while parse_expr_string_lit\n");
    return NULL;
  }
  memset(e, 0, sizeof(expression_t));

  e->type = EXPRESSION_STRING_LIT;
  token_t* t = advance(p);

  if (!t->string_value) {
    fprintf(stderr, "ERROR - string expression has no value");
    free_expression(e);
    return NULL;
  }

  e->string_lit.value = strdup(t->string_value);

  return e;
}

expression_t* ast_parse_expr_var(parser_t* p) 
{
  expression_t* e = (expression_t*) malloc(sizeof(expression_t));
  if (!e) {
    fprintf(stderr, "ERROR - oom while ast_parse_expr_var\n");
    return NULL;
  }
  memset(e, 0, sizeof(expression_t));

  e->type = EXPRESSION_VAR;
  token_t* var_tok = advance(p);

  if (!var_tok->string_value) {
    fprintf(stderr, "ERROR - ident token has no string value on ast_parse_expr_var\n");
    free_expression(e);
    return NULL;
  }

  e->var.name = strdup(var_tok->string_value);
  
  return e;
}

expression_t* ast_parse_expr_assign(parser_t* p)
{
  expression_t* e = (expression_t*) malloc(sizeof(expression_t));
  if (!e) {
    fprintf(stderr, "ERROR - oom while parse_expr_assign\n");
    return NULL; 
  }
  memset(e, 0, sizeof(expression_t));

  e->type = EXPRESSION_ASSIGN;
  
  // We compute lhs here otherwise we fallback in infinit loop 'id ='
  expression_t* lhs = (expression_t*) malloc(sizeof(expression_t));
  if (!lhs) {
    fprintf(stderr, "ERROR - oom while parse_expr_assign\n");
    free_expression(e);
    return NULL;
  }

  lhs->type = EXPRESSION_VAR;

  token_t* var_tok = advance(p);
  if (!var_tok->string_value) {
    fprintf(stderr, "ERROR - var_tok should have string value at parse_expr_assign\n"); 
    free_expression(e);
    free_expression(lhs);
    return NULL;
  }

  lhs->var.name = strdup(var_tok->string_value);
  if (!lhs->var.name) {
    fprintf(stderr, "ERROR - oom at parse_expr_assign\n"); 
    free_expression(e);
    free_expression(lhs);
    return NULL;
  }

  e->assign.lhs = lhs;

  expect(p, '=', "ERROR - should get = after id in parse assign\n");

  e->assign.rhs = parse_expression(p);

  return e;
}

expression_t* parse_expression(parser_t* p) 
{
  if (check(p, LEXER_token_id) && check_next(p, '=', 1)) {
    return ast_parse_expr_assign(p);
  } else if (check(p, LEXER_token_id)) {
    return ast_parse_expr_var(p);
  } else if (check(p, LEXER_token_dqstring)) {
    return ast_parse_expr_string_lit(p);
  } else {
    return ast_parse_expr_int_lit(p);
  }

  // Unreachable (normally)
  return NULL;
}

declaration_t* ast_parse_function(parser_t* p)
{
  declaration_t* decl = (declaration_t*) malloc(sizeof(declaration_t));
  if (!decl) { fprintf(stderr, "Out of memory\n"); return NULL; }
  memset(decl, 0, sizeof(declaration_t));

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

  while (!check(p, ')')) {
    typed_identifier_t param;

    if (!check(p, LEXER_token_id) || !check_is_type(p)) {
      fprintf(stderr, "ERROR - should get param type\n");
      free_declaration(decl);
      return NULL;
    }

    token_t* type_tok = advance(p);

    if (!type_tok->string_value) {
      fprintf(stderr, "ERROR - param type not found\n");
      free_declaration(decl);
      return NULL;
    }

    param.type.name = strdup(type_tok->string_value);
    if (!param.type.name) {
      fprintf(stderr, "ERROR - oom on function param parsing\n");
      free_declaration(decl);
      return NULL;
    }

    param.type.kind = get_type_kind_from_string(param.type.name);

    if (!check(p, LEXER_token_id)) {
      fprintf(stderr, "ERROR - should get var name after param type\n");
      free_declaration(decl);
      return NULL;
    }
    token_t* name_tok = advance(p);

    if (!name_tok->string_value) {
      fprintf(stderr, "ERROR - id token does not have string value\n");
      free_declaration(decl);
      return NULL;
    }

    param.name = strdup(name_tok->string_value);
    if (!param.name) {
      fprintf(stderr, "ERROR - oom on function param name parsing\n");
      free_declaration(decl);
      return NULL;
    }

    da_append(&(decl->func.params), param);
    if (check(p, ',')) {
      advance(p);
    }
  }

  // consume ')'
  advance(p);

  // we don't use expect here cause function can have no return type
  if (check(p, ':')) {
    advance(p);

    if (!check(p, LEXER_token_id)) {
      fprintf(stderr, "ERROR - specify return type");
      free_declaration(decl);
      return NULL;
    }

    token_t* ret_tok = advance(p); 
    if (!ret_tok->string_value) {
      fprintf(stderr, "ERROR - no return type specified after ':'");
      free_declaration(decl);
      return NULL;
    }

    decl->func.return_type.name = strdup(ret_tok->string_value);
    decl->func.return_type.kind = get_type_kind_from_string(decl->func.return_type.name);

    if (!decl->func.return_type.name) {
      fprintf(stderr, "ERROR - oom at ast_parse_function\n");
      free_declaration(decl);
      return NULL;
    }

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
  memset(d, 0, sizeof(declaration_t));

  d->type = DECLARATION_VAR;
  d->next = NULL;

  // For now, we assume that we can only fall back here if the newt token is a typedef
  // Which mean that next token is an token_id with a string_value
  // This Might cause error later on
  token_t* type_tok = advance(p);
  d->var_decl.ident.type.name = strdup(type_tok->string_value);
  if (!d->var_decl.ident.type.name) {
    fprintf(stderr, "ERROR - can't get type\n");
    free_declaration(d);
    return NULL;
  }

  if (strcmp(d->var_decl.ident.type.name, "int") == 0) {
    d->var_decl.ident.type.kind = TYPE_INT;
  } else if (strcmp(d->var_decl.ident.type.name, "string") == 0) {
    d->var_decl.ident.type.kind = TYPE_STRING;
  } else {
    // For now, let's mark this as an error,
    // In a more advanced compiler this should be marked as TYPE_CUSTOM
    fprintf(stderr, "ERROR - unknown type in parse_var_decl\n");
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

  d->var_decl.ident.name = strdup(name_tok->string_value);
  if (!d->var_decl.ident.name) {
    fprintf(stderr, "ERROR - oom\n");
    free_declaration(d);
    return NULL;
  }

  if (!expect(p, '=', "ERROR - missing '=' token")) {
    free_declaration(d);
    return NULL;
  }

  expression_t* e = parse_expression(p);
  if (!e) {
    fprintf(stderr, "ERROR - d->var.e is NULL\n");
    free_declaration(d);
    return NULL;
  }
  d->var_decl.init = e;

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
  memset(s, 0, sizeof(statement_t));

  s->type = STATEMENT_RETURN;
  s->ret.value = parse_expression(p);

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
  memset(s, 0, sizeof(statement_t));
  
  s->type = STATEMENT_DECL;
  s->next = NULL;
  
  s->decl_stmt.decl = ast_parse_var_decl(p);
  if (!s->decl_stmt.decl) {
    fprintf(stderr, "ERROR - parsing var declaration\n");
    free_statement(s);
    return NULL;
  }

  return s;
}

statement_t* ast_parse_expr_stmt(parser_t* p) 
{
  statement_t* s = (statement_t*) malloc(sizeof(statement_t));
  if (!s) {
    fprintf(stderr, "ERROR - oom while parse_expr_stmt\n"); 
    return NULL;
  }
  memset(s, 0, sizeof(statement_t));

  s->type = STATEMENT_EXPR;
  s->next = NULL;

  s->expr_stmt.expr = parse_expression(p);
  if (!s->expr_stmt.expr) {
    fprintf(stderr, "ERROR - parsing expr\n");
    free_statement(s);
    return NULL;
  }

  if(!expect(p, ';', "ERROR - missing ';' token at end of statement\n")) {
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
  } else if (check(p, LEXER_token_id)) {
    // This is some kind of fallback
    // TODO: work on this to include other use case 
    return ast_parse_expr_stmt(p);
  }

  return NULL;
}
