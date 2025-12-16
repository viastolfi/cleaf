#include "ast.h"

#define DA_LIB_IMPLEMENTATION
#include "../thirdparty/da.h"
#include "error.h"

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

  if (e->type == EXPRESSION_BINARY) {
    if (e->binary.left)
      free_expression(e->binary.left); 

    if (e->binary.right)
      free_expression(e->binary.right);
  }

  if (e->type == EXPRESSION_CALL) {
    if (e->call.callee)
      free(e->call.callee);
  
    if (e->call.args) {
      for (size_t i = 0; i < e->call.arg_count; ++i) 
        free_expression(e->call.args[i]);

      free(e->call.args);
    }
  }

  if (e->type == EXPRESSION_UNARY) 
    if (e->unary.operand)
      free_expression(e->unary.operand);  

  free(e);
}

void free_statement(statement_t* s) 
{
  if (!s)
    return;

  if (s->type == STATEMENT_RETURN) 
    free_expression(s->ret.value);

  if (s->type == STATEMENT_DECL)
      free_declaration(s->decl_stmt.decl);

  if (s->type == STATEMENT_EXPR)
    free_expression(s->expr_stmt.expr);

  if (s->type == STATEMENT_IF) {
    if (s->if_stmt.condition) 
      free_expression(s->if_stmt.condition); 
    if (s->if_stmt.then_branch) {
      da_foreach(statement_t*, it, s->if_stmt.then_branch)
        free_statement(*(it)); 
      da_free(s->if_stmt.then_branch);
      free(s->if_stmt.then_branch);
    }
    if (s->if_stmt.else_branch) {
      da_foreach(statement_t*, it, s->if_stmt.else_branch)
        free_statement(*(it));
      da_free(s->if_stmt.else_branch);
      free(s->if_stmt.else_branch);
    }
  }

  if (s->type == STATEMENT_WHILE) {
    if (s->while_stmt.condition)
     free_expression(s->while_stmt.condition); 
    if (s->while_stmt.body) {
      da_foreach(statement_t*, it, s->while_stmt.body)
        free_statement(*(it));
      da_free(s->while_stmt.body); 
      free(s->while_stmt.body);
    }
  }

  if (s->type == STATEMENT_FOR) {
    if (s->for_stmt.decl_init) {
      declaration_t* d = s->for_stmt.decl_init;
      if (d->type == DECLARATION_VAR || d->type == DECLARATION_FUNC) {
        free_declaration(s->for_stmt.decl_init);
      } else {
        free_expression(s->for_stmt.expr_init);
      }
    }
    if (s->for_stmt.condition)
      free_expression(s->for_stmt.condition);
    if (s->for_stmt.loop)
      free_expression(s->for_stmt.loop);
    if (s->for_stmt.body) {
      da_foreach(statement_t*, it, s->for_stmt.body)
       free_statement(*(it)); 
      da_free(s->for_stmt.body);
      free(s->for_stmt.body);
    }
  }

  free(s);
}

void free_declaration(declaration_t* d) 
{
  if (!d)
    return;

  if (d->type == DECLARATION_FUNC) {
    if (d->func.name)
      free(d->func.name);

    da_foreach(typed_identifier_t, it, &(d->func.params)) {
      if (it->name)
        free(it->name);
    }

    da_free(&(d->func.params));

    if (d->func.body) {
      da_foreach(statement_t*, it, d->func.body) {
        free_statement(*it);
      }
      da_free(d->func.body);
      free(d->func.body);
    }
  }

  if (d->type == DECLARATION_VAR) {
    if (d->var_decl.ident.name) 
      free(d->var_decl.ident.name);

    if(d->var_decl.init)
      free_expression(d->var_decl.init);
  }
  
  free(d);
}

token_t* peek(parser_t* p)
{
  return (size_t) p->pos < p->count ? &p->items[p->pos] : NULL;
}

token_t* peek_next(parser_t* p, int range) 
{
  return (size_t) p->pos + range < p->count ? &p->items[p->pos + range] : NULL; 
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
  if ((size_t) (p->pos + range) >= p->count)
    return false;

  return p->items[p->pos + range].type == kind;
}

