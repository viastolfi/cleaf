#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define LEXER_LIB_IMPLEMENTATION
#include "../src/frontend/lexer.h"
#define DA_LIB_IMPLEMENTATION
#include "../src/thirdparty/da.h"

#include "../src/frontend/ast_definition.h"
#include "../src/frontend/ast.h"

// ANSI colors for nicer UX
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_CYAN    "\033[36m"

// Enable to print lexer tokens
#define DEBUG_TOKENS 0

// Helper for better assertions
#define ASSERT_MSG(cond, msg)                                         \
  do {                                                                \
    if (!(cond)) {                                                    \
      printf(COLOR_RED "❌ Test failed:" COLOR_RESET " %s\n", msg);    \
      exit(1);                                                        \
    }                                                                 \
  } while (0)

parser_t get_token(char* source_code)
{
  parser_t parser = {0};
  lexer_t lex;


  char* storage = malloc(255);
  lexer_init_lexer(&lex, source_code, source_code + strlen(source_code), (char*) storage, 255);

  while (lexer_get_token(&lex)) {
    if (lex.token == LEXER_token_parse_error) {
      printf(COLOR_RED "\n<<< PARSE ERROR >>>\n" COLOR_RESET);
      break;
    }

#if DEBUG_TOKENS 
    printf(COLOR_CYAN "[token] " COLOR_RESET);
    lexer_print_token(&lex);
    printf("\n");
#endif

    token_t t = lexer_copy_token(&lex);
    da_append(&parser, t);
  }

  free(storage);
  return parser;
}

void test_fn_def(const char* name, char* source_code)
{
  printf(COLOR_YELLOW "→ Running test:" COLOR_RESET " %s\n", name);
  parser_t parser = get_token(source_code);

  declaration_t* decl = parse_declaration(&parser);

  ASSERT_MSG(decl != NULL, "Declaration should not be NULL");
  ASSERT_MSG(decl->type == (declaration_kind) DECLARATION_FUNC,
             "Declaration type should be DECLARATION_FUNC");

  ASSERT_MSG(decl->func.name != NULL, "Function name should not be NULL");
  ASSERT_MSG(strcmp(decl->func.name, "main") == 0,
              "Function name should be the same as in the source code");

  printf(COLOR_GREEN "✅ Test passed:" COLOR_RESET " %s\n\n", name);

  free_declaration(decl);
  da_free(&parser);
}

void test_fn_ret_type(const char* name, char* source_code)
{
  printf(COLOR_YELLOW "→ Running test:" COLOR_RESET " %s\n", name);
  parser_t parser = get_token(source_code);
  
  declaration_t* decl = parse_declaration(&parser);

  ASSERT_MSG(decl != NULL, "Declaration should not be NULL");
  ASSERT_MSG(decl->func.return_type.name != NULL,
              "Function return type should not be NULL");
  ASSERT_MSG(strcmp(decl->func.return_type.name, "int") == 0,
              "Function return type should be the same as in the source code");
  ASSERT_MSG(decl->func.return_type.kind == TYPE_INT,
              "Function return type should be TYPE_INT");

  printf(COLOR_GREEN "✅ Test passed:" COLOR_RESET " %s\n\n", name);

  free_declaration(decl);
  da_free(&parser);
}

void test_fn_params(const char* name, char* source_code) 
{
  printf(COLOR_YELLOW "→ Running test:" COLOR_RESET " %s\n", name);
  parser_t parser = get_token(source_code);
  
  declaration_t* decl = parse_declaration(&parser);

  ASSERT_MSG(decl != NULL, "Declaration should not be NULL");

  ASSERT_MSG(decl->func.params.count == 2,
              "Function should have 2 parameters");
  typed_identifier_t p1 = decl->func.params.items[0];
  ASSERT_MSG(p1.name != NULL,
              "Function first param name should not be NULL");
  ASSERT_MSG(strcmp(p1.name, "a") == 0,
              "Function first param name should be the same as in the source code");
  ASSERT_MSG(p1.type.kind == TYPE_INT,
              "Function first param type should be TYPE_INT");
  ASSERT_MSG(p1.type.name != NULL,
              "Function first param type name should not be NULL");
  ASSERT_MSG(strcmp(p1.type.name, "int") == 0,
              "Function first param type should be \"int\"");

  typed_identifier_t p2 = decl->func.params.items[1];
  ASSERT_MSG(p2.name != NULL,
              "Function second param name should not be NULL");
  ASSERT_MSG(strcmp(p2.name, "b") == 0,
              "Function second param name should be the same as in the source code");
  ASSERT_MSG(p2.type.kind == TYPE_STRING,
              "Function second param type should be TYPE_STRING");
  ASSERT_MSG(p2.type.name != NULL,
              "Function second param type name should not be NULL");
  ASSERT_MSG(strcmp(p2.type.name, "string") == 0,
              "Function second param type should be \"int\"");

  printf(COLOR_GREEN "✅ Test passed:" COLOR_RESET " %s\n\n", name);

  free_declaration(decl);
  da_free(&parser);
}

