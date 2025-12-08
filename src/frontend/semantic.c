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
        function_symbol_t* v = 
          (function_symbol_t*) analyzer->function_symbols->buckets[i]->value;
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
                      (void*) (uintptr_t) fs->params_type[i] + 1);

        analyzer->current_analyzed_function = (*it)->func.name; 
        semantic_check_scope(analyzer, (*it)->func.body, function_scope); 

        scope_exit(function_scope);
      }
  }

  semantic_error_display(analyzer); 
}

int semantic_check_name_not_reserved(semantic_analyzer_t* analyzer,
                                     const char* name)
{
  return 0;
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
  } else if (semantic_check_name_not_reserved(analyzer, decl->var_decl.ident.name)) {
    return 0;
  } else {
    return 1;
  }

  return 0;
}

type_kind semantic_check_expression(semantic_analyzer_t* analyzer,
                       expression_t* expr,
                       scope_t* scope)
{
  if (expr->type == EXPRESSION_INT_LIT) 
    return TYPE_INT;
  if (expr->type == EXPRESSION_STRING_LIT)
    return TYPE_STRING;
  if (expr->type == EXPRESSION_VAR) {
    uintptr_t k = (uintptr_t) scope_resolve(scope, expr->var.name);
    if (!k) {
      semantic_error_register(analyzer, 
          expr->source_pos - 1,
          "use of undefined variable");
      return TYPE_ERROR;
    }
    return (type_kind) k - 1; 
  }

  if (expr->type == EXPRESSION_BINARY) {
    // some kind of guard, may need to handle it better even if should not happend
    if (!expr->binary.left || !expr->binary.right)
      return TYPE_ERROR;

    expression_t* lhs = expr->binary.left;
    expression_t* rhs = expr->binary.right;

    type_kind lhs_type = semantic_check_expression(analyzer, lhs, scope);
    type_kind rhs_type = semantic_check_expression(analyzer, rhs, scope);

    if (lhs_type == TYPE_ERROR && rhs_type != TYPE_ERROR) {
      return rhs_type; 
    } else if (lhs_type != TYPE_ERROR && rhs_type == TYPE_ERROR) {
      return lhs_type; 
    } else if (lhs_type == rhs_type) {
      return lhs_type;
    } else {
      semantic_error_register(analyzer, 
          rhs->source_pos - 1,
          "wrong type conversion");
      return TYPE_ERROR;
    }
  }

  if (expr->type == EXPRESSION_ASSIGN) {
    expression_t* lhs = expr->assign.lhs; 
    expression_t* rhs = expr->assign.rhs;

    type_kind lhs_type = semantic_check_expression(analyzer, lhs, scope);
    type_kind rhs_type = semantic_check_expression(analyzer, rhs, scope);

    if (lhs_type == TYPE_ERROR && rhs_type != TYPE_ERROR) {
      return rhs_type; 
    } else if (lhs_type != TYPE_ERROR && rhs_type == TYPE_ERROR) {
      return lhs_type; 
    } else if (lhs_type == rhs_type) {
      return lhs_type;
    } else if (lhs_type == TYPE_UNTYPE) {
      return rhs_type; 
    }
    
    else {
      semantic_error_register(analyzer, 
          rhs->source_pos - 1,
          "wrong type conversion");
      return TYPE_ERROR;
    }
  }

  if (expr->type == EXPRESSION_UNARY) 
    if (expr->unary.operand) {
      type_kind t = semantic_check_expression(analyzer,
          expr->unary.operand,
          scope);
      if (t == TYPE_ERROR)
        return t;
      if (t != TYPE_INT) {
        semantic_error_register(analyzer,
           expr->unary.operand->source_pos - 1,
           "expression is not assignable"); 
      } else if (expr->unary.operand->type == EXPRESSION_CALL && 
                 expr->unary.op != UNARY_POST_INC &&
                 expr->unary.op != UNARY_POST_DEC) {
        semantic_error_register(analyzer,
            expr->unary.operand->source_pos - 1,
            "cannot modify rvalue");
      } else if (expr->unary.operand->type != EXPRESSION_VAR &&
                 expr->unary.op != UNARY_NEGATE) {
        semantic_error_register(analyzer,
           expr->unary.operand->source_pos - 1,
           "expression is not assignable"); 
      }
      
      return t;
    }

  if (expr->type == EXPRESSION_CALL) {
    function_symbol_t* fs = (function_symbol_t*) hashmap_get(
        analyzer->function_symbols,
        expr->call.callee);
    if (!fs) {
      semantic_error_register(analyzer,
          expr->source_pos - 1,
          "undefined function call");
      return TYPE_ERROR;
    }

    if (fs->params_count < expr->call.arg_count) {
      semantic_error_register(analyzer,
          expr->call.args[expr->call.arg_count - 1]->source_pos - 1,
          "too many arguments to function call");
        return TYPE_ERROR;
    }

    if (fs->params_count > expr->call.arg_count) {
      if (expr->call.arg_count == 0) {
        semantic_error_register(analyzer,
            expr->source_pos + 1,
            "too few arguments to function call");
      
      } else {
        semantic_error_register(analyzer,
          expr->call.args[expr->call.arg_count - 1]->source_pos - 1,
          "too few arguments to function call");
      
      }
      return TYPE_ERROR;
    }

    for (size_t i = 0; i < fs->params_count; ++i) {
      if (semantic_check_expression(analyzer,
            expr->call.args[i],
            scope) == TYPE_UNTYPE)
        continue;

      if (semantic_check_expression(analyzer,
            expr->call.args[i],
            scope) != fs->params_type[i]) {
        semantic_error_register(analyzer,
            expr->call.args[i]->source_pos - 1,
            "wrong type conversion");
        return TYPE_ERROR;
      }
    }

    return fs->return_type;
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

  type_kind rt = semantic_check_expression(analyzer, e, scope);

  if (rt == TYPE_ERROR) return;

  if (rt != fs->return_type && fs->return_type != TYPE_UNTYPE) {
    semantic_error_register(analyzer,
       e->source_pos,
       "wrong type conversion"); 
    return;
  }
}

void semantic_check_for_statement(semantic_analyzer_t* analyzer,
                                  statement_t* stmt,
                                  scope_t* scope)
{
  scope_t* for_scope = scope_enter(scope);

  if (stmt->for_stmt.decl_init) {
    declaration_t* decl = stmt->for_stmt.decl_init;
    if (analyze_declaration(analyzer, decl, for_scope)) {
      type_kind t = semantic_check_expression(analyzer, 
          decl->var_decl.init,
          for_scope);
      hashmap_put(for_scope->symbols, 
                  decl->var_decl.ident.name, 
                  (void*)(uintptr_t)t + 1);
    }
  }

  if (stmt->for_stmt.condition)
    semantic_check_expression(analyzer,
        stmt->for_stmt.condition,
        for_scope);

  if (stmt->for_stmt.loop)
    semantic_check_expression(analyzer,
        stmt->for_stmt.loop,
        for_scope);

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
          type_kind actual_type; 
          if (!decl->var_decl.init) {
            actual_type = decl->var_decl.ident.type;
            goto var_def_put;
          }
          actual_type = semantic_check_expression(analyzer,
              decl->var_decl.init,
              local_scope);
          if (expected_type != actual_type && expected_type != TYPE_UNTYPE) {
            if (actual_type != TYPE_ERROR) {
              semantic_error_register(analyzer, decl->source_pos - 1, "type mismatch");
              actual_type = TYPE_ERROR;
            }
          }

var_def_put:
          hashmap_put(local_scope->symbols, 
                    decl->var_decl.ident.name, 
                    (void*)(uintptr_t)actual_type + 1);
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

    if (stmt->type == STATEMENT_EXPR)
      semantic_check_expression(analyzer, stmt->expr_stmt.expr, local_scope);
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