bool check_is_type(parser_t* p) 
{
  token_t* tok = peek(p);
  if (!tok)
    return false;

  if (!tok->string_value) {
    if (p->error_ctx) {
      error_report_at_token(p->error_ctx, tok, ERROR_SEVERITY_ERROR, 
                           "expected type name");
    }
    return false;
  }

  // atm, we only handle int type
  // TODO: add other types
  if (strcmp(peek(p)->string_value, "int") == 0)
    return true;
  if (strcmp(peek(p)->string_value, "string") == 0)
    return true;
  if (strcmp(peek(p)->string_value, "var") == 0)
    return true;

  return false;
}

bool expect(parser_t* p, long kind, char* err) 
{
  if (check(p, kind)) {
    p->pos++;
    return true;
  }

  if (p->error_ctx) {
    token_t* tok = peek(p);
    if (tok) {
      error_report_at_token(p->error_ctx, tok, ERROR_SEVERITY_ERROR, "%s", err);
    } else {
      error_report_general(ERROR_SEVERITY_ERROR, "%s", err);
    }
  }
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
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return NULL;
  }

  memset(e, 0, sizeof(expression_t));
  e->type = EXPRESSION_INT_LIT;
  e->source_pos = peek(p)->source_pos;

  token_t* t = advance(p);
  e->int_lit.value = t->int_value;

  return e;
}

expression_t* ast_parse_expr_string_lit(parser_t* p) 
{
  expression_t* e = (expression_t*) malloc(sizeof(expression_t));
  if (!e) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return NULL;
  }
  memset(e, 0, sizeof(expression_t));

  e->type = EXPRESSION_STRING_LIT;
  e->source_pos = peek(p)->source_pos;

  token_t* t = advance(p);
  if (!t->string_value) {
    if (p->error_ctx) {
      error_report_at_token(p->error_ctx, t, ERROR_SEVERITY_ERROR,
                           "string literal has no value");
    }
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
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return NULL;
  }
  memset(e, 0, sizeof(expression_t));

  e->type = EXPRESSION_VAR;
  e->source_pos = peek(p)->source_pos;

  token_t* var_tok = advance(p);
  if (!var_tok->string_value) {
    if (p->error_ctx) {
      error_report_at_token(p->error_ctx, var_tok, ERROR_SEVERITY_ERROR,
                           "identifier has no value");
    }
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
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return NULL; 
  }
  memset(e, 0, sizeof(expression_t));

  e->type = EXPRESSION_ASSIGN;
  e->source_pos = peek(p)->source_pos;
  
  // We compute lhs here otherwise we fallback in infinit loop 'id ='
  expression_t* lhs = (expression_t*) malloc(sizeof(expression_t));
  if (!lhs) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    free_expression(e);
    return NULL;
  }

  lhs->type = EXPRESSION_VAR;

  token_t* var_tok = advance(p);
  if (!var_tok->string_value) {
    if (p->error_ctx) {
      error_report_at_token(p->error_ctx, var_tok, ERROR_SEVERITY_ERROR,
                           "expected identifier in assignment");
    }
    free_expression(e);
    free_expression(lhs);
    return NULL;
  }

  lhs->var.name = strdup(var_tok->string_value);
  if (!lhs->var.name) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    free_expression(e);
    free_expression(lhs);
    return NULL;
  }

  e->assign.lhs = lhs;

  expect(p, '=', "expected '=' in assignment");

  e->assign.rhs = parse_expression(p);

  if (!e->assign.rhs) {
    error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR,
                          "expected expression on assignment");
  }

  return e;
}

