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
#include "../src/middleend/hir.h"
#include "../src/backend/codegen.h"
#include "../src/backend/x86_64_definition.h"
#include "../src/thirdparty/error.h"
#include "../src/thirdparty/string_builder.h"

typedef struct { int n; } chunk_counter_t;

static void counter_chunk_gen(void* ctx, char* out)
{
  chunk_counter_t* c = (chunk_counter_t*)ctx;
  snprintf(out, RAND_CHUNK_LEN + 2, ".c%d", c->n++);
}

before_each(int, result, char* file_path, char* expected_path)
{
  // --- Read source file ---
  FILE* f = fopen(file_path, "rb");
  if (f == NULL) {
    fprintf(stderr, "error opening source file: %s\n", file_path);
    abort();
  }
  char* text = (char*)malloc(1 << 20);
  int len = (int)fread(text, 1, 1 << 20, f);
  fclose(f);
  if (len < 0) {
    fprintf(stderr, "error reading source file: %s\n", file_path);
    free(text);
    abort();
  }

  // --- Lex ---
  error_context_t* error_ctx = calloc(1, sizeof(error_context_t));
  if (!error_ctx) abort();
  error_init(error_ctx, file_path, text, len);

  parser_t p = {0};
  p.error_ctx = error_ctx;
  lexer_t lex;
  char* storage = malloc(255);
  if (!storage) abort();
  lexer_init_lexer(&lex, text, text + len, storage, 255);

  while (lexer_get_token(&lex)) {
    if (lex.token == LEXER_token_parse_error)
      break;
    token_t t = lexer_copy_token(&lex);
    da_append(&p, t);
  }
  free(storage);

  // --- Parse ---
  p.types = calloc(1, sizeof(known_type_array));
  populate_parser_known_type(p.types);

  declaration_array* program = calloc(1, sizeof(declaration_array));
  if (!program) abort();
  while ((size_t)p.pos < p.count) {
    declaration_t* decl = parse_declaration(&p);
    if (!decl) {
      fprintf(stderr, "parse error in: %s\n", file_path);
      abort();
    }
    da_append(program, decl);
  }

  free(text);
  for (size_t i = 0; i < p.count; i++) {
    if (p.items[i].string_value)
      free(p.items[i].string_value);
  }
  da_free(&p);

  // --- Semantic analysis ---
  semantic_analyzer_t analyzer = {0};
  analyzer.error_ctx = error_ctx;
  analyzer.ast = program;
  semantic_analyze(&analyzer);
  semantic_free_program_definition(&analyzer);
  if (analyzer.error_count > 0) {
    fprintf(stderr, "semantic error(s) in: %s\n", file_path);
    abort();
  }

  // --- HIR lowering ---
  HIR_function_array* hir_program = calloc(1, sizeof(HIR_function_array));
  if (!hir_program) abort();

  HIR_parser_t hir_parser = {0};
  hir_parser.error_ctx = error_ctx;
  hir_parser.hir_program = hir_program;
  chunk_counter_t chunk_counter = {0};
  hir_parser.gen_chunk = counter_chunk_gen;
  hir_parser.chunk_ctx = &chunk_counter;

  da_foreach(declaration_t*, it, program) {
    if (HIR_lower_function(&hir_parser, *it) != 0) {
      fprintf(stderr, "HIR lowering error in: %s\n", file_path);
      abort();
    }
  }

  // --- Codegen ---
  string_builder_t sb = {0};
  da_foreach(HIR_function_t*, it, hir_parser.hir_program) {
    if (CODEGEN_write_function(&sb, *it, &x86_64_target) != 0) {
      fprintf(stderr, "codegen error in: %s\n", file_path);
      abort();
    }
  }

  // --- Cleanup HIR & AST ---
  da_foreach(HIR_function_t*, it, hir_program) {
    HIR_free_function(*it);
  }
  da_free(hir_program);
  free(hir_program);

  da_foreach(declaration_t*, it, program) {
    free_declaration(*it);
  }
  da_free(program);
  free(program);
  free(error_ctx);

  // --- Read expected asm file ---
  FILE* fe = fopen(expected_path, "rb");
  if (fe == NULL) {
    fprintf(stderr, "error opening expected file: %s\n", expected_path);
    da_free(&sb);
    abort();
  }
  char* expected = (char*)malloc(1 << 20);
  int expected_len = (int)fread(expected, 1, 1 << 20, fe);
  fclose(fe);
  if (expected_len < 0) {
    fprintf(stderr, "error reading expected file: %s\n", expected_path);
    free(expected);
    da_free(&sb);
    abort();
  }
  expected[expected_len] = '\0';

  // --- Compare ---
  result = strcmp(expected, sb.items);

  free(expected);
  da_free(&sb);
}

ct_test(codegen_test, return_stmt, "test/codegen_case/return_stmt.clf", "test/codegen_case/return_stmt.asm") {
  ct_assert_eq(result, 0, "codegen gives right output");
}

ct_test(codegen_test, simple_binary, "test/codegen_case/simple_binary.clf", "test/codegen_case/simple_binary.asm") {
  ct_assert_eq(result, 0, "codegen gives right output");
}

ct_test(codegen_test, nested_binary, "test/codegen_case/nested_binary.clf", "test/codegen_case/nested_binary.asm") {
  ct_assert_eq(result, 0, "codegen gives right output");
}

ct_test(codegen_test, basic_var_decl, "test/codegen_case/basic_var_decl.clf", "test/codegen_case/basic_var_decl.asm") {
  ct_assert_eq(result, 0, "codegen gives right output");
}

ct_test(codegen_test, initialized_var_decl, "test/codegen_case/initialized_var_decl.clf", "test/codegen_case/initialized_var_decl.asm") {
  ct_assert_eq(result, 0, "codegen gives right output");
}

ct_test(codegen_test, expression_init_var_decl, "test/codegen_case/expression_init_var_decl.clf", "test/codegen_case/expression_init_var_decl.asm") {
  ct_assert_eq(result, 0, "codegen gives right output");
}

ct_test(codegen_test, var_loading, "test/codegen_case/var_loading.clf", "test/codegen_case/var_loading.asm") {
  ct_assert_eq(result, 0, "codegen gives right output");
}

ct_test(codegen_test, unary_op, "test/codegen_case/unary_op.clf", "test/codegen_case/unary_op.asm") {
  ct_assert_eq(result, 0, "codegen gives right output");
}

ct_test(codegen_test, basic_if, "test/codegen_case/basic_if.clf", "test/codegen_case/basic_if.asm") {
  ct_assert_eq(result, 0, "codegen gives right output");
}

ct_test(codegen_test, if_else, "test/codegen_case/if_else.clf", "test/codegen_case/if_else.asm") {
  ct_assert_eq(result, 0, "codegen gives right output");
}

ct_test(codegen_test, all_comparison_if, "test/codegen_case/all_comparison_if.clf", "test/codegen_case/all_comparison_if.asm") {
  ct_assert_eq(result, 0, "codegen gives right output");
}

ct_test(codegen_test, while_stmt, "test/codegen_case/while_stmt.clf", "test/codegen_case/while_stmt.asm") {
  ct_assert_eq(result, 0, "codegen gives right output");
}

ct_test(codegen_test, for_stmt, "test/codegen_case/for_stmt.clf", "test/codegen_case/for_stmt.asm") {
  ct_assert_eq(result, 0, "codegen gives right output");
}

ct_test(codegen_test, call, "test/codegen_case/call.clf", "test/codegen_case/call.asm") {
  ct_assert_eq(result, 0, "codegen gives right output");
}
