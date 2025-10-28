#include <stdio.h>
#include <stdlib.h>
#define LEXER_LIB_IMPLEMENTATION
#include "frontend/lexer.h"
#define DA_LIB_IMPLEMENTATION
#include "thirdparty/da.h"

#include "frontend/ast_definition.h"
#include "frontend/ast.h"
#include "frontend/ast_printer.h"

int main(int argc, char** argv) 
{
  if (argc < 2) {
    fprintf(stderr, "ERROR - please provide a Cleaf file to compile\n");
    return 1;
  }

  FILE *f = fopen(argv[1] ,"rb");
  if (f == NULL) {
    fprintf(stderr, "ERROR - can't open provided file\n"); 
  }

  char *text = (char *) malloc(1 << 20);
  int len = f ? (int) fread(text, 1, 1<<20, f) : -1;
  lexer_t lex;
  if (len < 0) {
    fprintf(stderr, "Error opening file\n");
    free(text);
    fclose(f);
    return 1;
  }
  fclose(f);

  parser_t parser = {0};
  lexer_init_lexer(&lex, text, text+len, (char*) malloc(4096), 4096);

  while (lexer_get_token(&lex)) {
    if (lex.token == LEXER_token_parse_error) {
      printf("\n<<<PARSE ERROR>>>\n");
      break;
    }
    lexer_print_token(&lex);
    printf("    ");
    token_t t = lexer_copy_token(&lex);
    da_append(&parser, t);
  }
  printf("\n");

  free(lex.string_storage);

  //declaration_array program = {0};
  while ((size_t) parser.pos < parser.count) {
    declaration_t* decl = parse_declaration(&parser); 
    if (decl == NULL) {
      printf("ERROR / END of ast parsing\n");
      break;
    }
    // print_declaration(decl, 1);
    free_declaration(decl);
    // ATM this raise segfault 
    // We comment it for now
    //da_append(&program, *decl);
  }


  da_free(&parser);
  free(text);
  return 0;
}