expression_t* ast_parse_expr_binary(parser_t* p) 
{
  expression_t* e = (expression_t*) malloc(sizeof(expression_t)); 
  if (!e) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return NULL;
  }
  memset(e, 0, sizeof(expression_t));

  e->type = EXPRESSION_BINARY;
  e->source_pos = peek(p)->source_pos;

  expression_t* left = (expression_t*) malloc(sizeof(expression_t));
  if (!left) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    free_expression(e);
    return NULL;
  }
  memset(left, 0, sizeof(expression_t));

  token_t* left_tok = advance(p);
  left->source_pos = left_tok->source_pos;
  switch (left_tok->type) {
    case LEXER_token_intlit:
      left->type = EXPRESSION_INT_LIT;
      left->int_lit.value = left_tok->int_value;
      break;
    case LEXER_token_dqstring:
      left->type = EXPRESSION_STRING_LIT;
      if (!left_tok->string_value) {
        if (p->error_ctx) {
          error_report_at_token(p->error_ctx, left_tok, ERROR_SEVERITY_ERROR,
                               "string literal has no value");
        }
        free_expression(e);
        free_expression(left);
        return NULL;
      }
      left->string_lit.value = strdup(left_tok->string_value);
      if (!left->string_lit.value) {
        error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
        free_expression(e);
        free_expression(left);
        return NULL;
      }
      break;
    case LEXER_token_id:
      left->type = EXPRESSION_VAR;
      if (!left_tok->string_value) {
        if (p->error_ctx) {
          error_report_at_token(p->error_ctx, left_tok, ERROR_SEVERITY_ERROR,
                               "identifier has no value");
        }
        free_expression(e);
        free_expression(left);
        return NULL;
      }
      left->var.name = strdup(left_tok->string_value);
      if (!left->var.name) {
        error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
        free_expression(e);
        free_expression(left);
        return NULL;
      }  
      break;
    default:
      // unexpected 
      if (p->error_ctx) {
        error_report_at_token(p->error_ctx, left_tok, ERROR_SEVERITY_ERROR,
                             "unexpected token in binary expression");
      }
      free_expression(e);
      free_expression(left);
      return NULL;
  }

  e->binary.left = left;

  token_t* op_tok = advance(p);
  switch (op_tok->type) {
    case '+':
     e->binary.op = BINARY_PLUS;
     break; 
    case '-':
     e->binary.op = BINARY_MINUS;
     break;
    case '*':
     e->binary.op = BINARY_MUL;
     break;
    case '/':
     e->binary.op = BINARY_DIV;
     break;
    case '>':
     e->binary.op = BINARY_GT;
     break;
    case LEXER_token_gteq:
     e->binary.op = BINARY_GTE;
     break;
    case '<':
     e->binary.op = BINARY_LT;
     break;
    case LEXER_token_lseq:
     e->binary.op = BINARY_LTE;
     break;
    case LEXER_token_eq:
     e->binary.op = BINARY_EQ;
     break;
    case LEXER_token_neq:
     e->binary.op = BINARY_NEQ;
     break;
    default:
     error_report_at_token(p->error_ctx, op_tok, ERROR_SEVERITY_ERROR,
         "unexpected binary operation token");
  }

  e->binary.right = parse_expression(p);
  if (!e->binary.right) {
    if (p->error_ctx) {
      error_report_at_token(p->error_ctx, op_tok, ERROR_SEVERITY_ERROR,
                           "expected expression after binary operator");
    }
    free_expression(e);
    return NULL;
  }

  return e;
}

expression_t*  ast_parse_expr_call(parser_t* p) 
{
  expression_t* e = (expression_t*) malloc(sizeof(expression_t));
  if (!e) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return NULL;
  }
  e->type = EXPRESSION_CALL;
  e->source_pos = peek(p)->source_pos;

  token_t* name_tok = advance(p);
  if (!name_tok->string_value) {
    error_report_at_token(p->error_ctx, name_tok, ERROR_SEVERITY_ERROR,
                          "identifier has no value");
    free_expression(e);
    return NULL;
  }
  e->call.callee = strdup(name_tok->string_value);
  if (!e->call.callee) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    free_expression(e);
    return NULL;
  }

  // consume '('
  advance(p);

  int l = 0;
  while (!check_next(p, ')', l)) {
    if ((size_t) p->pos + l > p->count) {
      error_report_at_token(p->error_ctx, peek_next(p, l - 1), ERROR_SEVERITY_ERROR, 
                            "unexpected EOF, looking for ')' token");
    }
    ++l;
  }

  e->call.arg_count = (size_t) ceil((double) l / 2);
  if (e->call.arg_count > 0) {
    e->call.args = (expression_t**) calloc(e->call.arg_count, sizeof(expression_t));

    for (size_t i = 0; i < e->call.arg_count; ++i) {
      expression_t* arg = parse_expression(p);  
      if (!arg) {
        error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR,
            "unexpected NULL argument");
        free_expression(e);
        return NULL;
      }
      e->call.args[i] = arg;

      if (!check(p, ')') && i == e->call.arg_count - 1) {
        error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR,
            "expected ')' after argument declaration");
        free_expression(e);
        return NULL;
      } 

      // if it's not the last, then it should be ','
      if (!check(p, ',') && i != e->call.arg_count - 1) {
        error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR, 
            "expected ',' after argument declaration");
        free_expression(e);
        return NULL;
      }

      advance(p);
    } 
  } else {
    e->call.args = NULL;
  
    // consume ')'
    advance(p); 
  }
  

  return e;
}

