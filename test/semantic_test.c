#define CTEST_BEFORE_EACH
#define CTEST_LIB_IMPLEMENTATION
#include "ctest.h"

#define LEXER_LIB_IMPLEMENTATION
#include "../src/frontend/lexer.h"
#define DA_LIB_IMPLEMENTATION
#include "../src/thirdparty/da.h"

#include "../src/frontend/ast_definition.h"
#include "../src/frontend/ast.h"
#include "../src/frontend/semantic.h"
#include "../src/frontend/error.h"

before_each(semantic_analyzer_t, analyzer, char* source_code) 
{
  parser_t p = {0};
  lexer_t lex;
  semantic_analyzer_t a = {0};
  error_context_t error_ctx;
  error_init(&error_ctx, "semantic_test.c", source_code, strlen(source_code));

  char* storage = malloc(255);
  lexer_init_lexer(&lex, 
                   source_code, 
                   source_code + strlen(source_code), 
                   (char*) storage,
                   255);

  while (lexer_get_token(&lex)) {
    if (lex.token == LEXER_token_parse_error)
     break; 

    token_t t = lexer_copy_token(&lex);
    da_append(&p, t);
  }

  free(storage);

  declaration_array program = {0};
  while ((size_t) p.pos < p.count) {
    declaration_t* decl = parse_declaration(&p);
    da_append(&program, decl); 
  }

  a.error_ctx = &error_ctx;
  a.ast = &program;
  a.error_count = 0;

  semantic_analyze(&a);

  analyzer = a; 
}

ct_test(semantic_analyze, fn_def_simple, "fn main() {}") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for simple function definition");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, fn_def_with_params, "fn main(int a, string b) {}") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for function with params");
  
  function_symbol_t* fs = (function_symbol_t*) hashmap_get(analyzer.function_symbols, "main");
  ct_assert_not_null(fs, "Function symbol should be in symbol table");
  ct_assert_eq((int)fs->params_count, 2, "Function should have 2 parameters");
  ct_assert_eq(fs->params_name[0], "a", "First param name should be 'a'");
  ct_assert_eq(fs->params_type[0], TYPE_INT, "First param type should be TYPE_INT");
  ct_assert_eq(fs->params_name[1], "b", "Second param name should be 'b'");
  ct_assert_eq(fs->params_type[1], TYPE_STRING, "Second param type should be TYPE_STRING");
  
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, fn_def_with_return_type, "fn main(): int {}") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for function with return type");
  
  function_symbol_t* fs = (function_symbol_t*) hashmap_get(analyzer.function_symbols, "main");
  ct_assert_not_null(fs, "Function symbol should be in symbol table");
  ct_assert_eq(fs->return_type, TYPE_INT, "Return type should be TYPE_INT");
  
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, fn_duplicate_definition, "fn main() {} fn main() {}") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for duplicate function definition");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, fn_duplicate_params, "fn main(int a, int a) {}") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for duplicate parameter names");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, var_decl_typed, "fn main() { int a = 5; }") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for typed variable declaration");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, var_decl_untyped, "fn main() { var a = 5; }") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for untyped variable declaration");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, var_redefinition, "fn main() { int a = 5; int a = 10; }") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for variable redefinition");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, var_undefined_use, "fn main() { int b = a + 1; }") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for using undefined variable");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, return_correct_type, "fn main(): int { return 5; }") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for correct return type");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, return_wrong_type_int_string, "fn main(): string { return 5; }") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for wrong return type (int instead of string)");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, return_wrong_type_string_int, "fn main(): int { return \"hello\"; }") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for wrong return type (string instead of int)");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, return_string_correct, "fn main(): string { return \"hello\"; }") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for correct string return");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, scope_if_statement, "fn main() { if (1 == 1) { int a = 5; } }") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for if statement scope");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, scope_if_else, "fn main() { if (1 == 1) { int a = 5; } else { int b = 10; } }") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for if-else statement scopes");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, scope_while_statement, "fn main() { while (1 == 1) { int a = 5; } }") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for while statement scope");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, scope_for_statement, "fn main() { for (var i = 0; i < 10; ++i) { int a = 5; } }") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for for statement scope");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, scope_for_var_in_init, "fn main() { for (var i = 0; i < 10; ++i) { i = 5; } }") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for using for init var in body");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, param_in_function_body, "fn main(int a): int { return a; }") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for using parameter in function body");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, param_redefinition_in_body, "fn main(int a) { int a = 5; }") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for redefining parameter in function body");
  semantic_free_function_definition(&analyzer);
}

ct_test(semantic_analyze, multiple_functions, "fn foo() {} fn bar() {}") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for multiple function definitions");
  semantic_free_function_definition(&analyzer);
}

