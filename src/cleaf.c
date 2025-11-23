#include <stdio.h>
#include <stdlib.h>
#define LEXER_LIB_IMPLEMENTATION
#include "frontend/lexer.h"
#define DA_LIB_IMPLEMENTATION
#include "thirdparty/da.h"

#include "frontend/ast_definition.h"
#include "frontend/ast.h"
#include "frontend/ast_printer.h"
#include "thirdparty/error.h"
#include "frontend/semantic.h"

#include "middleend/hir.h"

int main(int argc, char** argv) 
{
  if (argc < 2) {
    error_report_general(ERROR_SEVERITY_ERROR, "no input file provided");
    fprintf(stderr, "usage: %s <file.clf>\n", argv[0]);
    return 1;
  }

  FILE *f = fopen(argv[1] ,"rb");
  if (f == NULL) {
    error_report_general(ERROR_SEVERITY_ERROR, "cannot open file '%s'", argv[1]);
    return 1;
  }

  char *text = (char *) malloc(1 << 20);
  int len = f ? (int) fread(text, 1, 1<<20, f) : -1;
  lexer_t lex;
  if (len < 0) {
    error_report_general(ERROR_SEVERITY_ERROR, "failed to read file '%s'", argv[1]);
    free(text);
    fclose(f);
    return 1;
  }
  fclose(f);

  error_context_t error_ctx;
  error_init(&error_ctx, argv[1], text, len);

  parser_t parser = {0};
  parser.error_ctx = &error_ctx;
  
  lexer_init_lexer(&lex, text, text+len, (char*) malloc(4096), 4096);

  while (lexer_get_token(&lex)) {
    if (lex.token == LEXER_token_parse_error) {
      error_report_general(ERROR_SEVERITY_ERROR, "lexer parse error");
      return 1;
    }
    //lexer_print_token(&lex);
    //printf("\n");
    token_t t = lexer_copy_token(&lex);
    da_append(&parser, t);
  }

  free(lex.string_storage);

  declaration_array program = {0};
  while ((size_t) parser.pos < parser.count) {
    declaration_t* decl = parse_declaration(&parser); 
    if (decl == NULL) {
      error_report_general(ERROR_SEVERITY_ERROR, "ast parse error");
      return 1;
    }
    // print_declaration(decl, 1);
    da_append(&program, decl);
  }

  semantic_analyzer_t analyzer = {0};
  analyzer.error_ctx = &error_ctx;
  analyzer.ast = &program;
  analyzer.error_count = 0;

  semantic_analyze(&analyzer);
  semantic_free_function_definition(&analyzer);

  if (analyzer.error_count > 0) {
    error_report_general(ERROR_SEVERITY_NOTE, "%d errors during semantic analyzing, skip next phase", analyzer.error_count);
    return 1;
  }

  HIR_function_array* hir_program = calloc(1, sizeof(HIR_function_array));
  if (!hir_program) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }

  HIR_parser_t hir_parser = {0};
  hir_parser.error_ctx = &error_ctx;
  hir_parser.error_count = 0;
  hir_parser.hir_program = hir_program;
  da_foreach(declaration_t*, it, &program) {
    int lowering_result = HIR_lower_function(&hir_parser, *it);
    if (lowering_result != 0) {
      error_report_general(ERROR_SEVERITY_ERROR,
          "hir parsing error"); 
      return 1;
    }
  }

  da_foreach(HIR_function_t*, it, hir_parser.hir_program) {
    HIR_display_function(*it); 
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
    if (parser.items[i].string_value) {
      free(parser.items[i].string_value);
    }
  }
  
  da_free(&parser);
  free(text);
  return 0;
}
