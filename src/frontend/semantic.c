#include "semantic.h"

int is_param_name_declared(function_params_name_t* fpn, const char* name)
{
  da_foreach(char*, it, fpn) {
    if (strcmp((*it), name) == 0)
     return 1; 
  }
  return 0;
}

int is_function_name_declared(function_symbol_table_t* fst, const char* name)
{
  da_foreach(function_symbol_t, it, fst) {
    if (strcmp(it->name, name) == 0)
     return 1; 
  }
  return 0;
}

void semantic_free_function_definition(semantic_analyzer_t* analyzer)
{
  if (analyzer->fst) {
    da_foreach(function_symbol_t, it, analyzer->fst) {
      if ((*it).name)
        free((*it).name);  
      if ((*it).params_type)
        free((*it).params_type);
    }

    free(analyzer->fst->items);
    analyzer->fst->items = NULL;
    free(analyzer->fst);
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
  // TODO: add memory safety
  function_symbol_table_t* fst = (function_symbol_table_t*) malloc(sizeof(function_symbol_table_t));
  if (!fst) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");  
    return;
  }
  memset(fst, 0, sizeof(function_symbol_table_t));

  da_foreach(declaration_t*, it, analyzer->ast) {
    if ((*it)->type!= DECLARATION_FUNC)
      continue;

    if (is_function_name_declared(fst, (*it)->func.name)) {
      const char* pos = (*it)->source_pos + 1;
      error_report_at_position(analyzer->error_ctx, pos, ERROR_SEVERITY_ERROR,
          "already defined function name");
      continue;
    }

    function_symbol_t f = {0};
    char* name = strdup((*it)->func.name); 
    if (!name) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return; 
    }
    type_kind return_type = (*it)->func.return_type;
    f.name = name;
    f.return_type = return_type;
    if ((*it)->func.params.count > 0) {
      function_params_name_t declared = {0};

      f.params_type = calloc((*it)->func.params.count, sizeof(type_kind));
      if (!f.params_type) {
        error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
        return;
      }
      memset(f.params_type, 0, sizeof(*(f.params_type)));

      for (size_t i = 0; i < (*it)->func.params.count; ++i) {
        if (is_param_name_declared(&declared, (*it)->func.params.items[i].name)) {
          error_report_at_position(analyzer->error_ctx, (*it)->func.params.items[i].source_pos, ERROR_SEVERITY_ERROR,
              "already defined variable name");
        } else {
          da_append(&declared, (*it)->func.params.items[i].name);
        } 
        f.params_type[i] = (*it)->func.params.items[i].type; 
      }

      da_free(&declared);
    }
    da_append(fst, f);
  }

  analyzer->fst = fst;
}