expression_t* ast_parse_expr_unary(parser_t* p) 
{
  expression_t* e = (expression_t*) malloc(sizeof(expression_t));
  if (!e) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return NULL;
  }
  memset(e, 0, sizeof(expression_t));
  
  e->type = EXPRESSION_UNARY;
  e->source_pos = peek(p)->source_pos;

  if (check(p, LEXER_token_plusplus) ||
      check(p, LEXER_token_minusminus) ||
      check(p, '-') ||
      check(p, '!')) {
    token_t* op_tok = advance(p);
    switch (op_tok->type) {
      case LEXER_token_plusplus:
       e->unary.op = UNARY_PRE_INC; 
       break;
      case LEXER_token_minusminus:
       e->unary.op = UNARY_PRE_DEC;
       break;
      case '!':
       e->unary.op = UNARY_NOT;
       break;
      case '-':
       e->unary.op = UNARY_NEGATE;
       break;
      default:
       puts("UNREACHABLE");
    }

    expression_t* operand = parse_expression(p);
    if (!operand) {
      error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR,
          "expected expression after unary operation");
      free_expression(e);
      return NULL;
    }
    e->unary.operand = operand;
  } else {
    expression_t* operand = NULL;
    if (check(p, LEXER_token_id)) {
      operand = ast_parse_expr_var(p);
    } else if (check(p, LEXER_token_intlit)) {
      operand = ast_parse_expr_int_lit(p);
    } else if (check(p, LEXER_token_dqstring)) {
      operand = ast_parse_expr_string_lit(p); 
    }
    
    if (!operand) {
      error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR,
                           "expected identifier or literal before postfix operator");
      free_expression(e);
      return NULL;
    }
   
    e->unary.operand = operand;

    token_t* op_tok = advance(p);
    switch (op_tok->type) {
      case LEXER_token_plusplus:
       e->unary.op = UNARY_POST_INC;
       break;
      case LEXER_token_minusminus:
       e->unary.op = UNARY_POST_DEC; 
       break;
      default:
       puts("UNREACHABLE");
    }
  }

  return e;
}

expression_t* parse_expression(parser_t* p) 
{
  if (check_next(p, '=', 1)) 
    return ast_parse_expr_assign(p);

  if (check(p, LEXER_token_plusplus) ||
      check(p, LEXER_token_minusminus) ||
      check_next(p, LEXER_token_plusplus, 1) ||
      check_next(p, LEXER_token_minusminus, 1) ||
      check(p, '-') ||
      check(p, '!'))
    return ast_parse_expr_unary(p);

  if (check_next(p, '+', 1) ||
      check_next(p, '-', 1) ||
      check_next(p, '*', 1) ||
      check_next(p, '/', 1) ||
      check_next(p, '<', 1) ||
      check_next(p, '>', 1) ||
      check_next(p, LEXER_token_eq, 1) ||
      check_next(p, LEXER_token_neq, 1) ||
      check_next(p, LEXER_token_gteq, 1) ||
      check_next(p, LEXER_token_lseq, 1))
    return ast_parse_expr_binary(p);

  if (check(p, LEXER_token_id) && check_next(p, '(', 1)) {
    return ast_parse_expr_call(p);
  }

  if (check(p, LEXER_token_id)) 
    return ast_parse_expr_var(p);

  if (check(p, LEXER_token_dqstring)) 
    return ast_parse_expr_string_lit(p);
  
  if (check(p, LEXER_token_intlit))
      return ast_parse_expr_int_lit(p);

  return NULL;
}

