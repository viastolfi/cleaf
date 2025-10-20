#include <stdio.h>
#include <stdlib.h>
#define LEXER_LIB_IMPLEMENTATION
#include "lexer.h"
#define DA_LIB_IMPLEMENTATION
#include "thirdparty/da.h"

typedef struct 
{
  lexer_t* items;
  size_t count;
  size_t capacity;
} tokens_array;

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

  tokens_array tokens = {0};
  lexer_init_lexer(&lex, text, text+len, (char*) malloc(4096), 4096);

  while (lexer_get_token(&lex)) {
    if (lex.token == LEXER_token_parse_error) {
      printf("\n<<<PARSE ERROR>>>\n");
      break;
    }
    da_append(&tokens, lex);
  }

  da_free(&tokens);
  free(text);
  return 0;
}
