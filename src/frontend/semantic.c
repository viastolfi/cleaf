#include "semantic.h"

void semantic_analyze(declaration_array* ast) 
{
  function_symbol_table_t* funcs = semantic_load_function_definition(ast);
}

function_symbol_table_t* semantic_load_function_definition(declaration_array* ast) 
{
  // TODO: add memory safety
  function_symbol_table_t* fst = (function_symbol_table_t*) malloc(sizeof(function_symbol_table_t));
  da_foreach(declaration_t*, it, ast) {
    function_symbol_t f = {0};
    char* name = strdup((*it)->func.name); 
    type_t return_type = (*it)->func.return_type;
    f.name = name;
    f.return_type = return_type;
    if ((*it)->func.params.count > 0) {
      f.params_type = calloc((*it)->func.params.count, sizeof(type_t));
      for (size_t i = 0; i < (*it)->func.params.count; ++i) 
        f.params_type[i] = (*it)->func.params.items[i].type; 
    }
    da_append(fst, f);
  }

  return fst;
}
