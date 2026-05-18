#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LEXER_LIB_IMPLEMENTATION
#include "frontend/lexer.h"
#define DA_LIB_IMPLEMENTATION
#include "thirdparty/da.h"
#define LOG_LIB_IMPLEMENTATION
#include "thirdparty/log.h"

#include "frontend/ast_definition.h"
#include "frontend/ast.h"
#include "frontend/ast_printer.h"
#include "thirdparty/error.h"
#include "frontend/semantic.h"
#include "middleend/hir.h"

int main(int argc, char** argv) 
{
  const char* filename = NULL;
  log_verbosity_t verbosity = LOG_SILENT;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-v") == 0)
      verbosity = LOG_VERBOSE;
    else if (strcmp(argv[i], "-V") == 0)
      verbosity = LOG_DUMP;
    else if (argv[i][0] != '-')
      filename = argv[i];
    else {
      error_report_general(ERROR_SEVERITY_ERROR, "unknown flag '%s'", argv[i]);
      fprintf(stderr, "usage: %s [-v|-V] <file.clf>\n", argv[0]);
      return 1;
    }
  }

  log_set_verbosity(verbosity);

  if (!filename) {
    error_report_general(ERROR_SEVERITY_ERROR, "no input file provided");
    fprintf(stderr, "usage: %s [-v|-V] <file.clf>\n", argv[0]);
    return 1;
  }

  log_phase("compiling", "'%s'", filename);

  FILE *f = fopen(filename, "rb");
  if (f == NULL) {
    error_report_general(ERROR_SEVERITY_ERROR, "cannot open file '%s'", filename);
    return 1;
  }

  char *text = (char *) malloc(1 << 20);
  int len = (int) fread(text, 1, 1 << 20, f);
  fclose(f);
  if (len < 0) {
    error_report_general(ERROR_SEVERITY_ERROR, "failed to read file '%s'", filename);
    free(text);
    return 1;
  }

  error_context_t error_ctx;
  error_init(&error_ctx, filename, text, len);

  lexer_t lex;
  parser_t parser = {0};
  parser.error_ctx = &error_ctx;

  lexer_init_lexer(&lex, text, text + len, (char*) malloc(4096), 4096);

  while (lexer_get_token(&lex)) {
    if (lex.token == LEXER_token_parse_error) {
      error_report_general(ERROR_SEVERITY_ERROR, "lexer parse error");
      return 1;
    }
    token_t t = lexer_copy_token(&lex);
    da_append(&parser, t);
  }
  free(lex.string_storage);

  log_phase("lexing", "%zu tokens", parser.count);

  declaration_array program = {0};
  while ((size_t) parser.pos < parser.count) {
    declaration_t* decl = parse_declaration(&parser);
    if (decl == NULL) {
      error_report_general(ERROR_SEVERITY_ERROR, "ast parse error");
      return 1;
    }
    da_append(&program, decl);
  }

  log_phase("parsing", "%zu declaration(s)", program.count);

  if (log_is_dump()) {
    log_section_begin("AST");
    ast_print_program(&program);
    log_section_end();
  }

  semantic_analyzer_t analyzer = {0};
  analyzer.error_ctx = &error_ctx;
  analyzer.ast = &program;
  analyzer.error_count = 0;

  semantic_analyze(&analyzer);
  semantic_free_function_definition(&analyzer);

  if (analyzer.error_count > 0) {
    log_phase("semantic", "%d error(s)", analyzer.error_count);
    error_report_general(ERROR_SEVERITY_NOTE,
        "%d error(s) during semantic analysis, aborting", analyzer.error_count);
    return 1;
  }
  log_phase("semantic", "ok");

  HIR_function_array* hir_program = calloc(1, sizeof(HIR_function_array));
  if (!hir_program) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1;
  }

  HIR_parser_t hir_parser = {0};
  hir_parser.error_ctx = &error_ctx;
  hir_parser.error_count = 0;
  hir_parser.hir_program = hir_program;
  rand_t rng;
  rand_init(&rng);
  HIR_PARSER_USE_RNG(hir_parser, &rng);

  da_foreach(declaration_t*, it, &program) {
    int lowering_result = HIR_lower_function(&hir_parser, *it);
    if (lowering_result != 0) {
      error_report_general(ERROR_SEVERITY_ERROR, "HIR lowering error");
      return 1;
    }
  }

  log_phase("HIR lowering", "%zu function(s)", hir_program->count);

  if (log_is_dump()) {
    log_section_begin("HIR");
    da_foreach(HIR_function_t*, it, hir_parser.hir_program) {
      HIR_display_function(*it);
    }
    log_section_end();
  }

  da_foreach(declaration_t*, it, &program) {
    free_declaration(*it);
  }
  da_free(&program);

  da_foreach(HIR_function_t*, it, hir_program) {
    HIR_free_function(*it);
  }
  da_free(hir_program);
  free(hir_program);

  for (size_t i = 0; i < parser.count; i++) {
    if (parser.items[i].string_value)
      free(parser.items[i].string_value);
  }
  da_free(&parser);
  free(text);
  return 0;
}
