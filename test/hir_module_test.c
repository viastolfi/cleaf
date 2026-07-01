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
#include "../src/thirdparty/error.h"

// Same lex/parse/HIR pipeline as hir_test.c, but additionally sets
// `hir_parser.current_module` so we can assert on the Phase 3 name
// mangling behavior (module labels, mangled CALL targets, "_start"
// entry point detection) without needing a full multi-module build.
before_each(int, result, char* file_path, char* expected_path, char* module_name)
{
  FILE *f = fopen(file_path, "rb");
  if (f == NULL) {
    fprintf(stderr, "error on test file %s\n", file_path);
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

  p.types = calloc(1, sizeof(known_type_array));
  populate_parser_known_type(p.types);

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

  IR_function_array* hir_program = calloc(1, sizeof(IR_function_array));
  if (!hir_program) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    abort();
  }

  semantic_analyzer_t analyzer = {0};
  analyzer.error_ctx  = error_ctx;
  analyzer.ast        = program;
  analyzer.error_count = 0;
  semantic_analyze(&analyzer);

  HIR_parser_t hir_parser = {0};
  hir_parser.error_ctx = error_ctx;
  hir_parser.error_count = 0;
  hir_parser.hir_program = hir_program;
  hir_parser.struct_symbols = analyzer.struct_symbols;
  hir_parser.current_module = module_name;
  da_foreach(declaration_t*, it, program) {
    int lowering_result = IR_lower_function(&hir_parser, *it);
    if (lowering_result != 0) {
      error_report_general(ERROR_SEVERITY_ERROR,
          "hir parsing error"); 
      abort();
    }
  }
  
  char output[2048] = "\0";
  da_foreach(IR_function_t*, it, hir_parser.hir_program) {
    char* res = IR_generate_string_program(*it);
    strcat(output, res);
    free(res);
  }

  semantic_free_program_definition(&analyzer);

  da_foreach(declaration_t*, it, program) {
    free_declaration(*it);
  }
  da_free(program);

  FILE *fr = fopen(expected_path, "rb");
  if (fr == NULL) {
    fprintf(stderr, "error on test file %s\n", expected_path);
    abort();
  }

  char* textr = (char*) malloc(1 << 20);
  int lenr = fr ? (int) fread(textr, 1, 1<<20, fr) : -1;

  if (lenr < 0) {
    fprintf(stderr, "error while reading %s\n", expected_path);
    free(textr);
    fclose(fr);
    abort();
  }
  fclose(fr);
  textr[lenr] = '\0';

  result = strcmp(textr, output); 

  free(textr);
}

ct_test(hir_module_test, entry_point_main,
    "test/hir_module_case/entry_point_main.clf",
    "test/hir_module_case/entry_point_main.res",
    "main")
{
  ct_assert_eq(result, 0,
      "module 'main' function 'main' should be mangled to 'start' and "
      "use the EXIT instruction");
}

ct_test(hir_module_test, local_call_math,
    "test/hir_module_case/local_call_math.clf",
    "test/hir_module_case/local_call_math.res",
    "math")
{
  ct_assert_eq(result, 0,
      "local calls within a module should be mangled with the module's "
      "own name (module__symbol)");
}

ct_test(hir_module_test, non_main_fn_main_module,
    "test/hir_module_case/non_main_fn_main_module.clf",
    "test/hir_module_case/non_main_fn_main_module.res",
    "main")
{
  ct_assert_eq(result, 0,
      "a non-'main' function declared in module 'main' should be "
      "mangled to 'main__<name>', not treated as the entry point");
}
