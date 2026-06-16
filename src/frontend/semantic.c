#include "semantic.h"

static void semantic_resolve_type_size(
    semantic_analyzer_t* analyzer, known_type_t* t)
{
  if (t->size != 0)
    return;

  if (t->kind == TYPE_INT) {
    t->size = 8;
    t->name = "int";
  } else if (t->kind == TYPE_CUSTOM && t->name) {
    struct_symbol_t* sym =
      (struct_symbol_t*) hashmap_get(
          analyzer->struct_symbols, t->name);
    if (sym)
      t->element_size = sym->total_size;
  }
}

int string_array_contains(
    char** source, size_t source_len, const char* name)
{
  for (char** s = source; s < source + source_len; ++s)
    if (*s && strcmp(*s, name) == 0)
     return 1; 

  return 0;
}

void semantic_free_program_definition(semantic_analyzer_t* analyzer)
{
  if (analyzer->function_symbols) {
    for (size_t i = 0; i < 211; ++i) {
      if (analyzer->function_symbols->buckets[i]) {
        function_symbol_t* v = 
          (function_symbol_t*) 
          analyzer->function_symbols->buckets[i]->value;
        free(v->params_name);
        free(v->params_type);
      } 
    }
    hashmap_free(analyzer->function_symbols, 1);
    free(analyzer->function_symbols);
  }

  if (analyzer->struct_symbols) {
    for (size_t i = 0; i < 211; ++i) {
      if (analyzer->struct_symbols->buckets[i]) {
        struct_symbol_t* v = 
          (struct_symbol_t*) 
          analyzer->struct_symbols->buckets[i]->value;
        free(v->members_type);
        free(v->members_name);
      } 
    } 
    hashmap_free(analyzer->struct_symbols, 1);
    free(analyzer->struct_symbols);
  }

  da_free(&analyzer->semantic_errors);
}

void semantic_analyze(semantic_analyzer_t* analyzer) 
{
  if (analyzer->ast) {
    semantic_load_program_definition(analyzer);
    da_foreach(declaration_t*, it, analyzer->ast) {
      if ((*it)->type == DECLARATION_FUNC) {
        scope_t* function_scope = scope_enter(NULL);

        function_symbol_t* fs = (function_symbol_t*) hashmap_get(
            analyzer->function_symbols,
            (*it)->func.name);

        if (!fs) continue;

        for (size_t i = 0; i < fs->params_count; ++i) {
          variable_symbol_t* vs = 
            calloc(1, sizeof(variable_symbol_t));
          if (vs) {
            vs->type = fs->params_type[i].type;
            vs->is_constant = false;
            hashmap_put(
                function_scope->symbols, fs->params_name[i], vs);
          }
        }

        analyzer->current_analyzed_function = (*it)->func.name; 
        semantic_check_scope(
            analyzer, (*it)->func.body, function_scope); 

        scope_exit(function_scope);
      }
    }
  }

  semantic_error_display(analyzer); 
}

int semantic_check_name_not_reserved(const char* name)
{
  int left = 0, right = reserved_keyword_count - 1;

  while (left <= right) {
    int mid = (left + right) / 2;
    int cmp = strcmp(name, reserved_keywords[mid]);

    if (cmp == 0) return 1;
    if (cmp < 0) right = mid - 1;
    else left = mid + 1;
  }

  return 0;
}

