#include <stdio.h>
#include <stdlib.h>
#define LEXER_LIB_IMPLEMENTATION
#include "frontend/lexer.h"
#define DA_LIB_IMPLEMENTATION
#include "thirdparty/da.h"
#include "frontend/ast.h"

typedef struct 
{
  token_t* items;
  size_t count;
  size_t capacity;
} tokens_array;

void store_token(tokens_array* tokens, lexer_t* lex) 
{
  token_t token;
  token.type = lex->token;
  switch (lex->token) {
    case LEXER_token_id: 
      token.string_value = malloc(lex->string_len);
      strcpy(token.string_value, lex->string_value);
      token.string_len = lex->string_len;
      break;
    default:
      return;
  }

  da_append(tokens, token);
}

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
    store_token(&tokens, &lex);
  }

  expression_array expr = {0};
  // TODO: use da_foreach instead
  da_foreach(token_t, it, &tokens) {
    if (ast_build_ast(&expr, *it) != 0) {
      printf("ERROR / END of ast parsing\n");
      break;
    }
    printf("Token : %ld parsed \n", (*it).type);
  }

  // da_free(&tokens);
  free(text);
  return 0;
}
