#define CTEST_BEFORE_EACH
#define CTEST_LIB_IMPLEMENTATION
#include "ctest.h"

#define LEXER_LIB_IMPLEMENTATION
#include "../src/frontend/lexer.h"
#define DA_LIB_IMPLEMENTATION
#include "../src/thirdparty/da.h"

#include "../src/frontend/ast_definition.h"
#include "../src/frontend/ast.h"

// before_each setup: build parser from given source code
before_each(parser_t, parser, char* source_code)
{
  parser_t p = {0};
  lexer_t lex;

  char* storage = malloc(255);
  lexer_init_lexer(&lex, source_code, source_code + strlen(source_code), (char*)storage, 255);

  while (lexer_get_token(&lex)) {
    if (lex.token == LEXER_token_parse_error) {
      break;
    }

    token_t t = lexer_copy_token(&lex);
    da_append(&p, t);
  }

  free(storage);
  parser = p;
}

// === FUNCTION DECLARATIONS TESTS ===

ct_test(ast, fn_def, "fn main() {}")
{
  declaration_t* decl = parse_declaration(&parser);

  ct_assert_not_null(decl, "Declaration should not be NULL");
  ct_assert_eq(decl->type, DECLARATION_FUNC, "Declaration should be of type FUNCTION");
  ct_assert_not_null(decl->func.name, "Function name should not be NULL");
  ct_assert_eq(decl->func.name, "main", "Function name should match source code");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, fn_ret_type, "fn main(): int {}")
{
  declaration_t* decl = parse_declaration(&parser);

  ct_assert_not_null(decl, "Declaration should not be NULL");
  ct_assert_not_null(decl->func.return_type.name, "Return type should not be NULL");
  ct_assert_eq(decl->func.return_type.name, "int", "Return type name should match source code");
  ct_assert_eq(decl->func.return_type.kind, TYPE_INT, "Return type kind should be TYPE_INT");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, fn_params, "fn main(int a, string b) {}")
{
  declaration_t* decl = parse_declaration(&parser);
  ct_assert_not_null(decl, "Declaration should not be NULL");
  ct_assert_eq((int)decl->func.params.count, 2, "Function should have 2 parameters");

  typed_identifier_t p1 = decl->func.params.items[0];
  typed_identifier_t p2 = decl->func.params.items[1];

  ct_assert_eq(p1.name, "a", "First parameter name should be 'a'");
  ct_assert_eq(p1.type.kind, TYPE_INT, "First parameter type should be TYPE_INT");
  ct_assert_eq(p1.type.name, "int", "First parameter type name should be 'int'");

  ct_assert_eq(p2.name, "b", "Second parameter name should be 'b'");
  ct_assert_eq(p2.type.kind, TYPE_STRING, "Second parameter type should be TYPE_STRING");
  ct_assert_eq(p2.type.name, "string", "Second parameter type name should be 'string'");

  free_declaration(decl);
  da_free(&parser);
}

// === VARIABLE DECLARATIONS ===