void test_typed_int_var_decl(const char* name, char* source_code)
{
  printf(COLOR_YELLOW "→ Running test:" COLOR_RESET " %s\n", name);
  parser_t parser = get_token(source_code);
  
  declaration_t* decl = parse_declaration(&parser);

  ASSERT_MSG(decl != NULL, "Declaration should not be NULL");
  ASSERT_MSG(decl->var_decl.ident.name != NULL,
                 "Variable name should not be NULL");
  ASSERT_MSG(strcmp(decl->var_decl.ident.name, "i") == 0,
                  "Variable name should be the same as in the source code");
  ASSERT_MSG(decl->var_decl.ident.type.name != NULL,
                  "Variable type should not be NULL");
  ASSERT_MSG(strcmp(decl->var_decl.ident.type.name, "int") == 0,
                  "Variable type should be the same as in the source code");
  ASSERT_MSG(decl->var_decl.ident.type.kind == TYPE_INT,
                  "Variable type should be TYPE_INT");
  ASSERT_MSG(decl->var_decl.init != NULL,
                  "Variable init value should not be NULL");
  ASSERT_MSG(decl->var_decl.init->type == (expression_kind) EXPRESSION_INT_LIT,
                  "Expression type should be INT_LITERAL");
  ASSERT_MSG(decl->var_decl.init->int_lit.value == 3,
                  "Expression value should be the same as in the source code");
  printf(COLOR_GREEN "✅ Test passed:" COLOR_RESET " %s\n\n", name);

  free_declaration(decl);
  da_free(&parser);
}

void test_typed_string_var_decl(const char* name, char* source_code)
{
  printf(COLOR_YELLOW "→ Running test:" COLOR_RESET " %s\n", name);
  parser_t parser = get_token(source_code);
  
  declaration_t* decl = parse_declaration(&parser);

  ASSERT_MSG(decl != NULL, "Declaration should not be NULL");
  ASSERT_MSG(decl->var_decl.ident.name != NULL,
                 "Variable name should not be NULL");
  ASSERT_MSG(strcmp(decl->var_decl.ident.name, "i") == 0,
                  "Variable name should be the same as in the source code");
  ASSERT_MSG(decl->var_decl.ident.type.name != NULL,
                  "Variable type should not be NULL");
  ASSERT_MSG(strcmp(decl->var_decl.ident.type.name, "string") == 0,
                  "Variable type should be the same as in the source code");
  ASSERT_MSG(decl->var_decl.ident.type.kind == TYPE_STRING,
                  "Variable should be typed TYPE_STRING");
  ASSERT_MSG(decl->var_decl.init != NULL,
                  "Variable init value should not be NULL");
  ASSERT_MSG(decl->var_decl.init->type == (expression_kind) EXPRESSION_STRING_LIT,
                  "Expression type should be STRING_LITERAL");
  ASSERT_MSG(decl->var_decl.init->string_lit.value != NULL,
                  "Expression string value should not be NULL");
  ASSERT_MSG(strcmp(decl->var_decl.init->string_lit.value, "test") == 0,
                  "Expression value should be the same as in the source code");
  printf(COLOR_GREEN "✅ Test passed:" COLOR_RESET " %s\n\n", name);

  free_declaration(decl);
  da_free(&parser);
}

void test_int_lit_return(const char* name, char* source_code) 
{
  printf(COLOR_YELLOW "→ Running test:" COLOR_RESET " %s\n", name);
  parser_t parser = get_token(source_code);

  statement_t* s = parse_statement(&parser);

  ASSERT_MSG(s != NULL, "Statement should not be NULL");
  ASSERT_MSG(s->type == (statement_kind) STATEMENT_RETURN,
              "Statement kind should be STATEMENT_RETURN");
  ASSERT_MSG(s->ret.value != NULL,
              "Return type should not be NULL");
  ASSERT_MSG(s->ret.value->type == EXPRESSION_INT_LIT,
              "Return type should be typed INT_LIT");
  ASSERT_MSG(s->ret.value->int_lit.value == 1,
              "Return value should be the same as in the source code");

  printf(COLOR_GREEN "✅ Test passed:" COLOR_RESET " %s\n\n", name);

  free_statement(s);
  da_free(&parser);
}