declaration_t* ast_parse_function(parser_t* p)
{
  declaration_t* decl = (declaration_t*) malloc(sizeof(declaration_t));
  if (!decl) { 
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return NULL; 
  }
  memset(decl, 0, sizeof(declaration_t));

  decl->type = DECLARATION_FUNC;
  decl->source_pos = peek(p)->source_pos;

  // consume 'fn'
  advance(p);

  if (check(p, LEXER_token_id)) {
    token_t * name_tok = advance(p);
    if (name_tok->string_value) {
      decl->func.name = strdup(name_tok->string_value);
      if (!decl->func.name) { 
        error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
        free_declaration(decl); 
        return NULL; 
      }
    }
  } else {
    token_t* tok = peek(p);
    if (p->error_ctx && tok) {
      error_report_at_token(p->error_ctx, tok, ERROR_SEVERITY_ERROR,
                           "expected function name");
    }
    free_declaration(decl);
    return NULL;
  }

  if (!expect(p, '(', "expected '(' after function name")) {
    free_declaration(decl);
    return NULL;
  }

  while (!check(p, ')')) {
    typed_identifier_t param;

    if (!check(p, LEXER_token_id) || !check_is_type(p)) {
      token_t* tok = peek(p);
      if (p->error_ctx && tok) {
        error_report_at_token(p->error_ctx, tok, ERROR_SEVERITY_ERROR,
                             "expected parameter type");
      }
      free_declaration(decl);
      return NULL;
    }

    token_t* type_tok = advance(p);

    if (!type_tok->string_value) {
      if (p->error_ctx) {
        error_report_at_token(p->error_ctx, type_tok, ERROR_SEVERITY_ERROR,
                             "parameter type has no value");
      }
      free_declaration(decl);
      return NULL;
    }

    if (strcmp(type_tok->string_value, "var") == 0) {
      if (p->error_ctx) {
        error_report_at_token(p->error_ctx, type_tok, ERROR_SEVERITY_ERROR,
                              "`var` cannot be use for functions parameters. Use an explicit type instead");
      }
      free_declaration(decl);
      return NULL;
    }

    param.type = get_type_kind_from_string(type_tok->string_value);

    if (!check(p, LEXER_token_id)) {
      token_t* tok = peek(p);
      if (p->error_ctx && tok) {
        error_report_at_token(p->error_ctx, tok, ERROR_SEVERITY_ERROR,
                             "expected parameter name after type");
      }
      free_declaration(decl);
      return NULL;
    }
    token_t* name_tok = advance(p);

    if (!name_tok->string_value) {
      if (p->error_ctx) {
        error_report_at_token(p->error_ctx, name_tok, ERROR_SEVERITY_ERROR,
                             "parameter name has no value");
      }
      free_declaration(decl);
      return NULL;
    }

    param.name = strdup(name_tok->string_value);
    if (!param.name) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      free_declaration(decl);
      return NULL;
    }

    param.source_pos = name_tok->source_pos;

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
      token_t* tok = peek(p);
      if (p->error_ctx && tok) {
        error_report_at_token(p->error_ctx, tok, ERROR_SEVERITY_ERROR,
                             "expected return type after ':'");
      }
      free_declaration(decl);
      return NULL;
    }

    token_t* ret_tok = advance(p); 
    if (!ret_tok->string_value) {
      if (p->error_ctx) {
        error_report_at_token(p->error_ctx, ret_tok, ERROR_SEVERITY_ERROR,
                             "return type has no value");
      }
      free_declaration(decl);
      return NULL;
    }

    decl->func.return_type = get_type_kind_from_string(ret_tok->string_value);
  }

  if (!expect(p, '{', "expected '{' to start function body")) {
    free_declaration(decl);
    return NULL;
  }

  statement_t* s;
  statement_block_t* sb = (statement_block_t*) malloc(sizeof(statement_block_t));
  if (!sb) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    free_declaration(decl);
    return NULL;
  }
  memset(sb, 0, sizeof(statement_block_t));
  while ((s = parse_statement(p)) != NULL)
    da_append(sb, s);

  decl->func.body = sb;
  // consume '}'
  if (!expect(p, '}', "expected '}' after function body")) {
    free_declaration(decl);
    return NULL; 
  }
  return decl;
}

