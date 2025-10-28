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
  ASSERT_MSG(decl->func.return_type != NULL,
              "Function return type should not be NULL");
  ASSERT_MSG(strcmp(decl->func.return_type, "int") == 0,
              "Function return type should be the same as in the source code");

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
  ASSERT_MSG(decl->var.name != NULL,
                 "Variable name should not be NULL");
  ASSERT_MSG(strcmp(decl->var.name, "i") == 0,
                  "Variable name should be the same as in the source code");
  ASSERT_MSG(decl->var.type != NULL,
                  "Variable type should not be NULL");
  ASSERT_MSG(strcmp(decl->var.type, "int") == 0,
                  "Variable type should be the same as in the source code");
  ASSERT_MSG(decl->var.init != NULL,
                  "Variable init value should not be NULL");
  ASSERT_MSG(decl->var.init->type == (expression_kind) EXPRESSION_LIT,
                  "Expression type should be LITERAL");
  ASSERT_MSG(decl->var.init->int_value == 3,
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
  ASSERT_MSG(decl->var.name != NULL,
                 "Variable name should not be NULL");
  ASSERT_MSG(strcmp(decl->var.name, "i") == 0,
                  "Variable name should be the same as in the source code");
  ASSERT_MSG(decl->var.type != NULL,
                  "Variable type should not be NULL");
  ASSERT_MSG(strcmp(decl->var.type, "string") == 0,
                  "Variable type should be the same as in the source code");
  ASSERT_MSG(decl->var.init != NULL,
                  "Variable init value should not be NULL");
  ASSERT_MSG(decl->var.init->type == (expression_kind) EXPRESSION_LIT,
                  "Expression type should be LITERAL");
  ASSERT_MSG(decl->var.init->string_value != NULL,
                  "Expression string value should not be NULL");
  ASSERT_MSG(strcmp(decl->var.init->string_value, "test") == 0,
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
  ASSERT_MSG(s->ret.type != NULL,
              "Return type should not be NULL");
  ASSERT_MSG(strcmp(s->ret.type, "int") == 0,
              "Return type should be \"int\"");
  ASSERT_MSG(s->ret.int_value == 1,
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
  ASSERT_MSG(s->ret.type != NULL,
              "Return type should not be NULL");
  ASSERT_MSG(strcmp(s->ret.type, "string") == 0,
              "Return type should be string");
  ASSERT_MSG(s->ret.string_value != NULL,
              "Return string value should not be NULL");
  ASSERT_MSG(strcmp(s->ret.string_value, "test") == 0,
              "Return string value should be the same as in the source cod");

  printf(COLOR_GREEN "✅ Test passed:" COLOR_RESET " %s\n\n", name);
  
  free_statement(s);
  da_free(&parser);
}

int main(void)
{
  printf(COLOR_CYAN "\n=== Running AST Tests ===\n\n" COLOR_RESET);

  test_fn_def("Simple function definition", "fn main() {}");
  test_fn_ret_type("Function return type", "fn main(): int {}");
  test_typed_int_var_decl("Typed int var declaration", "int i = 3;");
  test_typed_string_var_decl("Typed string var declaration", "string i = \"test\";");
  test_int_lit_return("Integer literal return statement", "return 1;");
  test_string_lit_return("String literal return statement", "return \"test\";");

  printf(COLOR_GREEN "🎉 All tests passed successfully!\n" COLOR_RESET);
  return 0;
}