void test_string_lit_return(const char* name, char* source_code)
{
  printf(COLOR_YELLOW "→ Running test:" COLOR_RESET " %s\n", name);
  parser_t parser = get_token(source_code);

  statement_t* s = parse_statement(&parser);

  ASSERT_MSG(s != NULL, "Statement should not be NULL");
  ASSERT_MSG(s->type == (statement_kind) STATEMENT_RETURN,
              "Statement kind should be STATEMENT_RETURN");
  ASSERT_MSG(s->ret.value != NULL,
              "Return type should not be NULL");
  ASSERT_MSG(s->ret.value->type == EXPRESSION_STRING_LIT,
              "Return type should be typed STRING_LIT");
  ASSERT_MSG(s->ret.value->string_lit.value != NULL,
              "Return string value should not be NULL");
  ASSERT_MSG(strcmp(s->ret.value->string_lit.value, "test") == 0,
              "Return string value should be the same as in the source code");

  printf(COLOR_GREEN "✅ Test passed:" COLOR_RESET " %s\n\n", name);
  
  free_statement(s);
  da_free(&parser);
}

void test_var_return(const char* name, char* source_code)
{
  printf(COLOR_YELLOW "→ Running test:" COLOR_RESET " %s\n", name);
  parser_t parser = get_token(source_code);

  statement_t* s = parse_statement(&parser);

  ASSERT_MSG(s != NULL, "Statement should not be NULL");
  ASSERT_MSG(s->type == (statement_kind) STATEMENT_RETURN,
              "Statement kind should be STATEMENT_RETURN");
  ASSERT_MSG(s->ret.value != NULL,
              "Return type should not be NULL");
  ASSERT_MSG(s->ret.value->type == EXPRESSION_VAR,
              "Return type should be typed VAR");
  ASSERT_MSG(s->ret.value->var.name != NULL,
              "Return var name should not be NULL");
  ASSERT_MSG(strcmp(s->ret.value->var.name, "a") == 0,
              "Return string value should be the same as in the source code");

  printf(COLOR_GREEN "✅ Test passed:" COLOR_RESET " %s\n\n", name);
  
  free_statement(s);
  da_free(&parser);
}

void test_expr_assign(const char* name, char* source_code)
{
  printf(COLOR_YELLOW "→ Running test:" COLOR_RESET " %s\n", name);
  parser_t parser = get_token(source_code);

  statement_t* s = parse_statement(&parser);

  ASSERT_MSG(s != NULL, "Statement should not be NULL");
  ASSERT_MSG(s->type == STATEMENT_EXPR,
              "Statement should be of type EXPR");
  ASSERT_MSG(s->expr_stmt.expr != NULL,
              "Statement expression should not be NULL");
  expression_t* e = s->expr_stmt.expr;
  ASSERT_MSG(e->type == EXPRESSION_ASSIGN,
              "Expression should be of type ASSIGN");
  ASSERT_MSG(e->assign.lhs != NULL,
              "Assign left expression should not be NULL");
  expression_t* lhs = e->assign.lhs;
  ASSERT_MSG(lhs->type == EXPRESSION_VAR,
              "Assign left expression should be of type VAR"); 
  ASSERT_MSG(lhs->var.name != NULL,
              "Assign left expression var name should not be NULL");
  ASSERT_MSG(strcmp(lhs->var.name, "i") == 0,
              "Assign left expression var name should be the same as in the source code");
  ASSERT_MSG(e->assign.rhs != NULL,
              "Assign right expression should not be NULL");
  expression_t* rhs = e->assign.rhs;
  ASSERT_MSG(rhs->type == EXPRESSION_INT_LIT,
              "Assign right expression type should be the same as in the source code");
  ASSERT_MSG(rhs->int_lit.value == 4,
              "Assign right expression value should be the same as in the source code");

  printf(COLOR_GREEN "✅ Test passed:" COLOR_RESET " %s\n\n", name);
  
  free_statement(s);
  da_free(&parser);
}