int analyze_declaration(semantic_analyzer_t* analyzer,
                        declaration_t* decl,
                        scope_t* scope)
{
  if (scope_resolve(scope, decl->var_decl.ident.ident_name)) {
    semantic_error_register(analyzer,
        decl->var_decl.ident.source_pos - 1,
        "already defined variable redifinition");
    return 0;
  } else if (semantic_check_name_not_reserved(
        decl->var_decl.ident.ident_name)) {
    semantic_error_register(analyzer,
        decl->var_decl.ident.source_pos - 1,
        "can't named a variable using a reserved keyword");
    return 0;
  }

  if (decl->var_decl.ident.type.kind == TYPE_CUSTOM &&
      decl->var_decl.init) {
    if (!decl->var_decl.init->composite_literal.is_initializer)
      return 1;

    struct_symbol_t* struc_sym = (struct_symbol_t*)
      hashmap_get(
          analyzer->struct_symbols, 
          decl->var_decl.ident.type.name);
    if (struc_sym->members_count != 
        decl->var_decl.init->composite_literal.count) {
      semantic_error_register(
          analyzer, decl->var_decl.init->source_pos - 1,
          "must declare the exact number of members on custom var declaration");
      return 0;
    }

    expression_t* e = decl->var_decl.init;
    size_t total_found = 0;
    char** founds = 
      calloc(struc_sym->members_count, sizeof(char*));
    if (!founds) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 0;
    }

    for (size_t j = 0; j < e->composite_literal.count; ++j) {
      expression_t* assign = e->composite_literal.values[j];  
      int found = 0;
      for (size_t i = 0; i < struc_sym->members_count; ++ i) {
        if (strcmp(
              assign->assign.lhs->var.ident.ident_name, 
              struc_sym->members_name[i]) == 0) {
          for (size_t x = 0; x < struc_sym->members_count; ++x) {

            if (!founds[x])
              break;

            if (strcmp(
                  founds[x], struc_sym->members_name[i]) == 0) {
              semantic_error_register(
                 analyzer, assign->source_pos - 1,
                 "already declared struct member");
              free(founds);
              return 0;
            }
          }

          known_type_t actual = 
            semantic_check_expression(
                analyzer, assign->assign.rhs, scope);
          
          if (struc_sym->members_type[i].type.kind != actual.kind &&
              struc_sym->members_type[i].type.kind < actual.kind) {
            semantic_error_register(
                analyzer, assign->assign.rhs->source_pos - 1,
                "wrong type converstion");
          }

          founds[total_found++] = struc_sym->members_name[i];
          found = 1;
        }
      }
      if (!found) {
        free(founds);
        semantic_error_register(
            analyzer, assign->source_pos - 1,
            "member is not part of struct declaration");
        return 0;
      }
    }
    free(founds);
  }

  return 1;
}

known_type_t semantic_check_expr_int_lit(
    semantic_analyzer_t* analyzer,
    expression_t* expr,
    scope_t* scope)
{
  int v = expr->int_lit.value;
  types_t kind;
  if (v <= 255)
    kind = TYPE_U8;
  else if (v <= 65535)
    kind = TYPE_U16;
  else {
    semantic_error_register(analyzer, expr->source_pos - 1,
        "long intergers are not implemented yet");
    return (known_type_t){.kind = TYPE_ERROR};
  }
  return (known_type_t){
    .kind = kind,
    .name = types_description[kind].name,
    .element_size= types_description[kind].size,
  };
}

known_type_t semantic_check_expr_char_lit(
    semantic_analyzer_t* analyzer,
    expression_t* expr,
    scope_t* scope)
{
  return expr->var.ident.type;
}

known_type_t semantic_check_expr_var(
    semantic_analyzer_t* analyzer,
    expression_t* expr,
    scope_t* scope)
{
  variable_symbol_t* vs = 
    (variable_symbol_t*) scope_resolve(
        scope, expr->var.ident.ident_name);

  if (!vs) {
    semantic_error_register(analyzer, expr->source_pos - 1,
        "use of undefined variable");
    return (known_type_t){.kind = TYPE_ERROR};
  }

  known_type_t* k = &vs->type;

  if (expr->var.member) {
    struct_symbol_t* sym = 
      hashmap_get(analyzer->struct_symbols, k->name);

    // should never happened
    if (!sym) {
      semantic_error_register(analyzer, expr->source_pos - 1, 
          "use of undefined variable");
      return (known_type_t){.kind = TYPE_ERROR};
    }

    for (size_t i = 0; i < sym->members_count; ++i) {
      if (strcmp(
            expr->var.member->var.ident.ident_name,
            sym->members_name[i]) == 0) {
        semantic_resolve_type_size(analyzer, k);

        expr->var.ident.type = *k;
        expr->var.member->var.ident.type = 
          sym->members_type[i].type;

        return sym->members_type[i].type;
      }
    }
    semantic_error_register(
        analyzer, expr->var.member->source_pos - 1,
        "undefined struct member");
    return (known_type_t){.kind = TYPE_ERROR};
  }

  semantic_resolve_type_size(analyzer, k);
  expr->var.ident.type = *k;
  return *k; 
}