declaration_t* ast_parse_var_decl(parser_t* p)
{
  declaration_t* d = (declaration_t*) malloc(sizeof(declaration_t));
  if (!d) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return NULL;
  }
  memset(d, 0, sizeof(declaration_t));

  d->type = DECLARATION_VAR;
  d->source_pos = peek(p)->source_pos;

  // For now, we assume that we can only fall back here if the newt token is a typedef
  // Which mean that next token is an token_id with a string_value
  // This Might cause error later on
  token_t* type_tok = advance(p);

  if (!type_tok->string_value) {
    error_report_at_token(p->error_ctx, type_tok, ERROR_SEVERITY_ERROR,
       "expected type"); 
    free_declaration(d);
    return NULL;
  }

  if (strcmp(type_tok->string_value, "int") == 0) {
    d->var_decl.ident.type = TYPE_INT;
  } else if (strcmp(type_tok->string_value, "string") == 0) {
    d->var_decl.ident.type = TYPE_STRING;
  } else if (strcmp(type_tok->string_value, "var") == 0) {
    d->var_decl.ident.type = TYPE_UNTYPE; 
  } else {
    // For now, let's mark this as an error,
    // In a more advanced compiler this should be marked as TYPE_CUSTOM
    if (p->error_ctx) {
      error_report_at_token(p->error_ctx, type_tok, ERROR_SEVERITY_ERROR,
                           "unknown type '%s'", type_tok->string_value);
    }
    free_declaration(d);
    return NULL;
  }

  if (!check(p, LEXER_token_id)) {
    token_t* tok = peek(p);
    if (p->error_ctx && tok) {
      error_report_at_token(p->error_ctx, tok, ERROR_SEVERITY_ERROR,
                           "expected variable name");
    }
    free_declaration(d);
    return NULL;
  }

  token_t* name_tok = advance(p);
  if (!name_tok->string_value) {
    if (p->error_ctx) {
      error_report_at_token(p->error_ctx, name_tok, ERROR_SEVERITY_ERROR,
                           "variable name has no value");
    }
    free_declaration(d);
    return NULL;
  }

  d->var_decl.ident.name = strdup(name_tok->string_value);
  if (!d->var_decl.ident.name) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    free_declaration(d);
    return NULL;
  }

  d->var_decl.ident.source_pos = name_tok->source_pos;

  if (check(p, ';')) {
    // consume ';'
    advance(p);
    return d;
  }

  if (!expect(p, '=', "expected '=' in variable declaration")) {
    free_declaration(d);
    return NULL;
  }

  expression_t* e = parse_expression(p);
  if (!e) {
    token_t* tok = peek(p);
    if (p->error_ctx && tok) {
      error_report_at_token(p->error_ctx, tok, ERROR_SEVERITY_ERROR,
                           "expected expression in variable initialization");
    }
    free_declaration(d);
    return NULL;
  }
  d->var_decl.init = e;

  if (!expect(p, ';', "expected ';' after variable declaration")) {
    free_declaration(d);
    return NULL;
  }

  return d;
}

declaration_t* ast_parse_untype_var_decl(parser_t* p) 
{
  declaration_t* d = (declaration_t*) malloc(sizeof(declaration_t));
  if (!d) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return NULL;
  }
  memset(d, 0, sizeof(declaration_t));

  d->type = DECLARATION_VAR;
  d->source_pos = peek(p)->source_pos;

  // consume _var
  advance(p);

  d->var_decl.ident.type = TYPE_UNTYPE;

  if (!check(p, LEXER_token_id)) {
    error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR,
                            "expected identifier variable name");
    free_declaration(d);
    return NULL;
  }

  token_t* name_tok = advance(p);
  if (!name_tok->string_value) {
    error_report_at_token(p->error_ctx, name_tok, ERROR_SEVERITY_ERROR,
                            "identifier token has no value");
    free_declaration(d);
    return NULL;
  }

  d->var_decl.ident.name = strdup(name_tok->string_value);
  if (!d->var_decl.ident.name) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    free_declaration(d);
    return NULL;
  }

  d->var_decl.ident.source_pos = name_tok->source_pos;

  // case untype var as no init value
   if (check(p, ';')) {
    // consume ';'
    advance(p);
    return d;
  }

  if (!expect(p, '=', "expected '=' in variable declaration")) {
    free_declaration(d);
    return NULL;
  }

  expression_t* e = parse_expression(p);
  if (!e) {
    error_report_general(ERROR_SEVERITY_ERROR,
                          "error on expression parsing");
    free_declaration(d);
    return NULL;
  }

  d->var_decl.init = e;

  if (!expect(p, ';', "expected ';' after variable declaration")) {
    free_declaration(d);
    return NULL;
  }

  return d;
}

declaration_t* parse_declaration(parser_t* p)
{
  if (check(p, LEXER_token_id) && strcmp(peek(p)->string_value, "fn") == 0) {
    return ast_parse_function(p);
  }

  if (check(p, LEXER_token_id) && check_is_type(p)) {
    return ast_parse_var_decl(p);
  }

  if (check(p, LEXER_token_id) && strcmp(peek(p)->string_value, "var") == 0) {
    return ast_parse_untype_var_decl(p);
  }

  return NULL;
}

statement_t* ast_parse_return_stmt(parser_t* p) 
{
  statement_t* s = (statement_t*) malloc(sizeof(statement_t));
  if (s == NULL) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return NULL;
  }
  memset(s, 0, sizeof(statement_t));

  s->type = STATEMENT_RETURN;
  s->source_pos = peek(p)->source_pos;

  // consume 'return'
  advance(p);

  s->ret.value = parse_expression(p);

  if (!expect(p, ';', "expected ';' after return statement")) {
    free_statement(s);
    return NULL;
  }

  return s;
}