void test_expr_binary(const char* name, char* source_code)
{
  printf(COLOR_YELLOW "→ Running test:" COLOR_RESET " %s\n", name);
  parser_t parser = get_token(source_code);

  statement_t* s = parse_statement(&parser);

  ASSERT_MSG(s != NULL, "Statement should not be NULL");
  ASSERT_MSG(s->type == STATEMENT_EXPR,
              "Statement should be of type EXPR");
  ASSERT_MSG(s->expr_stmt.expr != NULL,
              "Statement expression should not be NULL");
  expression_t* e = s->expr_stmt.expr;
  ASSERT_MSG(e->type == EXPRESSION_BINARY,
              "Expression type should be BINARY");
  ASSERT_MSG(e->binary.op == '+',
              "Binary op should be the same as in the source code");
  ASSERT_MSG(e->binary.left != NULL,
              "Binary op left expr should not be NULL");
  expression_t* l = e->binary.left;
  ASSERT_MSG(l->type == EXPRESSION_VAR,
              "Binary left expr type should be the same as in the source code");
  ASSERT_MSG(l->var.name != NULL,
              "Left expression var name should not be NULL");
  ASSERT_MSG(strcmp(l->var.name, "i") == 0,
              "Left expression var name should be the same as in the source code");
  ASSERT_MSG(e->binary.right != NULL,
              "Binary op right expr should not be NULL");
  expression_t* r = e->binary.right;
  ASSERT_MSG(r->type == EXPRESSION_INT_LIT,
              "Right expression type should be the same as in the source code");
  ASSERT_MSG(r->int_lit.value == 4,
              "Right expression int_lit value should be the same as in the source code");

  printf(COLOR_GREEN "✅ Test passed:" COLOR_RESET " %s\n\n", name);
  
  free_statement(s);
  da_free(&parser);
}

void test_untype_string_var_decl(const char* name, char* source_code) {
  printf(COLOR_YELLOW "→ Running test:" COLOR_RESET " %s\n", name);
  parser_t parser = get_token(source_code);
  
  declaration_t* decl = parse_declaration(&parser);

  ASSERT_MSG(decl != NULL, "Declaration should not be NULL");
  ASSERT_MSG(decl->type == DECLARATION_VAR, 
              "Declaration should be of type VAR");
  ASSERT_MSG(decl->var_decl.ident.name != NULL,
              "Variable identifier name should not be NULL");
  ASSERT_MSG(strcmp(decl->var_decl.ident.name, "i") == 0,
              "Variable identifier name should be the same as in the source code");
  ASSERT_MSG(decl->var_decl.ident.type.kind == TYPE_UNTYPE,
              "Variable indetifier type sould be of type UNTYPE");
  ASSERT_MSG(decl->var_decl.ident.type.name != NULL,
              "Variable identifier type name should not be NULL");
  ASSERT_MSG(strcmp(decl->var_decl.ident.type.name, "var") == 0,
              "Variable identifier type should be the same as in the source code");
  ASSERT_MSG(decl->var_decl.init != NULL,
              "Variable init expression should not be NULL");
  ASSERT_MSG(decl->var_decl.init->type == EXPRESSION_INT_LIT,
              "Variable init expression should be of type INT_LIT");
  ASSERT_MSG(decl->var_decl.init->int_lit.value == 3,
              "Variable init expression int_lit value should be the same as in the source code");

  printf(COLOR_GREEN "✅ Test passed:" COLOR_RESET " %s\n\n", name);
  
  free_declaration(decl);
  da_free(&parser);
}

void test_uninitialize_var_decl(const char* name, char* source_code) 
{
  printf(COLOR_YELLOW "→ Running test:" COLOR_RESET " %s\n", name);
  parser_t parser = get_token(source_code);
  
  declaration_t* decl = parse_declaration(&parser);

  ASSERT_MSG(decl != NULL, "Declaration should not be NULL");
  ASSERT_MSG(decl->type == DECLARATION_VAR, 
              "Declaration should be of type VAR");
  ASSERT_MSG(decl->var_decl.ident.name != NULL,
              "Variable identifier name should not be NULL");
  ASSERT_MSG(strcmp(decl->var_decl.ident.name, "i") == 0,
              "Variable identifier name should be the same as in the source code");
  ASSERT_MSG(decl->var_decl.ident.type.kind == TYPE_UNTYPE,
              "Variable indetifier type sould be of type UNTYPE");
  ASSERT_MSG(decl->var_decl.ident.type.name != NULL,
              "Variable identifier type name should not be NULL");
  ASSERT_MSG(strcmp(decl->var_decl.ident.type.name, "var") == 0,
              "Variable identifier type should be the same as in the source code");
  ASSERT_MSG(decl->var_decl.init == NULL,
              "Variable init expression should be NULL");

  printf(COLOR_GREEN "✅ Test passed:" COLOR_RESET " %s\n\n", name);
  
  free_declaration(decl);
  da_free(&parser);
}