ct_test(ast, typed_int_var, "int i = 3;")
{
  declaration_t* decl = parse_declaration(&parser);

  ct_assert_eq(decl->var_decl.ident.name, "i", "Variable name should be 'i'");
  ct_assert_eq(decl->var_decl.ident.type.name, "int", "Variable type name should be 'int'");
  ct_assert_eq(decl->var_decl.ident.type.kind, TYPE_INT, "Variable type should be TYPE_INT");
  ct_assert_not_null(decl->var_decl.init, "Variable init expression should not be NULL");
  ct_assert_eq(decl->var_decl.init->type, EXPRESSION_INT_LIT, "Init expression should be INT literal");
  ct_assert_eq(decl->var_decl.init->int_lit.value, 3, "Init int value should be 3");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, typed_string_var, "string i = \"test\";")
{
  declaration_t* decl = parse_declaration(&parser);

  ct_assert_eq(decl->var_decl.ident.name, "i", "Variable name should be 'i'");
  ct_assert_eq(decl->var_decl.ident.type.name, "string", "Variable type name should be 'string'");
  ct_assert_eq(decl->var_decl.ident.type.kind, TYPE_STRING, "Variable type should be TYPE_STRING");
  ct_assert_eq(decl->var_decl.init->type, EXPRESSION_STRING_LIT, "Init expression should be STRING literal");
  ct_assert_eq(decl->var_decl.init->string_lit.value, "test", "Init string literal should be 'test'");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, untyped_var_decl, "var i = 3;")
{
  declaration_t* decl = parse_declaration(&parser);

  ct_assert_eq(decl->type, DECLARATION_VAR, "Declaration type should be VAR");
  ct_assert_eq(decl->var_decl.ident.name, "i", "Variable name should be 'i'");
  ct_assert_eq(decl->var_decl.ident.type.kind, TYPE_UNTYPE, "Variable type should be UNTYPE");
  ct_assert_eq(decl->var_decl.ident.type.name, "var", "Variable type name should be 'var'");
  ct_assert_eq(decl->var_decl.init->type, EXPRESSION_INT_LIT, "Init expression should be INT literal");
  ct_assert_eq(decl->var_decl.init->int_lit.value, 3, "Init int literal value should be 3");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, uninitialized_var_decl, "var i;")
{
  declaration_t* decl = parse_declaration(&parser);

  ct_assert_eq(decl->type, DECLARATION_VAR, "Declaration type should be VAR");
  ct_assert_eq(decl->var_decl.ident.name, "i", "Variable name should be 'i'");
  ct_assert_eq(decl->var_decl.ident.type.kind, TYPE_UNTYPE, "Variable type should be UNTYPE");
  ct_assert_eq(decl->var_decl.ident.type.name, "var", "Variable type name should be 'var'");
  ct_assert(!decl->var_decl.init, "Init expression should be NULL for uninitialized var");

  free_declaration(decl);
  da_free(&parser);
}

// === RETURN STATEMENTS ===

ct_test(ast, return_int, "return 1;")
{
  statement_t* s = parse_statement(&parser);

  ct_assert_eq(s->type, STATEMENT_RETURN, "Statement should be RETURN");
  ct_assert_eq(s->ret.value->type, EXPRESSION_INT_LIT, "Return expression should be INT literal");
  ct_assert_eq(s->ret.value->int_lit.value, 1, "Return int value should be 1");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, return_string, "return \"test\";")
{
  statement_t* s = parse_statement(&parser);

  ct_assert_eq(s->type, STATEMENT_RETURN, "Statement should be RETURN");
  ct_assert_eq(s->ret.value->type, EXPRESSION_STRING_LIT, "Return expression should be STRING literal");
  ct_assert_eq(s->ret.value->string_lit.value, "test", "Return string literal should be 'test'");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, return_var, "return a;")
{
  statement_t* s = parse_statement(&parser);

  ct_assert_eq(s->type, STATEMENT_RETURN, "Statement should be RETURN");
  ct_assert_eq(s->ret.value->type, EXPRESSION_VAR, "Return expression should be VAR");
  ct_assert_eq(s->ret.value->var.name, "a", "Returned var name should be 'a'");

  free_statement(s);
  da_free(&parser);
}

// === EXPRESSIONS ===