statement_t* ast_parse_decl_stmt(parser_t* p) 
{
  statement_t* s = (statement_t*) malloc(sizeof(statement_t));
  if (s == NULL) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return NULL;
  }
  memset(s, 0, sizeof(statement_t));
  
  s->type = STATEMENT_DECL;
  s->source_pos = peek(p)->source_pos;
  
  s->decl_stmt.decl = parse_declaration(p);
  if (!s->decl_stmt.decl) {
    // Error already reported by ast_parse_var_decl
    free_statement(s);
    return NULL;
  }

  return s;
}

statement_t* ast_parse_expr_stmt(parser_t* p) 
{
  statement_t* s = (statement_t*) malloc(sizeof(statement_t));
  if (!s) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return NULL;
  }
  memset(s, 0, sizeof(statement_t));

  s->type = STATEMENT_EXPR;
  s->source_pos = peek(p)->source_pos;

  s->expr_stmt.expr = parse_expression(p);
  if (!s->expr_stmt.expr) {
    // Error already reported by parse_expression
    free_statement(s);
    return NULL;
  }

  if(!expect(p, ';', "expected ';' after expression statement")) {
    free_statement(s);
    return NULL; 
  }

  return s;
}

statement_t* ast_parse_if_stmt(parser_t* p) 
{
  statement_t* s = (statement_t*) malloc(sizeof(statement_t));
  if (!s) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return NULL; 
  }
  memset(s, 0, sizeof(statement_t));

  s->type = STATEMENT_IF;
  s->source_pos = peek(p)->source_pos;

  // consume _if
  advance(p);

  if (!expect(p, '(', "expected '(' after if statement")) {
    free_statement(s);  
    return NULL;
  }
  s->if_stmt.condition = parse_expression(p);
  if (!s->if_stmt.condition) {
    error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR,
             "expected condition"); 
    free_statement(s);
    return NULL;
  }

  if (!expect(p, ')', "expected ')' after expression")) {
    free_statement(s);
    return NULL; 
  }

  if (!expect(p, '{', "expected '{' after statement")) {
    free_statement(s);
    return NULL;  
  }

  statement_block_t* then_sb = (statement_block_t*) malloc(sizeof(statement_block_t));
  if (!then_sb) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    free_statement(s);
    return NULL; 
  }
  memset(then_sb, 0, sizeof(statement_block_t));
  s->if_stmt.then_branch = then_sb;
  
  while (!check(p, '}')) {
    statement_t* stmt = parse_statement(p);
    if (!stmt) {
      error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR, 
            "expected statement"); 
      free_statement(s);
      return NULL;
    }
    da_append(then_sb, stmt);
  }

  if (!expect(p, '}', "expected '}' after if body")) {
    free_statement(s); 
    return NULL;
  }

  if (check(p, LEXER_token_id) && strcmp(peek(p)->string_value, "else") == 0) {
    advance(p); 

    if (!expect(p, '{', "expected '{' after else stmt")) {
      free_statement(s);
      return NULL; 
    }

    statement_block_t* else_sb = (statement_block_t*) malloc(sizeof(statement_block_t));
    if (!else_sb) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      free_statement(s);
      return NULL;
    }
    memset(else_sb, 0, sizeof(statement_block_t));
    s->if_stmt.else_branch = else_sb;
    
    while (!check(p, '}')) {
      statement_t* stmt = parse_statement(p);
      if (!stmt) {
        error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
        free_statement(s);
        return NULL;
      } 
      da_append(else_sb, stmt);
    }

    if (!expect(p, '}', "expected '}' after else body")) {
      free_statement(s);
      return NULL;
    }
  }

  return s;
}