known_type_t semantic_check_expr_binary(
    semantic_analyzer_t* analyzer,
    expression_t* expr,
    scope_t* scope)
{
  // some kind of guard, may need to handle it better even if should not happend
  if (!expr->binary.left || !expr->binary.right)
    return (known_type_t){.kind = TYPE_ERROR};

  expression_t* lhs = expr->binary.left;
  expression_t* rhs = expr->binary.right;

  known_type_t lhs_type = semantic_check_expression(analyzer, lhs, scope);
  known_type_t rhs_type = semantic_check_expression(analyzer, rhs, scope);

  if (lhs_type.kind == TYPE_ERROR && 
      rhs_type.kind != TYPE_ERROR) {
    return rhs_type; 
  } 
  else if (lhs_type.kind != TYPE_ERROR && 
      rhs_type.kind == TYPE_ERROR) {
    return lhs_type; 
  } 
  else if (lhs_type.kind == rhs_type.kind) {
    return lhs_type;
  } 
  else if (lhs_type.kind < rhs_type.kind) {
    return rhs_type; 
  }
  else if (lhs_type.kind > rhs_type.kind) {
    return lhs_type; 
  }
  else {
    semantic_error_register(analyzer, 
        rhs->source_pos - 1,
        "wrong type conversion");
    return (known_type_t){.kind = TYPE_ERROR};
  }
}

known_type_t semantic_check_expr_assign(
    semantic_analyzer_t* analyzer,
    expression_t* expr,
    scope_t* scope)
{
  expression_t* lhs = expr->assign.lhs; 
  expression_t* rhs = expr->assign.rhs;

  known_type_t lhs_type = 
    semantic_check_expression(analyzer, lhs, scope);
  known_type_t rhs_type = 
    semantic_check_expression(analyzer, rhs, scope);

  variable_symbol_t* sym = 
    (variable_symbol_t*) scope_resolve(
        scope,
        lhs->var.ident.ident_name);

  if (!sym) goto assign_type_check;

  struct_symbol_t* struct_sym = 
    (struct_symbol_t*) hashmap_get(
        analyzer->struct_symbols,
        sym->type.name);

  function_symbol_t* func_sym =
    (function_symbol_t*) hashmap_get(
        analyzer->function_symbols,
        analyzer->current_analyzed_function);

  if (sym->is_constant) {
    semantic_error_register(
        analyzer, lhs->source_pos - 1,
        "you are trying to reassign constant variable, this is not authorized");
  } 
  else if (struct_sym) {
    // Not sure this could work with nested stuct members
    // This is something I should work on later
    // For now, let's keep this simple
    // TODO: refactor this later
    expression_t* member = lhs->var.member;
    for (size_t i = 0; i < struct_sym->members_count; ++i) {
      if (strcmp(
            struct_sym->members_name[i], 
            member->var.ident.ident_name) != 0) 
        continue;
         
      if (struct_sym->members_type[i].is_constant) {
        semantic_error_register(
            analyzer, member->source_pos - 1,
            "you are trying to reassign constant variable, this is not authorized");
      }
    }       
  } 
  else if (func_sym) {
    for (size_t i = 0; i < func_sym->params_count; ++i) {
      if (strcmp(
           func_sym->params_name[i],
           lhs->var.ident.ident_name) != 0)
       continue;

      if (func_sym->params_type[i].is_constant) {
        semantic_error_register(
            analyzer, lhs->source_pos - 1,
            "you are trying to reassign constant variable, this is not authorized");
      } 
    }
  }

assign_type_check:
  if (lhs_type.kind == TYPE_ERROR && 
      rhs_type.kind != TYPE_ERROR) {
    return rhs_type; 
  } 
  else if (lhs_type.kind != TYPE_ERROR && 
      rhs_type.kind == TYPE_ERROR) {
    return lhs_type; 
  } 
  else if (lhs_type.kind == rhs_type.kind) {
    return lhs_type;
  } 
  else if (lhs_type.kind == TYPE_UNTYPE) {
    return rhs_type; 
  }
  else if (lhs_type.kind < rhs_type.kind) {
    semantic_error_register(
        analyzer, rhs->source_pos - 1,
        "value exceed lhs max accepting integer value");
    return (known_type_t){.kind = TYPE_ERROR};
  } else if (lhs_type.kind > rhs_type.kind) {
    return lhs_type; 
  }
  
  else {
    semantic_error_register(analyzer, 
        rhs->source_pos - 1,
        "wrong type conversion");
    return (known_type_t){.kind = TYPE_ERROR};
  }
}

