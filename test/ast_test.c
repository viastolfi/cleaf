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

  p.types = calloc(1, sizeof(known_type_array));
  populate_parser_known_type(p.types);

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
  ct_assert_eq(decl->func.return_type.kind, TYPE_INT, "Return type kind should be TYPE_INT");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, fn_params, "fn main(int a, int b) {}")
{
  declaration_t* decl = parse_declaration(&parser);
  ct_assert_not_null(decl, "Declaration should not be NULL");
  ct_assert_eq((int)decl->func.params.count, 2, "Function should have 2 parameters");

  typed_identifier_t p1 = decl->func.params.items[0];
  typed_identifier_t p2 = decl->func.params.items[1];

  ct_assert_eq(p1.ident_name, "a", "First parameter name should be 'a'");
  ct_assert_eq(p1.type.kind, TYPE_INT, "First parameter type should be TYPE_INT");

  ct_assert_eq(p2.ident_name, "b", "Second parameter name should be 'b'");
  ct_assert_eq(p2.type.kind, TYPE_INT, "Second parameter type should be TYPE_INT");

  free_declaration(decl);
  da_free(&parser);
}

// === VARIABLE DECLARATIONS ===

ct_test(ast, typed_int_var, "int i = 3;")
{
  declaration_t* decl = parse_declaration(&parser);

  ct_assert_eq(decl->var_decl.ident.ident_name, "i", "Variable name should be 'i'");
  ct_assert_eq(decl->var_decl.ident.type.kind, TYPE_INT, "Variable type should be TYPE_INT");
  ct_assert_not_null(decl->var_decl.init, "Variable init expression should not be NULL");
  ct_assert_eq(decl->var_decl.init->type, EXPRESSION_INT_LIT, "Init expression should be INT literal");
  ct_assert_eq(decl->var_decl.init->int_lit.value, 3, "Init int value should be 3");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, untyped_var_decl, "var i = 3;")
{
  declaration_t* decl = parse_declaration(&parser);

  ct_assert_eq(decl->type, DECLARATION_VAR, "Declaration type should be VAR");
  ct_assert_eq(decl->var_decl.ident.ident_name, "i", "Variable name should be 'i'");
  ct_assert_eq(decl->var_decl.ident.type.kind, TYPE_VAR, "Variable type should be VAR");
  ct_assert_eq(decl->var_decl.init->type, EXPRESSION_INT_LIT, "Init expression should be INT literal");
  ct_assert_eq(decl->var_decl.init->int_lit.value, 3, "Init int literal value should be 3");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, uninitialized_var_decl, "var i;")
{
  declaration_t* decl = parse_declaration(&parser);

  ct_assert_eq(decl->type, DECLARATION_VAR, "Declaration type should be VAR");
  ct_assert_eq(decl->var_decl.ident.ident_name, "i", "Variable name should be 'i'");
  ct_assert_eq(decl->var_decl.ident.type.kind, TYPE_VAR, "Variable type should be VAR");
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

ct_test(ast, return_var, "return a;")
{
  statement_t* s = parse_statement(&parser);

  ct_assert_eq(s->type, STATEMENT_RETURN, "Statement should be RETURN");
  ct_assert_eq(s->ret.value->type, EXPRESSION_VAR, "Return expression should be VAR");
  ct_assert_eq(s->ret.value->var.ident.ident_name, "a", "Returned var name should be 'a'");

  free_statement(s);
  da_free(&parser);
}

// === EXPRESSIONS ===

ct_test(ast, expr_assign, "i = 4;")
{
  statement_t* s = parse_statement(&parser);
  expression_t* e = s->expr_stmt.expr;

  ct_assert_eq(e->type, EXPRESSION_ASSIGN, "Expression should be ASSIGN");
  ct_assert_eq(e->assign.lhs->var.ident.ident_name, "i", "LHS var name should be 'i'");
  ct_assert_eq(e->assign.rhs->int_lit.value, 4, "RHS int literal value should be 4");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, expr_binary, "i + 4;")
{
  statement_t* s = parse_statement(&parser);
  expression_t* e = s->expr_stmt.expr;

  ct_assert_eq(e->type, EXPRESSION_BINARY, "Expression type should be BINARY");
  ct_assert_eq(e->binary.op, BINARY_ADD, "Binary op should be ADD");
  ct_assert_eq(e->binary.left->var.ident.ident_name, "i", "Left operand var should be 'i'");
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
  ct_assert_eq(e->binary.left->var.ident.ident_name, "i", "Left operand var should be 'i'");
  ct_assert_eq(e->binary.right->int_lit.value, 5, "Right operand value should be 5");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, function_call, "test(a, 5);")
{
  statement_t* s = parse_statement(&parser);
  expression_t* e = s->expr_stmt.expr;

  ct_assert_eq(e->type, EXPRESSION_CALL, "Expression type should be CALL");
  ct_assert_eq(e->call.callee, "test", "Function name should be 'test'");
  ct_assert_eq((int)e->call.arg_count, 2, "Function call should have 3 args");

  ct_assert_eq(e->call.args[0]->var.ident.ident_name, "a", "Arg1 should be var 'a'");
  ct_assert_eq(e->call.args[1]->int_lit.value, 5, "Arg2 should be int 5");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, unary_pre_inc, "++i;") 
{
  statement_t* s = parse_statement(&parser);
  expression_t* e = s->expr_stmt.expr;

  ct_assert_eq(e->type, EXPRESSION_UNARY, "Expression type should be UNARY");
  ct_assert_eq(e->unary.op, UNARY_PRE_INC, "Unary op should be PRE_INC");
  ct_assert_eq(e->unary.operand->var.ident.ident_name, "i", "Unary operand name should be 'i'");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, unary_post_dec, "i--;")
{
  statement_t* s = parse_statement(&parser);
  expression_t* e = s->expr_stmt.expr;

  ct_assert_eq(e->type, EXPRESSION_UNARY, "Expression type should be UNARY");
  ct_assert_eq(e->unary.op, UNARY_POST_DEC, "Unary op should be POST_DEC");
  ct_assert_eq(e->unary.operand->var.ident.ident_name, "i", "Unary operand name should be 'i'");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, unary_single_char, "!i;")
{
  statement_t* s = parse_statement(&parser);
  expression_t* e = s->expr_stmt.expr;

  ct_assert_eq(e->type, EXPRESSION_UNARY, "Expression type should be UNARY");
  ct_assert_eq(e->unary.op, UNARY_NOT, "Unary op should be NOT");
  ct_assert_eq(e->unary.operand->var.ident.ident_name, "i", "Unary operand name should be 'i'");

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
  ct_assert_eq(cond->binary.left->var.ident.ident_name, "a", "LHS var name should be 'a'");
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
  ct_assert_eq(cond->binary.op, BINARY_EQ, "Binary op should be EQ");
  ct_assert_eq(cond->binary.left->var.ident.ident_name, "i", "LHS var name should be 'i'");
  ct_assert_eq(cond->binary.right->int_lit.value, 10, "RHS literal should be 10");

  statement_t* body = s->while_stmt.body->items[0];

  ct_assert_eq(body->expr_stmt.expr->assign.lhs->var.ident.ident_name, "i", "LHS var name should be 'i'");
  ct_assert_eq(body->expr_stmt.expr->assign.rhs->int_lit.value, 3, "RHS literal value should be 3");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, for_statement, "for (var i = 0; i < 10; ++i) { b = 3; }")
{
  statement_t* s = parse_statement(&parser);
  ct_assert_eq(s->type, STATEMENT_FOR, "Statement type should be FOR");
  ct_assert_not_null(s->for_stmt.decl_init, "For statement declaration should not be NULL");
  ct_assert_eq(s->for_stmt.decl_init->type, DECLARATION_VAR, "Declaration type should be VAR");
  expression_t* cond = s->for_stmt.condition;
  ct_assert_eq(cond->type, EXPRESSION_BINARY, "Condition type should be BINARY");
  ct_assert_eq(cond->binary.op, BINARY_LT, "Binary expression type should be LT");
  expression_t* loop = s->for_stmt.loop;
  ct_assert_eq(loop->type, EXPRESSION_UNARY, "Loop type should be UNARY");
  ct_assert_eq(loop->unary.op, UNARY_PRE_INC, "Unary expression type should be PRE_INC");
  ct_assert_eq(s->for_stmt.body->items[0]->expr_stmt.expr->type, EXPRESSION_ASSIGN, "Body expression type should be assign");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, var_function_param, "fn main(var a): int { return a; }")
{
  declaration_t* decl = parse_declaration(&parser);
  ct_assert_null(decl, "Decl should be NULL");
}

ct_test(ast, struct_declaration, "struct v2 { int a; int b; }") 
{
  declaration_t* decl = parse_declaration(&parser);
  ct_assert_not_null(decl, "struct decl should not be NULL");
  ct_assert_eq(decl->type, DECLARATION_STRUCT, "decl type should be DECLARATION_STRUCT");
  ct_assert_not_null(decl->struc.name, "struct name should not be NULL");
  ct_assert_eq(decl->struc.name, "v2", "struct name should be the same as writted in code");
  ct_assert_eq(decl->struc.members.count, 2, "struct should have two members");
  ct_assert_not_null(decl->struc.members.items[0].ident_name, "first struct member name should not be NULL");
  ct_assert_eq(decl->struc.members.items[0].ident_name, "a", "first struct member should have same name as defined in code");
  ct_assert_eq(decl->struc.members.items[0].type.kind, TYPE_INT, "first struct member should have same type as declared in code");
  ct_assert_not_null(decl->struc.members.items[1].ident_name, "second struct member name should not be NULL");
  ct_assert_eq(decl->struc.members.items[1].ident_name, "b", "second struct member should have same name as defined in code");
  ct_assert_eq(decl->struc.members.items[1].type.kind, TYPE_INT, "second struct member should have same type as declared in code");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, struct_declaration_var, "struct v1 { var a; int b }")
{
  declaration_t* decl = parse_declaration(&parser);
  ct_assert_null(decl, "decl should be NULL if defined with 'var' as one of its member");

  da_free(&parser);
}

ct_test(ast, struct_declaration_unknown_type, "struct v2 { dump a; int b; }") 
{
  declaration_t* decl = parse_declaration(&parser);
  ct_assert_null(decl, "decl should be NULL if defined with unkown type as one of its member");

  da_free(&parser);
}

ct_test(ast, struct_var_zero_init, "struct v2 { int a; int b; } v2 a = { 0 };")
{
  declaration_t* struct_decl = parse_declaration(&parser);
  ct_assert_not_null(struct_decl, "struct decl should not be NULL");

  declaration_t* decl = parse_declaration(&parser);
  ct_assert_not_null(decl, "var decl should not be NULL");
  ct_assert_eq(decl->type, DECLARATION_VAR, "declaration type should be VAR");
  ct_assert_eq(decl->var_decl.ident.type.kind, TYPE_CUSTOM, "var type should be TYPE_CUSTOM");
  ct_assert_not_null(decl->var_decl.init, "init expression should not be NULL");
  ct_assert_eq(decl->var_decl.init->type, EXPRESSION_COMPOSITE_LITERAL, "init expression should be COMPOSITE_LITERAL");
  ct_assert_eq((int)decl->var_decl.init->composite_literal.is_initializer, 0, "is_initializer should be false for zero init");

  free_declaration(struct_decl);
  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, struct_var_designated_init, "struct v2 { int a; int b; } v2 a = { .a = 1, .b = 2 };")
{
  declaration_t* struct_decl = parse_declaration(&parser);
  ct_assert_not_null(struct_decl, "struct decl should not be NULL");

  declaration_t* decl = parse_declaration(&parser);
  ct_assert_not_null(decl, "var decl should not be NULL");
  ct_assert_eq(decl->type, DECLARATION_VAR, "declaration type should be VAR");
  ct_assert_not_null(decl->var_decl.init, "init expression should not be NULL");
  ct_assert_eq(decl->var_decl.init->type, EXPRESSION_COMPOSITE_LITERAL, "init should be COMPOSITE_LITERAL");
  ct_assert_eq((int)decl->var_decl.init->composite_literal.is_initializer, 1, "is_initializer should be true for designated init");
  ct_assert_eq((int)decl->var_decl.init->composite_literal.count, 2, "should have 2 field assignments");
  ct_assert_eq(decl->var_decl.init->composite_literal.values[0]->assign.lhs->var.ident.ident_name, "a", "first field name should be 'a'");
  ct_assert_eq(decl->var_decl.init->composite_literal.values[0]->assign.rhs->int_lit.value, 1, "first field value should be 1");
  ct_assert_eq(decl->var_decl.init->composite_literal.values[1]->assign.lhs->var.ident.ident_name, "b", "second field name should be 'b'");
  ct_assert_eq(decl->var_decl.init->composite_literal.values[1]->assign.rhs->int_lit.value, 2, "second field value should be 2");

  free_declaration(struct_decl);
  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, struct_var_designated_init_single, "struct v1 { int x; } v1 a = { .x = 42 };")
{
  declaration_t* struct_decl = parse_declaration(&parser);
  ct_assert_not_null(struct_decl, "struct decl should not be NULL");

  declaration_t* decl = parse_declaration(&parser);
  ct_assert_not_null(decl, "var decl should not be NULL");
  ct_assert_eq(decl->type, DECLARATION_VAR, "declaration type should be VAR");
  ct_assert_not_null(decl->var_decl.init, "init expression should not be NULL");
  ct_assert_eq(decl->var_decl.init->type, EXPRESSION_COMPOSITE_LITERAL, "init should be COMPOSITE_LITERAL");
  ct_assert_eq((int)decl->var_decl.init->composite_literal.is_initializer, 1, "is_initializer should be true for designated init");
  ct_assert_eq((int)decl->var_decl.init->composite_literal.count, 1, "should have exactly 1 field assignment");
  ct_assert_eq(decl->var_decl.init->composite_literal.values[0]->assign.lhs->var.ident.ident_name, "x", "field name should be 'x'");
  ct_assert_eq(decl->var_decl.init->composite_literal.values[0]->assign.rhs->int_lit.value, 42, "field value should be 42");

  free_declaration(struct_decl);
  free_declaration(decl);
  da_free(&parser);
}

// === ASM STATEMENTS ===

ct_test(ast, asm_single_instr, "asm(\"mov rax, 60\");")
{
  statement_t* s = parse_statement(&parser);

  ct_assert_not_null(s, "asm statement should not be NULL");
  ct_assert_eq(s->type, STATEMENT_ASM, "statement type should be STATEMENT_ASM");
  ct_assert_eq((int)s->asm_stmt.instr_count, 1, "asm statement should have 1 instruction");
  ct_assert_eq((int)s->asm_stmt.arg_count, 0, "asm statement should have 0 args");
  ct_assert_eq(s->asm_stmt.instr[0], "mov rax, 60", "instruction string should match");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, asm_multiple_instr, "asm(\"mov rax, 60\", \"xor rdi, rdi\", \"syscall\");")
{
  statement_t* s = parse_statement(&parser);

  ct_assert_not_null(s, "asm statement should not be NULL");
  ct_assert_eq(s->type, STATEMENT_ASM, "statement type should be STATEMENT_ASM");
  ct_assert_eq((int)s->asm_stmt.instr_count, 3, "asm statement should have 3 instructions");
  ct_assert_eq((int)s->asm_stmt.arg_count, 0, "asm statement should have 0 args");
  ct_assert_eq(s->asm_stmt.instr[0], "mov rax, 60", "first instruction should match");
  ct_assert_eq(s->asm_stmt.instr[1], "xor rdi, rdi", "second instruction should match");
  ct_assert_eq(s->asm_stmt.instr[2], "syscall", "third instruction should match");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, asm_with_args, "asm(\"mov rdi,\", a);")
{
  statement_t* s = parse_statement(&parser);

  ct_assert_not_null(s, "asm statement should not be NULL");
  ct_assert_eq(s->type, STATEMENT_ASM, "statement type should be STATEMENT_ASM");
  ct_assert_eq((int)s->asm_stmt.instr_count, 1, "asm statement should have 1 instruction");
  ct_assert_eq((int)s->asm_stmt.arg_count, 1, "asm statement should have 1 arg");
  ct_assert_eq(s->asm_stmt.instr[0], "mov rdi,", "instruction string should match");
  ct_assert_not_null(s->asm_stmt.args[0], "arg expression should not be NULL");
  ct_assert_eq(s->asm_stmt.args[0]->type, EXPRESSION_VAR, "arg expression should be a variable");
  ct_assert_eq(s->asm_stmt.args[0]->var.ident.ident_name, "a", "arg variable name should be 'a'");

  free_statement(s);
  da_free(&parser);
}

ct_test(ast, asm_only_args, "asm(a, 42);")
{
  statement_t* s = parse_statement(&parser);

  ct_assert_not_null(s, "asm statement should not be NULL");
  ct_assert_eq(s->type, STATEMENT_ASM, "statement type should be STATEMENT_ASM");
  ct_assert_eq((int)s->asm_stmt.instr_count, 0, "asm statement should have 0 instructions");
  ct_assert_eq((int)s->asm_stmt.arg_count, 2, "asm statement should have 2 args");
  ct_assert_eq(s->asm_stmt.args[0]->type, EXPRESSION_VAR, "first arg should be a variable");
  ct_assert_eq(s->asm_stmt.args[0]->var.ident.ident_name, "a", "first arg name should be 'a'");
  ct_assert_eq(s->asm_stmt.args[1]->type, EXPRESSION_INT_LIT, "second arg should be INT literal");
  ct_assert_eq(s->asm_stmt.args[1]->int_lit.value, 42, "second arg value should be 42");

  free_statement(s);
  da_free(&parser);
}

// === PRECISE INTEGER TYPES ===

ct_test(ast, typed_u8_var, "u8 i = 5;")
{
  declaration_t* decl = parse_declaration(&parser);

  ct_assert_eq(decl->var_decl.ident.ident_name, "i", "Variable name should be 'i'");
  ct_assert_eq(decl->var_decl.ident.type.kind, TYPE_U8, "Variable type should be TYPE_U8");
  ct_assert_eq((int)decl->var_decl.ident.type.size, 1, "Variable size should be 1 byte");
  ct_assert_not_null(decl->var_decl.init, "Variable init should not be NULL");
  ct_assert_eq(decl->var_decl.init->int_lit.value, 5, "Init value should be 5");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, typed_u16_var, "u16 i = 300;")
{
  declaration_t* decl = parse_declaration(&parser);

  ct_assert_eq(decl->var_decl.ident.ident_name, "i", "Variable name should be 'i'");
  ct_assert_eq(decl->var_decl.ident.type.kind, TYPE_U16, "Variable type should be TYPE_U16");
  ct_assert_eq((int)decl->var_decl.ident.type.size, 2, "Variable size should be 2 bytes");
  ct_assert_eq(decl->var_decl.init->int_lit.value, 300, "Init value should be 300");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, typed_u32_var, "u32 i = 5;")
{
  declaration_t* decl = parse_declaration(&parser);

  ct_assert_eq(decl->var_decl.ident.ident_name, "i", "Variable name should be 'i'");
  ct_assert_eq(decl->var_decl.ident.type.kind, TYPE_U32, "Variable type should be TYPE_U32");
  ct_assert_eq((int)decl->var_decl.ident.type.size, 4, "Variable size should be 4 bytes");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, typed_u64_var, "u64 i = 5;")
{
  declaration_t* decl = parse_declaration(&parser);

  ct_assert_eq(decl->var_decl.ident.ident_name, "i", "Variable name should be 'i'");
  ct_assert_eq(decl->var_decl.ident.type.kind, TYPE_U64, "Variable type should be TYPE_U64");
  ct_assert_eq((int)decl->var_decl.ident.type.size, 8, "Variable size should be 8 bytes");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, fn_ret_type_u8, "fn f(): u8 {}")
{
  declaration_t* decl = parse_declaration(&parser);

  ct_assert_not_null(decl, "Declaration should not be NULL");
  ct_assert_eq(decl->func.return_type.kind, TYPE_U8, "Return type kind should be TYPE_U8");
  ct_assert_eq((int)decl->func.return_type.size, 1, "Return type size should be 1 byte");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, fn_params_u8_u16, "fn f(u8 a, u16 b) {}")
{
  declaration_t* decl = parse_declaration(&parser);

  ct_assert_not_null(decl, "Declaration should not be NULL");
  ct_assert_eq((int)decl->func.params.count, 2, "Function should have 2 parameters");

  typed_identifier_t p1 = decl->func.params.items[0];
  typed_identifier_t p2 = decl->func.params.items[1];

  ct_assert_eq(p1.ident_name, "a", "First param name should be 'a'");
  ct_assert_eq(p1.type.kind, TYPE_U8, "First param type should be TYPE_U8");
  ct_assert_eq((int)p1.type.size, 1, "First param size should be 1 byte");

  ct_assert_eq(p2.ident_name, "b", "Second param name should be 'b'");
  ct_assert_eq(p2.type.kind, TYPE_U16, "Second param type should be TYPE_U16");
  ct_assert_eq((int)p2.type.size, 2, "Second param size should be 2 bytes");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, struct_var_designated_init_three_fields, "struct v3 { int a; int b; int c; } v3 s = { .a = 1, .b = 2, .c = 3 };")
{
  declaration_t* struct_decl = parse_declaration(&parser);
  ct_assert_not_null(struct_decl, "struct decl should not be NULL");

  declaration_t* decl = parse_declaration(&parser);
  ct_assert_not_null(decl, "var decl should not be NULL");
  ct_assert_eq(decl->type, DECLARATION_VAR, "declaration type should be VAR");
  ct_assert_not_null(decl->var_decl.init, "init expression should not be NULL");
  ct_assert_eq(decl->var_decl.init->type, EXPRESSION_COMPOSITE_LITERAL, "init should be COMPOSITE_LITERAL");
  ct_assert_eq((int)decl->var_decl.init->composite_literal.is_initializer, 1, "is_initializer should be true");
  ct_assert_eq((int)decl->var_decl.init->composite_literal.count, 3, "should have 3 field assignments");
  ct_assert_eq(decl->var_decl.init->composite_literal.values[2]->assign.lhs->var.ident.ident_name, "c", "third field name should be 'c'");
  ct_assert_eq(decl->var_decl.init->composite_literal.values[2]->assign.rhs->int_lit.value, 3, "third field value should be 3");

  free_declaration(struct_decl);
  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, const_in_func_parameter, "fn main(int! a) { }") {
  declaration_t* decl = parse_declaration(&parser);
  ct_assert_not_null(decl, "function decl should not be NULL");
  ct_assert_eq(decl->func.params.count, 1, "function should have one parameter");
  ct_assert_eq(decl->func.params.items[0].is_constant, true, "first function parameter should be marked as constant");

  free_declaration(decl);
  da_free(&parser);
}

ct_test(ast, char_var_decl, "char a = 'a';") {
  declaration_t* decl = parse_declaration(&parser);
  ct_assert_not_null(decl, "char var declaration should not be null");
  ct_assert_eq(decl->var_decl.ident.type.name, "char", "var type should be char");
  ct_assert_eq(decl->var_decl.ident.ident_name, "a", "Variable name should be 'a'");
  ct_assert_eq(decl->var_decl.ident.type.kind, TYPE_CHAR, "Variable type should be TYPE_CHAR");
  ct_assert_not_null(decl->var_decl.init, "Variable init expression should not be NULL");
  ct_assert_eq(decl->var_decl.init->type, EXPRESSION_CHAR_LIT, "Init expression should be char literal");
  ct_assert_eq(decl->var_decl.init->char_lit.value, 'a', "Init char value should be 'a'");


  free_declaration(decl);
  da_free(&parser);
}
