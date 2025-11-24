#define CTEST_BEFORE_EACH
#define CTEST_LIB_IMPLEMENTATION
#include "ctest.h"

#define LEXER_LIB_IMPLEMENTATION
#include "../src/frontend/lexer.h"
#define DA_LIB_IMPLEMENTATION
#include "../src/thirdparty/da.h"

#include "../src/frontend/ast_definition.h"
#include "../src/frontend/ast.h"
#include "../src/middleend/hir.h"
#include "../src/thirdparty/error.h"

before_each(int, result, char* file_path, char* expected_path)
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

  for (size_t i = 0; i < p.count; i++) {
    if (p.items[i].string_value) {
      free(p.items[i].string_value);
    }
  }
  da_free(&p);

  HIR_function_array* hir_program = calloc(1, sizeof(HIR_function_array));
  if (!hir_program) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    abort();
  }

  HIR_parser_t hir_parser = {0};
  hir_parser.error_ctx = error_ctx;
  hir_parser.error_count = 0;
  hir_parser.hir_program = hir_program;
  da_foreach(declaration_t*, it, program) {
    int lowering_result = HIR_lower_function(&hir_parser, *it);
    if (lowering_result != 0) {
      error_report_general(ERROR_SEVERITY_ERROR,
          "hir parsing error"); 
      abort();
    }
  }

  // assuming in text context, we have only one function
  char* res = HIR_generate_string_program(hir_parser.hir_program->items[0]); 
  da_foreach(declaration_t*, it, program) {
    free_declaration(*it);
  }
  da_free(program);

  FILE *fr = fopen(expected_path, "rb");
  if (fr == NULL) {
    fprintf(stderr, "error on test file test/semantic_file/%s\n", expected_path);
    abort();
  }

  char* textr = (char*) malloc(1 << 20);
  int lenr = fr ? (int) fread(textr, 1, 1<<20, fr) : -1;

  if (len < 0) {
    fprintf(stderr, "error while reading %s\n", expected_path);
    free(textr);
    fclose(fr);
    abort();
  }
  fclose(fr);

  result = strcmp(textr, res); 

  free(textr);
  free(res);
}

ct_test(hir_test, return_stmt, "test/hir_case/return_stmt.clf", "test/hir_case/return_stmt.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
}
