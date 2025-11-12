#include "semantic.h"

void semantic_free_function_definition(function_symbol_table_t* fst)
{
  if (fst->items) {
    da_foreach(function_symbol_t, it, fst) {
      if ((*it).name)
        free((*it).name);  
      if ((*it).params_type)
        free((*it).params_type);
    }

    free(fst->items);
    fst->items = NULL;
  }

  free(fst);
}

void semantic_analyze(declaration_array* ast) 
{
  if (ast) {
    function_symbol_table_t* funcs = semantic_load_function_definition(ast);
    if (funcs)
      semantic_free_function_definition(funcs);
  }
}

function_symbol_table_t* semantic_load_function_definition(declaration_array* ast) 
{
  // TODO: add memory safety
  function_symbol_table_t* fst = (function_symbol_table_t*) malloc(sizeof(function_symbol_table_t));
  if (!fst) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");  
    return NULL;
  }
  memset(fst, 0, sizeof(function_symbol_table_t));

  da_foreach(declaration_t*, it, ast) {
    if ((*it)->type!= DECLARATION_FUNC)
      continue;

    function_symbol_t f = {0};
    char* name = strdup((*it)->func.name); 
    if (!name) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return NULL; 
    }
    type_kind return_type = (*it)->func.return_type;
    f.name = name;
    f.return_type = return_type;
    if ((*it)->func.params.count > 0) {
      f.params_type = calloc((*it)->func.params.count, sizeof(type_kind));
      if (!f.params_type) {
        error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
        return NULL;
      }
      for (size_t i = 0; i < (*it)->func.params.count; ++i) 
        f.params_type[i] = (*it)->func.params.items[i].type; 
    }
    da_append(fst, f);
  }

  return fst;
}
