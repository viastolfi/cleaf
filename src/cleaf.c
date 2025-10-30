#include <stdio.h>
#include <stdlib.h>
#define LEXER_LIB_IMPLEMENTATION
#include "frontend/lexer.h"
#define DA_LIB_IMPLEMENTATION
#include "thirdparty/da.h"

#include "frontend/ast_definition.h"
#include "frontend/ast.h"
#include "frontend/ast_printer.h"
#include "frontend/error.h"

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

  // Initialize error context
  error_context_t error_ctx;
  error_init(&error_ctx, argv[1], text, len);

  parser_t parser = {0};
  parser.error_ctx = &error_ctx;
  
  lexer_init_lexer(&lex, text, text+len, (char*) malloc(4096), 4096);

  while (lexer_get_token(&lex)) {
    if (lex.token == LEXER_token_parse_error) {
      error_report_general(ERROR_SEVERITY_ERROR, "lexer parse error");
      break;
    }
    lexer_print_token(&lex);
    printf("    ");
    token_t t = lexer_copy_token(&lex);
    da_append(&parser, t);
  }
  printf("\n");

  free(lex.string_storage);

  declaration_array program = {0};
  while ((size_t) parser.pos < parser.count) {
    declaration_t* decl = parse_declaration(&parser); 
    if (decl == NULL) {
      break;
    }
    // print_declaration(decl, 1);
    da_append(&program, decl);
  }

  da_foreach(declaration_t*, it, &program) {
    free_declaration(*it);
  }
  da_free(&program);
  da_free(&parser);
  free(text);
  return 0;
}