void test_function_call(const char* name, char* source_code)
{
  printf(COLOR_YELLOW "→ Running test:" COLOR_RESET " %s\n", name);
  parser_t parser = get_token(source_code);

  statement_t* s = parse_statement(&parser);

  ASSERT_MSG(s != NULL, "Statement should not be NULL");
  ASSERT_MSG(s->type == STATEMENT_EXPR,
      "Statement should be of type EXPR");
  ASSERT_MSG(s->expr_stmt.expr != NULL,
      "Statement expression should not be NULL");
  expression_t* e = s->expr_stmt.expr;
  ASSERT_MSG(e->type == EXPRESSION_CALL,
      "Expression type should be CALL");
  ASSERT_MSG(e->call.callee != NULL,
      "Expression function call name should not be NULL");
  ASSERT_MSG(strcmp(e->call.callee, "test") == 0,
      "Expression function call name should be the same as in the source code");
  ASSERT_MSG(e->call.arg_count == 3,
      "Expression function call arg count should be the same as in the source code");
  ASSERT_MSG(e->call.args != NULL,
      "Expression function call args should not be NULL");
  ASSERT_MSG(e->call.args[0] != NULL,
      "Func call first arg should not be NULL");
  expression_t* a1 = e->call.args[0];
  ASSERT_MSG(a1->type == EXPRESSION_VAR,
      "First arg should be of type VAR");
  ASSERT_MSG(a1->var.name != NULL,
      "First arg var name should not be NULL");
  ASSERT_MSG(strcmp(a1->var.name, "a") == 0,
      "First arg var name should be the same as in the source code");
  ASSERT_MSG(e->call.args[1] != NULL,
      "Function call second arg should not be NULL");
  expression_t* a2 = e->call.args[1];
  ASSERT_MSG(a2->type == EXPRESSION_INT_LIT,
      "Second arg should be of type INT_LIT");
  ASSERT_MSG(a2->int_lit.value == 5,
      "Second arg int value should be the same as in the source code");
  ASSERT_MSG(e->call.args[2] != NULL,
      "Function call third arg should not be NULL");
  expression_t* a3 = e->call.args[2];
  ASSERT_MSG(a3->type == EXPRESSION_STRING_LIT,
      "Third argument should be of type STRING_LIT");
  ASSERT_MSG(a3->string_lit.value != NULL,
      "Third argument string value should not be NULL");
  ASSERT_MSG(strcmp(a3->string_lit.value, "test") == 0,
      "Third argument string value should be the same as in the source code");

  printf(COLOR_GREEN "✅ Test passed:" COLOR_RESET " %s\n\n", name);
  
  free_statement(s);
  da_free(&parser);
}

int main(void)
{
  printf(COLOR_CYAN "\n=== Running AST Tests ===\n\n" COLOR_RESET);

  test_fn_def("Simple function definition", "fn main() {}");
  test_fn_ret_type("Function return type", "fn main(): int {}");
  test_fn_params("Function parameters", "fn main(int a, string b) {}");
  test_typed_int_var_decl("Typed int var declaration", "int i = 3;");
  test_typed_string_var_decl("Typed string var declaration", "string i = \"test\";");
  test_untype_string_var_decl("Untyped var declaraion", "var i = 3;");
  test_uninitialize_var_decl("Uninitialize var declaration", "var i;");
  test_int_lit_return("Integer literal return statement", "return 1;");
  test_string_lit_return("String literal return statement", "return \"test\";");
  test_var_return("Variable return statement", "return a;");
  test_expr_assign("Variable assignment", "i = 4;");
  test_expr_binary("Binary operation", "i + 4;");
  test_function_call("Function call", "test(a, 5, \"test\");");

  printf(COLOR_GREEN "🎉 All tests passed successfully!\n" COLOR_RESET);
  return 0;
}

