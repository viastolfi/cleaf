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

typedef struct { int n; } chunk_counter_t;

static void counter_chunk_gen(void* ctx, char* out)
{
  chunk_counter_t* c = (chunk_counter_t*)ctx;
  snprintf(out, RAND_CHUNK_LEN + 2, ".c%d", c->n++);
}

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
  chunk_counter_t chunk_counter = {0};
  hir_parser.gen_chunk = counter_chunk_gen;
  hir_parser.chunk_ctx = &chunk_counter;
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
    fprintf(stderr, "error on test file test/semantic_file/%s\n", expected_path);
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

ct_test(hir_test, return_stmt, "test/hir_case/return_stmt.clf", "test/hir_case/return_stmt.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
}

ct_test(hir_test, simple_binary, "test/hir_case/simple_binary.clf", "test/hir_case/simple_binary.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
}

ct_test(hir_test, nested_binary, "test/hir_case/nested_binary.clf", "test/hir_case/nested_binary.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
}

ct_test(hir_test, basic_var_decl, "test/hir_case/basic_var_decl.clf", "test/hir_case/basic_var_decl.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
}

ct_test(hir_test, initialized_var_decl, "test/hir_case/initialized_var_decl.clf", "test/hir_case/initialized_var_decl.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
}

ct_test(hir_test, expression_init_var_decl, "test/hir_case/expression_init_var_decl.clf", "test/hir_case/expression_init_var_decl.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
}

ct_test(hir_test, var_loading, "test/hir_case/var_loading.clf", "test/hir_case/var_loading.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
} 

ct_test(hir_test, unary_op, "test/hir_case/unary_op.clf", "test/hir_case/unary_op.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
}

ct_test(hir_test, basic_if, "test/hir_case/basic_if.clf", "test/hir_case/basic_if.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
}

ct_test(hir_test, if_else, "test/hir_case/if_else.clf", "test/hir_case/if_else.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
}

ct_test(hir_test, all_comparison_if, "test/hir_case/all_comparison_if.clf", "test/hir_case/all_comparison_if.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
}

ct_test(hir_test, while_stmt, "test/hir_case/while_stmt.clf", "test/hir_case/while_stmt.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
}

ct_test(hir_test, for_stmt, "test/hir_case/for_stmt.clf", "test/hir_case/for_stmt.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
}

ct_test(hir_test, call, "test/hir_case/call.clf", "test/hir_case/call.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
}

ct_test(hir_test, struct_var_allocation, "test/hir_case/struct_var_allocation.clf", "test/hir_case/struct_var_allocation.res") {
  ct_assert_eq(result, 0, "hit parsing give right output");
}

ct_test(hir_test, struct_var_zero_init, "test/hir_case/struct_var_zero_init.clf", "test/hir_case/struct_var_zero_init.res") {
  ct_assert_eq(result, 0, "hir parsing give right output");
}

ct_test(hir_test, struct_var_designated_init, "test/hir_case/struct_var_designated_init.clf", "test/hir_case/struct_var_designated_init.res") {
  ct_assert_eq(result, 0, "hir parsing give right output for designated struct initializer");
}

ct_test(hir_test, struct_var_designated_init_reordered, "test/hir_case/struct_var_designated_init_reordered.clf", "test/hir_case/struct_var_designated_init_reordered.res") {
  ct_assert_eq(result, 0, "hir parsing gives right output for reordered designated struct initializer");
}

ct_test(hir_test, struct_member_access_first, "test/hir_case/struct_member_access_first.clf", "test/hir_case/struct_member_access_first.res") {
  ct_assert_eq(result, 0, "hir parsing gives right output for struct member access (first member)");
}

ct_test(hir_test, struct_member_access_second, "test/hir_case/struct_member_access_second.clf", "test/hir_case/struct_member_access_second.res") {
  ct_assert_eq(result, 0, "hir parsing gives right output for struct member access (second member, offset 8)");
}

ct_test(hir_test, u8_u16_u64_vars, "test/hir_case/u8_u16_u64_vars.clf", "test/hir_case/u8_u16_u64_vars.res") {
  ct_assert_eq(result, 0, "hir gives b/w/q prefixes for u8/u16/u64 typed variables");
}

ct_test(hir_test, int_binary_typed, "test/hir_case/int_binary_typed.clf", "test/hir_case/int_binary_typed.res") {
  ct_assert_eq(result, 0, "hir propagates int size through binary ops and return");
}

ct_test(hir_test, asm_no_args, "test/hir_case/asm_no_args.clf", "test/hir_case/asm_no_args.res") {
  ct_assert_eq(result, 0, "hir gives right output for asm with no args");
}

ct_test(hir_test, asm_with_arg, "test/hir_case/asm_with_arg.clf", "test/hir_case/asm_with_arg.res") {
  ct_assert_eq(result, 0, "hir gives right output for asm with variable interpolation");
}

ct_test(hir_test, char_var_declaration, "test/hir_case/char_var_declaration.clf", "test/hir_case/char_var_declaration.res") {
  ct_assert_eq(result, 0, "hir gives right output for char var declaration");
}

ct_test(hir_test, free_stmt, "test/hir_case/free_stmt.clf", "test/hir_case/free_stmt.res") {
  ct_assert_eq(result, 0, "hir gives right output for free statement");
}

ct_test(hir_test, array_u8_init, "test/hir_case/array_u8_init.clf", "test/hir_case/array_u8_init.res") {
  ct_assert_eq(result, 0, "hir gives right output for u8 array with initializer");
}

ct_test(hir_test, array_int_init, "test/hir_case/array_int_init.clf", "test/hir_case/array_int_init.res") {
  ct_assert_eq(result, 0, "hir gives right output for int array with initializer");
}

ct_test(hir_test, array_no_init, "test/hir_case/array_no_init.clf", "test/hir_case/array_no_init.res") {
  ct_assert_eq(result, 0, "hir gives right output for array declaration without initializer");
}

ct_test(hir_test, array_int_index_literal, "test/hir_case/array_int_index_literal.clf", "test/hir_case/array_int_index_literal.res") {
  ct_assert_eq(result, 0, "hir gives right output for int array access with literal index 0");
}

ct_test(hir_test, array_int_index_nonzero, "test/hir_case/array_int_index_nonzero.clf", "test/hir_case/array_int_index_nonzero.res") {
  ct_assert_eq(result, 0, "hir gives right output for int array access with non-zero literal index");
}

ct_test(hir_test, array_u8_index_literal, "test/hir_case/array_u8_index_literal.clf", "test/hir_case/array_u8_index_literal.res") {
  ct_assert_eq(result, 0, "hir gives right output for u8 array access (element_size=1 in DIRECT_MUL)");
}

ct_test(hir_test, array_index_var_index, "test/hir_case/array_index_var_index.clf", "test/hir_case/array_index_var_index.res") {
  ct_assert_eq(result, 0, "hir gives right output for array access with variable index");
}

ct_test(hir_test, array_index_as_var, "test/hir_case/array_index_as_var.clf", "test/hir_case/array_index_as_var.res") {
  ct_assert_eq(result, 0, "hir gives right output for array index result stored in a variable");
}
