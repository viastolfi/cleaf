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
    for (size_t i = 0; i < 211; ++i) {
      if (analyzer->function_symbols->buckets[i]) {
        function_symbol_t* v = (function_symbol_t*) analyzer->function_symbols->buckets[i]->value;
        free(v->params_name);
        free(v->params_type);
      } 
    }
    hashmap_free(analyzer->function_symbols, 1);
    free(analyzer->function_symbols);
  }
}

void semantic_analyze(semantic_analyzer_t* analyzer) 
{
  if (analyzer->ast) {
    semantic_load_function_definition(analyzer);
    da_foreach(declaration_t*, it, analyzer->ast) 
      if ((*it)->type == DECLARATION_FUNC) 
        semantic_check_scope(analyzer, (*it)->func.body, NULL); 
  }

  semantic_free_function_definition(analyzer);
}

void semantic_check_scope(semantic_analyzer_t* analyzer, 
                          statement_block_t* body, 
                          scope_t* scope)
{
  scope_t* local_scope = scope_enter(scope);
  if (!local_scope) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return;
  }

  da_foreach(statement_t*, it, body) {
    statement_t* stmt = *it; 

    if (stmt->type == STATEMENT_DECL) {
      if (stmt->decl_stmt.decl->type == DECLARATION_VAR) {
        declaration_t* decl = stmt->decl_stmt.decl; 

        if (scope_resolve(local_scope, decl->var_decl.ident.name)) {
          error_report_at_position(
              analyzer->error_ctx, 
              decl->var_decl.ident.source_pos - 1,
              ERROR_SEVERITY_ERROR, 
              "already defined variable redifinition");
        } else {
          hashmap_put(local_scope->symbols, 
                      decl->var_decl.ident.name, 
                      &decl->var_decl.ident.type);
        }
      }
    }

    if (stmt->type == STATEMENT_IF) {
      semantic_check_scope(analyzer, stmt->if_stmt.then_branch, local_scope); 
    } 
  }

  scope_exit(local_scope);
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
          "already defined function redifinition");
      continue;
    }

    function_symbol_t* value = (function_symbol_t*) malloc(sizeof(function_symbol_t));
    if (!value) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      hashmap_free(func_sym, 1);
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
        if (string_array_contains(value->params_name, actual_count, (*it)->func.params.items[i].name)) 
          error_report_at_position(analyzer->error_ctx, (*it)->func.params.items[i].source_pos - 1, ERROR_SEVERITY_ERROR,
              "already defined function parameters redifinition");

        value->params_name[i] = (*it)->func.params.items[i].name;
        value->params_type[i] = (*it)->func.params.items[i].type;
        actual_count++;
      }
    }
    hashmap_put(func_sym, (*it)->func.name, value);
  }

  analyzer->function_symbols = func_sym;
}