statement_t* ast_parse_while_stmt(parser_t* p)
{
  statement_t* s = (statement_t*) malloc(sizeof(statement_t));
  if (!s) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return NULL;
  }
  memset(s, 0, sizeof(statement_t));

  s->type = STATEMENT_WHILE;
  s->source_pos = peek(p)->source_pos;

  // consume _while
  advance(p);

  if (!expect(p, '(', "expected '(' after while identifier")) {
    free_statement(s);
    return NULL; 
  }

  expression_t* cond = parse_expression(p);
  if (!cond) {
    error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR,
        "expected condition in while statement");
    free_statement(s);
    return NULL;
  }

  s->while_stmt.condition = cond;

  if (!expect(p, ')', "expected ')' after condition")) {
    free_statement(s); 
    return NULL;
  }

  if (!expect(p, '{', "expected '{' after while declaration")) {
    free_statement(s);  
    return NULL;
  }

  statement_block_t* block = (statement_block_t*) malloc(sizeof(statement_block_t));
  if (!block) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    free_statement(s);
    return NULL;
  }
  memset(block, 0, sizeof(statement_block_t));
  s->while_stmt.body = block;
  
  while(!check(p, '}')) {
    statement_t* stmt = parse_statement(p); 
    if (!stmt) {
      error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR,
         "expected statement in while body"); 
      free_statement(s);
      return NULL;
    }
    da_append(block, stmt);
  }

  if (!expect(p, '}', "expected '}' after while body")) {
    free_statement(s); 
    return NULL;
  }

  return s;
}

statement_t* ast_parse_for_stmt(parser_t* p)
{
  statement_t* s = (statement_t*) malloc(sizeof(statement_t));
  if (!s) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return NULL;
  }
  memset(s, 0, sizeof(statement_t));

  s->type = STATEMENT_FOR;
  s->source_pos = peek(p)->source_pos;

  // consume _for
  advance(p);

  if (!expect(p, '(', "expected '(' after for statement")) {
    free_statement(s);
    return NULL; 
  }
  
  if (check_is_type(p) || strcmp(peek(p)->string_value, "var") == 0) {
    declaration_t* init = parse_declaration(p);
    if (!init) {
      error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR,
         "expected init declaration"); 
      free_statement(s);
      return NULL;
    }
    s->for_stmt.decl_init = init;
  } else {
    expression_t* init = parse_expression(p);
    if (!init) {
      error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR,
          "expected init expression"); 
      free_statement(s);
      return NULL;
    }
    s->for_stmt.expr_init = init;

    if (!expect(p, ';', "expected ';' between for expressions")) {
      free_statement(s);
      return NULL; 
    }  
  }

  expression_t* condition = parse_expression(p);
  if (!condition) {
    error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR,
       "expected condition expression"); 
    free_statement(s);
    return NULL;
  }
  s->for_stmt.condition = condition;

  if (!expect(p, ';', "expected ';' between for expressions")) {
    free_statement(s);
    return NULL; 
  }

  expression_t* loop = parse_expression(p);
  if (!loop) {
    error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR,
       "expected loop expression"); 
    free_statement(s);
    return NULL;
  }
  s->for_stmt.loop = loop;

  if (!expect(p, ')', "expected ')' after for statement")) {
    free_statement(s);
    return NULL; 
  }

  if (!expect(p, '{', "expected '{' after for declaration")) {
    free_statement(s); 
    return NULL;
  }

  statement_block_t* body = (statement_block_t*) malloc(sizeof(statement_block_t));
  if (!body) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    free_statement(s);
    return NULL;
  }
  memset(body, 0, sizeof(statement_block_t));
  s->for_stmt.body = body;

  while(!check(p, '}')) {
    statement_t* stmt = parse_statement(p); 
    if (!stmt) {
      error_report_at_token(p->error_ctx, peek(p), ERROR_SEVERITY_ERROR,
         "expected statement"); 
      free_statement(s);
      return NULL;
    }
    da_append(body, stmt);
  }

  if (!expect(p, '}', "expected '}' after body")) {
    free_statement(s);
    return NULL; 
  }

  return s;
}

statement_t* parse_statement(parser_t* p) 
{
  if (check(p, LEXER_token_id) && strcmp(peek(p)->string_value, "return") == 0) {
    return ast_parse_return_stmt(p);
  } 

  if (check(p, LEXER_token_id) && strcmp(peek(p)->string_value, "if") == 0) {
    return ast_parse_if_stmt(p);
  }

  if (check(p, LEXER_token_id) && strcmp(peek(p)->string_value, "while") == 0) {
    return ast_parse_while_stmt(p);
  }

  if (check(p, LEXER_token_id) && strcmp(peek(p)->string_value, "for") == 0) {
    return ast_parse_for_stmt(p); 
  }

  // WARNING: this can be unsafe if string_value is NULL
  // TODO: keep an eye on this
  if (check(p, LEXER_token_id) && ((check_is_type(p)) || strcmp(peek(p)->string_value, "var") == 0)) {
    return ast_parse_decl_stmt(p);
  } 

  if ((size_t) p->pos < p->count) 
    return ast_parse_expr_stmt(p);

  return NULL;
}