ct_test(ast, expr_assign, "i = 4;")
{
  statement_t* s = parse_statement(&parser);
  expression_t* e = s->expr_stmt.expr;

  ct_assert_eq(e->type, EXPRESSION_ASSIGN, "Expression should be ASSIGN");
  ct_assert_eq(e->assign.lhs->var.name, "i", "LHS var name should be 'i'");
  ct_assert_eq(e->assign.rhs->int_lit.value, 4, "RHS int literal value should be 4");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, expr_binary, "i + 4;")
{
  statement_t* s = parse_statement(&parser);
  expression_t* e = s->expr_stmt.expr;

  ct_assert_eq(e->type, EXPRESSION_BINARY, "Expression type should be BINARY");
  ct_assert_eq(e->binary.op, BINARY_PLUS, "Binary op should be PLUS");
  ct_assert_eq(e->binary.left->var.name, "i", "Left operand var should be 'i'");
  ct_assert_eq(e->binary.right->int_lit.value, 4, "Right operand value should be 4");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, expr_binary_gt, "i > 5;")
{
  statement_t* s = parse_statement(&parser);
  expression_t* e = s->expr_stmt.expr;

  ct_assert_eq(e->type, EXPRESSION_BINARY, "Expression type should be BINARY");
  ct_assert_eq(e->binary.op, BINARY_GT, "Binary op should be GT");
  ct_assert_eq(e->binary.left->var.name, "i", "Left operand var should be 'i'");
  ct_assert_eq(e->binary.right->int_lit.value, 5, "Right operand value should be 5");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, function_call, "test(a, 5, \"test\");")
{
  statement_t* s = parse_statement(&parser);
  expression_t* e = s->expr_stmt.expr;

  ct_assert_eq(e->type, EXPRESSION_CALL, "Expression type should be CALL");
  ct_assert_eq(e->call.callee, "test", "Function name should be 'test'");
  ct_assert_eq((int)e->call.arg_count, 3, "Function call should have 3 args");

  ct_assert_eq(e->call.args[0]->var.name, "a", "Arg1 should be var 'a'");
  ct_assert_eq(e->call.args[1]->int_lit.value, 5, "Arg2 should be int 5");
  ct_assert_eq(e->call.args[2]->string_lit.value, "test", "Arg3 should be string 'test'");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, unary_pre_inc, "++i;") 
{
  statement_t* s = parse_statement(&parser);
  expression_t* e = s->expr_stmt.expr;

  ct_assert_eq(e->type, EXPRESSION_UNARY, "Expression type should be UNARY");
  ct_assert_eq(e->unary.op, UNARY_PRE_INC, "Unary op should be PRE_INC");
  ct_assert_eq(e->unary.operand->var.name, "i", "Unary operand name should be 'i'");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, unary_post_dec, "i--;")
{
  statement_t* s = parse_statement(&parser);
  expression_t* e = s->expr_stmt.expr;

  ct_assert_eq(e->type, EXPRESSION_UNARY, "Expression type should be UNARY");
  ct_assert_eq(e->unary.op, UNARY_POST_DEC, "Unary op should be POST_DEC");
  ct_assert_eq(e->unary.operand->var.name, "i", "Unary operand name should be 'i'");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, unary_single_char, "!i;")
{
  statement_t* s = parse_statement(&parser);
  expression_t* e = s->expr_stmt.expr;

  ct_assert_eq(e->type, EXPRESSION_UNARY, "Expression type should be UNARY");
  ct_assert_eq(e->unary.op, UNARY_NOT, "Unary op should be NOT");
  ct_assert_eq(e->unary.operand->var.name, "i", "Unary operand name should be 'i'");

  free_statement(s);
  da_free(&parser);
}

// === CONTROL FLOW ===

ct_test(ast, if_statement, "if (a == 4) { a = 3; } else { a = 4; }")
{
  statement_t* s = parse_statement(&parser);
  ct_assert_eq(s->type, STATEMENT_IF, "Statement type should be IF");
  expression_t* cond = s->if_stmt.condition;

  ct_assert_eq(cond->type, EXPRESSION_BINARY, "Condition should be BINARY expression");
  ct_assert_eq(cond->binary.op, BINARY_EQ, "Binary op should be EQ");
  ct_assert_eq(cond->binary.left->var.name, "a", "LHS var name should be 'a'");
  ct_assert_eq(cond->binary.right->int_lit.value, 4, "RHS literal should be 4");

  statement_t* then_stmt = s->if_stmt.then_branch->items[0];
  statement_t* else_stmt = s->if_stmt.else_branch->items[0];

  ct_assert_eq(then_stmt->expr_stmt.expr->assign.rhs->int_lit.value, 3, "Then branch assigns 3");
  ct_assert_eq(else_stmt->expr_stmt.expr->assign.rhs->int_lit.value, 4, "Else branch assigns 4");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, while_statement, "while (i == 10) { i = 3; }") 
{
  statement_t* s = parse_statement(&parser);
  ct_assert_eq(s->type, STATEMENT_WHILE, "Statemet type should be WHILE");
  expression_t* cond = s->while_stmt.condition;

  ct_assert_eq(cond->type, EXPRESSION_BINARY, "Condition should be BINARY expression");
  ct_assert_eq(cond->binary.op, BINARY_PLUS, "Binary op should be EQ");
  ct_assert_eq(cond->binary.left->var.name, "i", "LHS var name should be 'i'");
  ct_assert_eq(cond->binary.right->int_lit.value, 10, "RHS literal should be 10");

  statement_t* body = s->while_stmt.body->items[0];

  ct_assert_eq(body->expr_stmt.expr->assign.lhs->var.name, "i", "LHS var name should be 'i'");
  ct_assert_eq(body->expr_stmt.expr->assign.rhs->int_lit.value, 3, "RHS literal value should be 3");

  free_statement(s);
  da_free(&parser);
}
