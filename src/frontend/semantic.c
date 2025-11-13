#include "semantic.h"

int string_array_contains(char** source, size_t source_len, const char* name)
{
  for (char** s = source; s < source + source_len; ++s)
    if (*s && strcmp(*s, name) == 0)
     return 1; 

  return 0;
}

void semantic_free_function_definition(semantic_analyzer_t* analyzer)
{
  if (analyzer->function_symbols) {
    hashmap_free(analyzer->function_symbols);
  }
}

void semantic_analyze(semantic_analyzer_t* analyzer) 
{
  if (analyzer->ast) 
    semantic_load_function_definition(analyzer);

  semantic_free_function_definition(analyzer);
}

void semantic_load_function_definition(semantic_analyzer_t* analyzer) 
{
  hashmap_t* func_sym = (hashmap_t*) malloc(sizeof(hashmap_t));
  if (!func_sym) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");  
    return;
  }
  memset(func_sym, 0, sizeof(hashmap_t));

  da_foreach(declaration_t*, it, analyzer->ast) {
    if ((*it)->type!= DECLARATION_FUNC)
      continue;

    if (hashmap_get(func_sym, (*it)->func.name)) {
      const char* pos = (*it)->source_pos + 1;
      error_report_at_position(analyzer->error_ctx, pos, ERROR_SEVERITY_ERROR,
          "already defined function name");
      continue;
    }

    function_symbol_t* value = (function_symbol_t*) malloc(sizeof(function_symbol_t));
    if (!value) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      hashmap_free(func_sym);
      return; 
    }
    memset(value, 0, sizeof(function_symbol_t));

    value->return_type = (*it)->func.return_type;

    if ((*it)->func.params.count > 0) {
      size_t actual_count = 0;
      value->params_name = calloc((*it)->func.params.count, sizeof(char*));
      if (!value->params_name) {
        error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
        return ;
      }

      value->params_type = calloc((*it)->func.params.count, sizeof(type_kind));
      if (!value->params_type) {
        error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
        return;
      }
      value->params_count = (*it)->func.params.count;

      for (size_t i = 0; i < (*it)->func.params.count; ++i) {

        if (string_array_contains(value->params_name, actual_count, (*it)->func.params.items[i].name)) {
          error_report_at_position(analyzer->error_ctx, (*it)->func.params.items[i].source_pos - 1, ERROR_SEVERITY_ERROR,
              "already defined variable name");
        } 

        value->params_name[i] = (*it)->func.params.items[i].name;
        value->params_type[i] = (*it)->func.params.items[i].type;
        actual_count++;
      }
    }
    hashmap_put(func_sym, (*it)->func.name, value);
  }

  analyzer->function_symbols = func_sym;
}
