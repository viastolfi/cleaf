#include "ast.h"

#define DA_LIB_IMPLEMENTATION
#include "../thirdparty/da.h"

int ast_parse_ident(expression_array* ast, token_t token)
{
  if (strcmp(token.string_value, "fn") == 0 ) {
      declaration_t d = {.type = DECLARATION_FUNC, .next = NULL, .func = {0}};
      da_append(ast, d);
      return 0;
  }

  // fallback is variable declaration
  // TODO: implement this
  return 1;
}

int ast_build_ast(expression_array* ast, token_t token)
{
  switch (token.type) {
    case (LEXER_token_id):
      return ast_parse_ident(ast, token);
    // NOT IMPLEMENTED YET
    default:
      return 1;
  }
}
