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

  lexer_init_lexer(&lex, source_code, source_code + strlen(source_code), (char*) malloc(255), 255);

  while (lexer_get_token(&lex)) {
    if (lex.token == LEXER_token_parse_error) {
      printf(COLOR_RED "\n<<< PARSE ERROR >>>\n" COLOR_RESET);
      break;
    }

#if DEBUG_TOKENS
    printf(COLOR_CYAN "[token] %-15s (len=%zu)\n" COLOR_RESET, lex.token_name, lex.token_len);
#endif

    token_t t = lexer_copy_token(&lex);
    da_append(&parser, t);
  }

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
}

void test_typed_var_decl(const char* name, char* source_code)
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
}

int main(void)
{
  printf(COLOR_CYAN "\n=== Running AST Tests ===\n\n" COLOR_RESET);

  test_fn_def("Simple function definition", "fn main() {}");
  test_fn_ret_type("Function return type", "fn main(): int {}");
  test_typed_var_decl("Typed var declaration", "int i = 3;");

  printf(COLOR_GREEN "🎉 All tests passed successfully!\n" COLOR_RESET);
  return 0;
}

