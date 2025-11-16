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

  da_free(&analyzer->semantic_errors);
}

void semantic_analyze(semantic_analyzer_t* analyzer) 
{
  if (analyzer->ast) {
    semantic_load_function_definition(analyzer);
    da_foreach(declaration_t*, it, analyzer->ast) 
      if ((*it)->type == DECLARATION_FUNC) {
        scope_t* function_scope = scope_enter(NULL);

        function_symbol_t* fs = (function_symbol_t*) hashmap_get(
            analyzer->function_symbols,
            (*it)->func.name);

        for (size_t i = 0; i < fs->params_count; ++i) 
          hashmap_put(function_scope->symbols,
                      fs->params_name[i],
                      &(fs->params_type[i]));

        analyzer->current_analyzed_function = (*it)->func.name; 
        semantic_check_scope(analyzer, (*it)->func.body, function_scope); 

        scope_exit(function_scope);
      }
  }

  semantic_error_display(analyzer); 
}

int analyze_declaration(semantic_analyzer_t* analyzer,
                        declaration_t* decl,
                        scope_t* scope)
{
  if (scope_resolve(scope, decl->var_decl.ident.name)) {
    semantic_error_register(analyzer,
        decl->var_decl.ident.source_pos - 1,
        "already defined variable redifinition");
    return 0;
  } else {
    return 1;
  }

  return 0;
}

type_kind analyze_expression(semantic_analyzer_t* analyzer,
                       expression_t* expr,
                       scope_t* scope)
{
  if (expr->type == EXPRESSION_INT_LIT) 
    return TYPE_INT;
  if (expr->type == EXPRESSION_STRING_LIT)
    return TYPE_STRING;
  if (expr->type == EXPRESSION_VAR) {
    type_kind* k = (type_kind*) hashmap_get(scope->symbols, 
        expr->var.name);
    if (!k) {
      semantic_error_register(analyzer, 
          expr->source_pos - 1,
          "use of undefined variable");
      return TYPE_ERROR;
    }
    return *k; 
  }

    if (expr->type == EXPRESSION_BINARY) {
      // some kind of guard, may need to handle it better even if should not happend
      if (!expr->binary.left || !expr->binary.right)
        return TYPE_ERROR;

      expression_t* lhs = expr->binary.left;
      expression_t* rhs = expr->binary.right;

      type_kind lhs_type = analyze_expression(analyzer, lhs, scope);
      type_kind rhs_type = analyze_expression(analyzer, rhs, scope);

      if (lhs_type == TYPE_ERROR && rhs_type != TYPE_ERROR) {
        return rhs_type; 
      } else if (lhs_type != TYPE_ERROR && rhs_type == TYPE_ERROR) {
        return lhs_type; 
      } else if (lhs_type == rhs_type) {
        return lhs_type;
      } else {
        semantic_error_register(analyzer, 
            rhs->source_pos - 1,
            "wrong type convertion");
        return TYPE_ERROR;
      }
  }

  return TYPE_ERROR;
}

void semantic_check_return_statement(semantic_analyzer_t* analyzer,
                                     statement_t* stmt,
                                     scope_t* scope)
{
  expression_t* e = stmt->ret.value;
  function_symbol_t* fs = (function_symbol_t*) hashmap_get(
      analyzer->function_symbols, 
      analyzer->current_analyzed_function);

  // some kind of guard, should be always false as parser is working
  if (!e) return;

  if (e->type == EXPRESSION_INT_LIT) 
    if (fs->return_type != TYPE_INT) 
      semantic_error_register(analyzer,
                              e->source_pos,
                              "incompatible return type");

  if (e->type == EXPRESSION_STRING_LIT)
    if (fs->return_type != TYPE_STRING)
      semantic_error_register(analyzer,
                              e->source_pos,
                              "incompatible return type");
}

void semantic_check_for_statement(semantic_analyzer_t* analyzer,
                                  statement_t* stmt,
                                  scope_t* scope)
{
  scope_t* for_scope = scope_enter(scope);

  if (stmt->for_stmt.decl_init) {
    declaration_t* decl = stmt->for_stmt.decl_init;
    if (analyze_declaration(analyzer, decl, for_scope))
      hashmap_put(for_scope->symbols, 
                  decl->var_decl.ident.name, 
                  &decl->var_decl.ident.type);
  }

  // TODO: analyze condition and loop

  semantic_check_scope(analyzer, stmt->for_stmt.body, for_scope);

  scope_exit(for_scope);
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

        if (analyze_declaration(analyzer, decl, local_scope)) {
          type_kind expected_type = decl->var_decl.ident.type;
          type_kind actual_type = analyze_expression(analyzer,
              decl->var_decl.init,
              local_scope);
          if (expected_type != actual_type && expected_type != TYPE_UNTYPE) {
            if (actual_type != TYPE_ERROR) {
              semantic_error_register(analyzer, decl->source_pos - 1, "type mismatch");
              actual_type = TYPE_ERROR;
            }
          }
          hashmap_put(local_scope->symbols, 
                    decl->var_decl.ident.name, 
                    &actual_type);
        }
      }
    }

    if (stmt->type == STATEMENT_IF) {
      if (stmt->if_stmt.then_branch)
        semantic_check_scope(analyzer, stmt->if_stmt.then_branch, local_scope); 
      if (stmt->if_stmt.else_branch)
        semantic_check_scope(analyzer, stmt->if_stmt.else_branch, local_scope);
    } 

    if (stmt->type == STATEMENT_WHILE) {
      if (stmt->while_stmt.body) 
        semantic_check_scope(analyzer, stmt->while_stmt.body, local_scope); 
    }

    if (stmt->type == STATEMENT_FOR) 
      semantic_check_for_statement(analyzer, stmt, local_scope); 

    if (stmt->type == STATEMENT_RETURN) 
      semantic_check_return_statement(analyzer, stmt, local_scope); 

    // if (stmt->type == STATEMENT_EXPR)
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
      semantic_error_register(analyzer, pos, "already defined function redifinition");
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
        if (string_array_contains(value->params_name, 
                                  actual_count, 
                                  (*it)->func.params.items[i].name)) 
          semantic_error_register(
              analyzer, 
              (*it)->func.params.items[i].source_pos - 1,
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

void semantic_error_display(semantic_analyzer_t* analyzer)
{
  da_foreach(diagnostic_t, it, &analyzer->semantic_errors) 
    error_report_at_position(analyzer->error_ctx,
                             it->position,
                             ERROR_SEVERITY_ERROR,
                             it->message);
}

void semantic_error_register(semantic_analyzer_t* analyzer, 
                             const char* pos, 
                             const char* msg) 
{
  diagnostic_t err = {.position = pos, .message = msg};
  da_append(&analyzer->semantic_errors, err);
  analyzer->error_count++;
}
