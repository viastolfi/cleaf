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

before_each(semantic_analyzer_t, analyzer, char* file_path)
{
  FILE *f = fopen(file_path, "rb");
  if (f == NULL) {
    fprintf(stderr, "error on test file test/semantic_file/%s\n", file_path);
    abort();
  }

  char* text = (char*) malloc(1 << 20);
  int len = f ? (int) fread(text, 1, 1<<20, f) : -1;

  if (len < 0) {
    fprintf(stderr, "error while reading %s\n", file_path);
    free(text);
    fclose(f);
    abort();
  }
  fclose(f);

  parser_t p = {0};
  lexer_t lex;
  semantic_analyzer_t a = {0};

  error_context_t* error_ctx = calloc(1, sizeof(error_context_t));
  if (!error_ctx) abort();
  error_init(error_ctx, file_path, text, len);

  declaration_array* program = calloc(1, sizeof(declaration_array));
  if (!program) abort();

  char* storage = malloc(255);
  if (!storage) abort();
  lexer_init_lexer(&lex,
      text,
      text + len,
      storage,
      255);

  while (lexer_get_token(&lex)) {
    if (lex.token == LEXER_token_parse_error)
      break;

    token_t t = lexer_copy_token(&lex);
    da_append(&p, t);
  }

  free(storage);

  while ((size_t)p.pos < p.count) {
    declaration_t* decl = parse_declaration(&p);
    da_append(program, decl);
  }

  free(text);

  a.error_ctx    = error_ctx;
  a.ast          = program;
  a.error_count  = 0;

  semantic_analyze(&a);

  for (size_t i = 0; i < p.count; i++) {
    if (p.items[i].string_value) {
      free(p.items[i].string_value);
    }
  }
  da_free(&p);

  analyzer = a;
}

void free_analyzer(semantic_analyzer_t* analyzer)
{
  da_foreach(declaration_t*, it, analyzer->ast) {
    free_declaration(*it);
  }

  da_free(analyzer->ast);
  free(analyzer->ast);
  free(analyzer->error_ctx);

  semantic_free_function_definition(analyzer);
}


ct_test(semantic_analyze, fn_def_simple, "test/semantic_case/fn_def_simple.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for simple function definition");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, fn_def_with_params, "test/semantic_case/fn_def_with_params.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for function with params");

  function_symbol_t* fs = (function_symbol_t*) hashmap_get(analyzer.function_symbols, "main");
  ct_assert_not_null(fs, "Function symbol should be in symbol table");
  ct_assert_eq((int)fs->params_count, 2, "Function should have 2 parameters");
  ct_assert_eq(fs->params_name[0], "a", "First param name should be 'a'");
  ct_assert_eq(fs->params_type[0], TYPE_INT, "First param type should be TYPE_INT");
  ct_assert_eq(fs->params_name[1], "b", "Second param name should be 'b'");
  ct_assert_eq(fs->params_type[1], TYPE_STRING, "Second param type should be TYPE_STRING");

  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, fn_def_with_return_type, "test/semantic_case/fn_def_with_return_type.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for function with return type");

  function_symbol_t* fs = (function_symbol_t*) hashmap_get(analyzer.function_symbols, "main");
  ct_assert_not_null(fs, "Function symbol should be in symbol table");
  ct_assert_eq(fs->return_type, TYPE_INT, "Return type should be TYPE_INT");

  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, fn_duplicate_definition, "test/semantic_case/fn_duplicate_definition.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for duplicate function definition");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, fn_duplicate_params, "test/semantic_case/fn_duplicate_params.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for duplicate parameter names");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, var_decl_typed, "test/semantic_case/var_decl_typed.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for typed variable declaration");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, var_decl_untyped, "test/semantic_case/var_decl_untyped.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for untyped variable declaration");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, var_redefinition, "test/semantic_case/var_redefinition.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for variable redefinition");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, var_undefined_use, "test/semantic_case/var_undefined_use.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for using undefined variable");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, binary_wrong_type, "test/semantic_case/binary_wrong_type.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have one error for adding two different type variable");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, nested_binary_wrong_type, "test/semantic_case/nested_binary_wrong_type.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have one error for adding two different type variable");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, error_type_expression_usage, "test/semantic_case/error_type_expression_usage.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have one error for wrong var type initialization and usage of this var later");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, return_correct_type, "test/semantic_case/return_correct_type.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for correct return type");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, return_wrong_type_int_string, "test/semantic_case/return_wrong_type_int_string.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for wrong return type (int instead of string)");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, return_wrong_type_string_int, "test/semantic_case/return_wrong_type_string_int.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for wrong return type (string instead of int)");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, return_string_correct, "test/semantic_case/return_string_correct.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for correct string return");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, scope_if_statement, "test/semantic_case/scope_if_statement.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for if statement scope");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, scope_if_else, "test/semantic_case/scope_if_else.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for if-else statement scopes");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, scope_while_statement, "test/semantic_case/scope_while_statement.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for while statement scope");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, scope_for_statement, "test/semantic_case/scope_for_statement.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for for statement scope");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, scope_for_var_in_init, "test/semantic_case/scope_for_var_in_init.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for using for init var in body");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, param_in_function_body, "test/semantic_case/param_in_function_body.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for using parameter in function body");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, param_redefinition_in_body, "test/semantic_case/param_redefinition_in_body.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for redefining parameter in function body");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, multiple_functions, "test/semantic_case/multiple_functions.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for multiple function definitions");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, assign_expression, "test/semantic_case/assign_expression.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for variable assignement");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, assign_expression_type_mismatch, "test/semantic_case/assign_expression_type_mismatch.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for type mismatch variable assignment");
  free_analyzer(&analyzer);
}