known_type_t semantic_check_expr_unary(
    semantic_analyzer_t* analyzer,
    expression_t* expr,
    scope_t* scope)
{
  if (expr->unary.operand) {
    known_type_t t = semantic_check_expression(analyzer,
        expr->unary.operand,
        scope);
    if (t.kind == TYPE_ERROR)
      return t;
    if (t.kind != TYPE_INT &&
        t.kind != TYPE_U8  &&
        t.kind != TYPE_U16 &&
        t.kind != TYPE_U32 &&
        t.kind != TYPE_U64) {
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

  return (known_type_t){.kind = TYPE_ERROR};
}

known_type_t semantic_check_expr_call(
    semantic_analyzer_t* analyzer,
    expression_t* expr,
    scope_t* scope)
{
  function_symbol_t* fs = (function_symbol_t*) hashmap_get(
      analyzer->function_symbols,
      expr->call.callee);
  if (!fs) {
    semantic_error_register(analyzer,
        expr->source_pos - 1,
        "undefined function call");
    return (known_type_t){.kind = TYPE_ERROR};
  }

  if (fs->params_count < expr->call.arg_count) {
    semantic_error_register(analyzer,
        expr->call.args[expr->call.arg_count - 1]->source_pos - 1,
        "too many arguments to function call");
      return (known_type_t){.kind = TYPE_ERROR};
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
    return (known_type_t){.kind = TYPE_ERROR};
  }

  for (size_t i = 0; i < fs->params_count; ++i) {
    known_type_t arg_type = semantic_check_expression(analyzer,
          expr->call.args[i],
          scope);
    if (arg_type.kind == TYPE_UNTYPE)
      continue;

    if (arg_type.kind != fs->params_type[i].type.kind &&
        fs->params_type[i].type.kind < arg_type.kind) {
      semantic_error_register(analyzer,
          expr->call.args[i]->source_pos - 1,
          "wrong type conversion");
      return (known_type_t){.kind = TYPE_ERROR};
    }
  }

  return fs->return_type;
}

known_type_t semantic_check_expr_composite_literal(
    semantic_analyzer_t* analyzer,
    expression_t* expr,
    scope_t* scope)
{
  return (known_type_t){.kind = TYPE_CUSTOM}; 
}

known_type_t semantic_check_expression(
    semantic_analyzer_t* analyzer,
    expression_t* expr,
    scope_t* scope)
{
  if (expr->type == EXPRESSION_INT_LIT)
    return semantic_check_expr_int_lit(analyzer, expr, scope);
  if (expr->type == EXPRESSION_CHAR_LIT)
    return semantic_check_expr_char_lit(analyzer, expr, scope);
  if (expr->type == EXPRESSION_VAR)
    return semantic_check_expr_var(analyzer, expr, scope);
  if (expr->type == EXPRESSION_BINARY)
    return semantic_check_expr_binary(analyzer, expr, scope);
  if (expr->type == EXPRESSION_ASSIGN)
    return semantic_check_expr_assign(analyzer, expr, scope);
  if (expr->type == EXPRESSION_UNARY)
    return semantic_check_expr_unary(analyzer, expr, scope);
  if (expr->type == EXPRESSION_CALL)
    return semantic_check_expr_call(analyzer, expr, scope);
  if (expr->type == EXPRESSION_COMPOSITE_LITERAL)
    return semantic_check_expr_composite_literal(analyzer, expr, scope);
  return (known_type_t){.kind = TYPE_ERROR};
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

  known_type_t rt = semantic_check_expression(analyzer, e, scope);

  if (rt.kind == TYPE_ERROR) return;

  if (rt.kind < fs->return_type.kind) return;

  if (rt.kind > fs->return_type.kind) {
    semantic_error_register(
       analyzer, e->source_pos - 1,
       "returned value exceed expected value");
    return;
  }

  if (rt.kind != fs->return_type.kind && 
      fs->return_type.kind != TYPE_UNTYPE) {
    semantic_error_register(analyzer,
       e->source_pos - 1,
       "wrong type conversion"); 
    return;
  }
}

void semantic_check_for_statement(semantic_analyzer_t* analyzer,
                                  statement_t* stmt,
                                  scope_t* scope)
{
  scope_t* for_scope = scope_enter(scope);

  if (stmt->for_stmt.init_kind == FOR_INIT_DECL && 
      stmt->for_stmt.decl_init) {

    declaration_t* decl = stmt->for_stmt.decl_init;

    if (analyze_declaration(analyzer, decl, for_scope)) {
      known_type_t inferred = 
        semantic_check_expression(
            analyzer, decl->var_decl.init, for_scope);

      variable_symbol_t* vs = calloc(1, sizeof(variable_symbol_t));

      if (vs && decl->var_decl.ident.type.kind != TYPE_VAR) {
        vs->type = inferred;
        semantic_resolve_type_size(analyzer, &vs->type);
      } else {
        vs->type = (known_type_t) {
          .kind = TYPE_INT,
          .name = types_description[TYPE_INT].name,
          .element_size = types_description[TYPE_INT].size,
        };
      }
      vs->is_constant = false;
      decl->var_decl.ident.type = vs->type;
      hashmap_put(
          for_scope->symbols, decl->var_decl.ident.ident_name, vs);
    }
  } 

  else if (stmt->for_stmt.init_kind == FOR_INIT_EXPR && 
      stmt->for_stmt.expr_init) {
    semantic_check_expression(
        analyzer, stmt->for_stmt.expr_init, for_scope);
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

void semantic_check_var_declaration(
    semantic_analyzer_t* analyzer,
    declaration_t* decl,
    scope_t* scope)
{
  if (!analyze_declaration(analyzer, decl, scope))
    return;

  known_type_t expected_type = decl->var_decl.ident.type;
  known_type_t actual_type; 

  if (!decl->var_decl.init) {
    actual_type = decl->var_decl.ident.type;
    goto var_def_put;
  }

  if (decl->var_decl.ident.type.array_len > 0) {
    size_t len = decl->var_decl.ident.type.array_len;
    for (size_t i = 0; i < len; ++i) {
      expression_t* e = 
        decl->var_decl.init->composite_literal.values[i];

      actual_type = semantic_check_expression(analyzer, e, scope);

      if (expected_type.kind < actual_type.kind) {
        semantic_error_register(
            analyzer, e->source_pos -1,
            "can't store value. Exceeding max type accetping value");
      }
      if (expected_type.kind >= actual_type.kind &&
          actual_type.kind != TYPE_CHAR)
        continue;

      if (actual_type.kind != TYPE_ERROR) {
        semantic_error_register(
            analyzer, decl->source_pos - 1, 
            "type mismatch");
        actual_type.kind = TYPE_ERROR;
      } 
    }
    goto var_def_put;
  }

  actual_type = 
    semantic_check_expression(
        analyzer, 
        decl->var_decl.init,
        scope); 
  
  if (expected_type.kind != actual_type.kind && 
      expected_type.kind != TYPE_UNTYPE &&
      expected_type.kind != TYPE_VAR) {

    if (expected_type.kind < actual_type.kind) {
      semantic_error_register(
          analyzer, decl->source_pos -1,
          "can't store value. Exceeding max type accetping value");
    }
    if (expected_type.kind >= actual_type.kind &&
        actual_type.kind != TYPE_CHAR)
      goto var_def_put;

    if (actual_type.kind != TYPE_ERROR) {
      semantic_error_register(
          analyzer, decl->source_pos - 1, 
          "type mismatch");
      actual_type.kind = TYPE_ERROR;
    }
  }

var_def_put:
  variable_symbol_t* vs = calloc(1, sizeof(variable_symbol_t));
  if (expected_type.kind == TYPE_VAR) {
    vs->type = (known_type_t) {
      .kind = TYPE_INT,
      .name = types_description[TYPE_INT].name,
      .element_size = types_description[TYPE_INT].size,
    };
  } 
  else {
    vs->type = expected_type;
  }
  semantic_resolve_type_size(analyzer, &vs->type);
  vs->is_constant = decl->var_decl.ident.is_constant;
  decl->var_decl.ident.type = vs->type;
  hashmap_put(
      scope->symbols, 
      decl->var_decl.ident.ident_name, vs);
}

void semantic_check_if_statement(semantic_analyzer_t* analyzer,
                                 statement_t* stmt,
                                 scope_t* scope)
{
  semantic_check_expression(
      analyzer, stmt->if_stmt.condition, scope);
  if (stmt->if_stmt.then_branch)
    semantic_check_scope(
        analyzer, stmt->if_stmt.then_branch, scope); 
  if (stmt->if_stmt.else_branch)
    semantic_check_scope(
        analyzer, stmt->if_stmt.else_branch, scope);
}

void semantic_check_while_statement(semantic_analyzer_t* analyzer,
                                    statement_t* stmt,
                                    scope_t* scope)
{
  semantic_check_expression(
      analyzer, stmt->while_stmt.condition, scope);
  if (stmt->while_stmt.body) 
    semantic_check_scope(
        analyzer, stmt->while_stmt.body, scope); 
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
        semantic_check_var_declaration(
            analyzer, stmt->decl_stmt.decl, local_scope);
      }
    }

    if (stmt->type == STATEMENT_IF)
      semantic_check_if_statement(analyzer, stmt, local_scope); 

    if (stmt->type == STATEMENT_WHILE)
      semantic_check_while_statement(analyzer, stmt, local_scope); 

    if (stmt->type == STATEMENT_FOR) 
      semantic_check_for_statement(analyzer, stmt, local_scope); 

    if (stmt->type == STATEMENT_RETURN) 
      semantic_check_return_statement(analyzer, stmt, local_scope); 

    if (stmt->type == STATEMENT_ASM)
      semantic_check_asm_statement(analyzer, stmt, local_scope);

    if (stmt->type == STATEMENT_EXPR)
      semantic_check_expression(
          analyzer, stmt->expr_stmt.expr, local_scope);

    if (stmt->type == STATEMENT_FREE)
      semantic_check_free_statement(analyzer, stmt, local_scope);
  }

  scope_exit(local_scope);
}

void semantic_check_free_statement(
    semantic_analyzer_t* analyzer,
    statement_t* stmt,
    scope_t* scope)
{
  known_type_t t = 
    semantic_check_expression(analyzer, stmt->free_stmt.expr, scope);

  if (t.kind != TYPE_CUSTOM) {
    semantic_error_register(
        analyzer, stmt->free_stmt.expr->source_pos - 1,
        "you are tryning to free unallocated memory. Please not that only stuct typed variable and arrays are allocated in the heap");
    return;
  }

  variable_symbol_t* sym = (variable_symbol_t*)
    scope_resolve(scope, stmt->free_stmt.expr->var.ident.ident_name);
  free(sym);
  scope_remove(scope, stmt->free_stmt.expr->var.ident.ident_name);
}

void semantic_check_asm_statement(
    semantic_analyzer_t* analyzer,
    statement_t* stmt,
    scope_t* scope)
{
  size_t count = 0;
  for (size_t i = 0; i < stmt->asm_stmt.instr_count; ++i) {
    char* str = stmt->asm_stmt.instr[i];
    while ((str = strchr(str, '%')) != NULL) {
      count++;
      str++; 
    } 
  }

  if (count != stmt->asm_stmt.arg_count) {
    semantic_error_register( 
        analyzer, stmt->source_pos - 1,
        "error in asm statement, please make sure you used as much '%%' symbols in the instructions as argument you are giving to the function");
  }

  for (size_t i = 0; i < stmt->asm_stmt.arg_count; ++i) {
    semantic_check_expression(
        analyzer, stmt->asm_stmt.args[i], scope);
  }
}

void semantic_load_program_definition(semantic_analyzer_t* analyzer) 
{
  // TODO: return some sort of status code to make this stop the compiler
  hashmap_t* func_sym = (hashmap_t*) malloc(sizeof(hashmap_t));
  if (!func_sym) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");  
    return;
  }
  memset(func_sym, 0, sizeof(hashmap_t));
  hashmap_t* struct_sym = calloc(1, sizeof(hashmap_t));
  if (!struct_sym) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return;
  }

  da_foreach(declaration_t*, it, analyzer->ast) {
    if ((*it)->type == DECLARATION_FUNC) {
      if (semantic_check_name_not_reserved((*it)->func.name)) {
        semantic_error_register(analyzer,
            (*it)->source_pos + 1,
            "can't named a function using a reserved keyword");
        continue;
      }

      if (hashmap_get(func_sym, (*it)->func.name)) {
        const char* pos = (*it)->source_pos + 1;
        semantic_error_register(analyzer, pos, 
            "already defined function redefinition");
        continue;
      }

      function_symbol_t* value = 
        (function_symbol_t*) malloc(sizeof(function_symbol_t));
      if (!value) {
        error_report_general(ERROR_SEVERITY_ERROR, 
            "out of memory"); 
        hashmap_free(func_sym, 1);
        hashmap_free(struct_sym, 1);
        return; 
      }
      memset(value, 0, sizeof(function_symbol_t));

      value->return_type.kind = (*it)->func.return_type.kind;

      if ((*it)->func.params.count <= 0) {
        goto hash_func_put; 
      }

      size_t actual_count = 0;
      value->params_name = 
        calloc((*it)->func.params.count, sizeof(char*));
      if (!value->params_name) {
        error_report_general(ERROR_SEVERITY_ERROR, 
            "out of memory"); 
        hashmap_free(func_sym, 1);
        hashmap_free(struct_sym, 1);
        return ;
      }

      value->params_type = 
        calloc((*it)->func.params.count, sizeof(variable_symbol_t));
      if (!value->params_type) {
        error_report_general(ERROR_SEVERITY_ERROR, 
            "out of memory"); 
        hashmap_free(func_sym, 1);
        hashmap_free(struct_sym, 1);
        return;
      }
      value->params_count = (*it)->func.params.count;

      for (size_t i = 0; i < (*it)->func.params.count; ++i) {
        if (string_array_contains(value->params_name, 
              actual_count, 
              (*it)->func.params.items[i].ident_name)) 
          semantic_error_register(
              analyzer, 
              (*it)->func.params.items[i].source_pos - 1,
              "already defined function parameters redifinition");

        value->params_name[i] = 
          (*it)->func.params.items[i].ident_name;
        value->params_type[i].type = 
          (*it)->func.params.items[i].type;
        value->params_type[i].is_constant =
          (*it)->func.params.items[i].is_constant;
        actual_count++;
      }

hash_func_put:
      hashmap_put(func_sym, (*it)->func.name, value);
    } else if ((*it)->type == DECLARATION_STRUCT) {
      if (semantic_check_name_not_reserved((*it)->struc.name)) {
        semantic_error_register(analyzer, (*it)->source_pos + 1,
           "can't name a struct after a reserved keyword"); 
        continue;
      } 

      if (hashmap_get(struct_sym, (*it)->struc.name)) {
        const char* pos = (*it)->source_pos + 1; 
        semantic_error_register(analyzer, pos,
            "already defined struct redifinition");
        continue;
      }

      struct_symbol_t* value = calloc(1, sizeof(struct_symbol_t));
      if (!value) {
        error_report_general(ERROR_SEVERITY_ERROR,
           "out of memory"); 
        hashmap_free(func_sym, 1);
        hashmap_free(struct_sym, 1);
        return;
      }

      // we still store the struct for redeclaration error but with dumie values
      if ((*it)->struc.members.count <= 0) {
        semantic_error_register(analyzer, (*it)->source_pos + 1,
           "struct might have at least one member"); 
        goto hash_struct_put;
      }

      value->members_count = (*it)->struc.members.count;
      value->members_name =
        calloc(value->members_count, sizeof(char*));
      if (!value->members_name) {
        error_report_general(ERROR_SEVERITY_ERROR, 
            "out of memory"); 
        hashmap_free(func_sym, 1);
        hashmap_free(struct_sym, 1);
        return ;
      }

      value->members_type = 
        calloc(value->members_count, sizeof(variable_symbol_t));
      if (!value->members_type) {
        error_report_general(ERROR_SEVERITY_ERROR, 
            "out of memory"); 
        hashmap_free(func_sym, 1);
        hashmap_free(struct_sym, 1);
        return;
      }

      size_t actual_count = 0;
      for (size_t i = 0; i < (*it)->struc.members.count; ++i) {
        if (string_array_contains(value->members_name,
             actual_count,
            (*it)->struc.members.items[i].ident_name))
          semantic_error_register(
            analyzer,
            (*it)->struc.members.items[i].source_pos - 1,
            "already defined struct members redifinition");

        value->members_name[i] =
          (*it)->struc.members.items[i].ident_name;
        value->members_type[i].type =
          (*it)->struc.members.items[i].type;
        value->members_type[i].is_constant =
          (*it)->struc.members.items[i].is_constant;
        value->total_size += 
          (*it)->struc.members.items[i].type.element_size;
        actual_count++;
      }

hash_struct_put:
      hashmap_put(struct_sym, (*it)->struc.name, value);
    }
  }

  analyzer->function_symbols = func_sym;
  analyzer->struct_symbols = struct_sym;
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
