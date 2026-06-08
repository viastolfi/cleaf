#include "compiler_definition.h"

void compiler_resources_free(compiler_resources_t* res)
{
  da_foreach(known_type_t, it, res->parser.types) {
    if (it->kind == TYPE_CUSTOM)
      if (it->name)
        free(it->name);
  }
  da_free(res->parser.types);
  free(res->parser.types);

  if (res->hir_program) {
    da_foreach(IR_function_t*, it, res->hir_program) {
      IR_free_function(*it);
    }
    da_free(res->hir_program);
    free(res->hir_program);
    res->hir_program = NULL;
  }

  da_foreach(declaration_t*, it, &res->program) {
    free_declaration(*it);
  }
  da_free(&res->program);

  for (size_t i = 0; i < res->parser.count; i++) {
    if (res->parser.items[i].string_value)
      free(res->parser.items[i].string_value);
  }
  da_free(&res->parser);

  free(res->text);
  res->text = NULL;
}

