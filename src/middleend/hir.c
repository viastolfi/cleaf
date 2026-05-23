#include "hir.h"
  
void HIR_free_instruction(HIR_instruction_t* instr) {
  switch (instr->kind) {
    case HIR_STORE_VAR:
      if (instr->var.name)
        free(instr->var.name);
      break;
    case HIR_LOAD_VAR:
      if (instr->var.name)
        free(instr->var.name);
      break;
    case HIR_JMP_NOT_EQUAL:
    case HIR_JMP:
    case HIR_JMP_EQUAL:
    case HIR_JMP_GREATER_THAN_EQUAL:
    case HIR_JMP_GREATER_THAN:
    case HIR_JMP_LOWER_THAN_EQUAL:
    case HIR_JMP_LOWER_THAN:
      if (instr->chunk_name)
        free(instr->chunk_name);
      break;
    case HIR_CHUNK:
      if (instr->chunk_name)
        free(instr->chunk_name);
      break;
    case HIR_CALL:
      if (instr->func_name)
        free(instr->func_name);
      break;
    default:
      break;
  }
  free(instr);
}

void HIR_free_function(HIR_function_t* func) 
{
  if (func->name)
    free(func->name);
    
  if (func->code) {
    da_foreach(HIR_instruction_t*, it, func->code) 
      HIR_free_instruction(*it); 
    free(func->code->items);
    free(func->code);
  }

  free(func);
}

int HIR_lower_declaration(
    HIR_parser_t* hir,
    declaration_t* decl,
    HIR_function_t* func)
{
  (void)hir;
  if (decl->type != DECLARATION_VAR) {
    error_report_general(ERROR_SEVERITY_ERROR, 
        "awaiting var declaration, getting something else");
    return -1;
  }

  HIR_instruction_t* instr = calloc(1, sizeof(HIR_instruction_t));
  if (!instr) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return -1;  
  }

  instr->kind = HIR_STORE_VAR;
  instr->var.name = strdup(decl->var_decl.ident.ident_name);
  if (!instr->var.name) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return -1;
  }

  if (decl->var_decl.init) {
    HIR_lower_expression(hir, decl->var_decl.init, func);   
    instr->var.is_init = 1;
    instr->a = func->next_temp_id;
  } else {
    if (decl->var_decl.ident.type.kind == TYPE_INT) {
      instr->var.is_init = 0; 
    }
    else if (decl->var_decl.ident.type.kind == TYPE_CUSTOM) {
      HIR_instruction_t* alloc = calloc(
          1, sizeof(HIR_instruction_t));   
      if (!alloc) {
        error_report_general(
            ERROR_SEVERITY_ERROR, "out of memory"); 
        return -1;
      }
      alloc->kind = HIR_ALLOC;
      alloc->alloc_size = decl->var_decl.ident.type.size;

      da_append(func->code, alloc);
      instr->var.is_init = 1;
      instr->a = -1;
    }
  }

  da_append(func->code, instr);

  // TODO: this only works if we only store int
  // Must change as we add more base types and custom types
  func->stack_reserve_size += 8;
  return 0;
}

int HIR_lower_unary_expression(HIR_parser_t* hir,
    expression_t* expr,
    HIR_function_t* func)
{
  (void)hir;
  if (expr->unary.op == UNARY_POST_INC ||
      expr->unary.op == UNARY_POST_DEC) {
    HIR_instruction_t* load = calloc(1, sizeof(HIR_instruction_t)); 
    if (!load) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }

    load->kind = HIR_LOAD_VAR;
    load->var.name = strdup(expr->unary.operand->var.name);
    load->dest = ++(func->next_temp_id);
    if (!load->var.name) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }

    da_append(func->code, load);  

    HIR_instruction_t* mov = calloc(1, sizeof(HIR_instruction_t));
    if (!mov) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");  
      return 1;
    }

    mov->kind = HIR_MOV;
    mov->dest = func->next_temp_id + 1;
    mov->a = func->next_temp_id;
    da_append(func->code, mov);

    HIR_instruction_t* op = calloc(1, sizeof(HIR_instruction_t));
    if (!op) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");  
      return 1;
    }
    
    switch (expr->unary.op) {
      case UNARY_POST_INC: 
        op->kind = HIR_INC;
        break;
      case UNARY_POST_DEC:
        op->kind = HIR_DEC;
        break;
      default: break;
    }
    op->dest = func->next_temp_id + 1;
    da_append(func->code, op);

    HIR_instruction_t* str = calloc(1, sizeof(HIR_instruction_t));
    if (!str) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }

    str->kind = HIR_STORE_VAR;
    str->a = func->next_temp_id + 1; 
    str->var.name = strdup(expr->unary.operand->var.name);
    str->var.is_init = 1;
    if (!str->var.name) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");  
      return 1;
    }

    da_append(func->code, str);
    return 0;
  }

  if (expr->unary.op == UNARY_PRE_INC ||
      expr->unary.op == UNARY_PRE_DEC) {
    HIR_instruction_t* load = calloc(1, sizeof(HIR_instruction_t)); 
    if (!load) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return 1;   
    }

    load->kind = HIR_LOAD_VAR;
    load->dest = ++(func->next_temp_id);
    load->var.name = strdup(expr->unary.operand->var.name);
    if (!load->var.name) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }
    da_append(func->code, load);

    HIR_instruction_t* op = calloc(1, sizeof(HIR_instruction_t));
    if (!op) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }

    switch (expr->unary.op) {
      case UNARY_PRE_INC:
        op->kind = HIR_INC;
        break;
      case UNARY_PRE_DEC:
        op->kind = HIR_DEC;
        break;
      default:
        break;  
    }
    op->dest = func->next_temp_id;
    da_append(func->code, op);

    HIR_instruction_t* str = calloc(1, sizeof(HIR_instruction_t));
    if (!str) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }

    str->kind = HIR_STORE_VAR;
    str->a = func->next_temp_id;
    str->var.name = strdup(expr->unary.operand->var.name);
    if (!str->var.name) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return 1; 
    }
    str->var.is_init = 1;

    da_append(func->code, str);
    return 0;
  }

  error_report_general(ERROR_SEVERITY_NOT_IMPLEMENTED,
      "NEGATE and NOT unary expression are not implemented by cleaf compiler yet");

  return 1;
}

int HIR_lower_call_expression(HIR_parser_t* hir,
    expression_t* expr,
    HIR_function_t* func)
{
  for (int i = 0; i < (int) expr->call.arg_count; ++i) {
    HIR_instruction_t* set_arg = calloc(1, sizeof(HIR_instruction_t)); 
    if (!set_arg) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }
    set_arg->kind = HIR_MOV;
    set_arg->dest = -i - 1;

    if (HIR_lower_expression(hir, expr->call.args[i], func) != 0)
      return 1;

    set_arg->a = func->next_temp_id;
    
    da_append(func->code, set_arg);
  } 

  HIR_instruction_t* call = calloc(1, sizeof(HIR_instruction_t));
  if (!call) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  call->kind = HIR_CALL;
  call->func_name = strdup(expr->call.callee);
  if (!call->func_name) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  da_append(func->code, call);

  HIR_instruction_t* result = calloc(1, sizeof(HIR_instruction_t));
  if (!result) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1;
  }
  result->kind = HIR_MOV;
  result->dest = ++(func->next_temp_id);
  result->a = -1;
  da_append(func->code, result);

  return 0;
}

int HIR_lower_binary_expression(expression_t* expr,
    HIR_parser_t* hir,
    HIR_instruction_t* instr,
    HIR_function_t* func)
{
  switch(expr->binary.op) {
    case BINARY_ADD:
      instr->binary_op = HIR_BINARY_ADD;
      break;
    case BINARY_SUB:
      instr->binary_op = HIR_BINARY_SUB;
      break;
    case BINARY_MUL:
      instr->binary_op = HIR_BINARY_MUL;
      break;
    case BINARY_EQ:
    case BINARY_NEQ:
    case BINARY_GT:
    case BINARY_GTE:
    case BINARY_LT:
    case BINARY_LTE:
      instr->binary_op = HIR_BINARY_CMP;
      break;
    default:
      return 1;
  }

  if (HIR_lower_expression(hir, expr->binary.left, func) != 0)
    return -1;
  instr->a = func->next_temp_id;

  if (HIR_lower_expression(hir, expr->binary.right, func) != 0)
    return -1;
  instr->b = func->next_temp_id;

  return 0;
}

int HIR_lower_expression(HIR_parser_t* hir,
    expression_t* expr,
    HIR_function_t* func)
{
  if (expr->type == EXPRESSION_INT_LIT) {
    HIR_instruction_t* instr = calloc(1, sizeof(HIR_instruction_t));  
    if (!instr) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return -1;
    }
    instr->kind = HIR_INT_CONST;
    instr->dest = ++(func->next_temp_id);
    instr->int_value = expr->int_lit.value;
    da_append(func->code, instr);
    return 0;
  }

  if (expr->type == EXPRESSION_VAR) {
    HIR_instruction_t* instr = calloc(1, sizeof(HIR_instruction_t));  
    if (!instr) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return -1;
    }

    instr->kind = HIR_LOAD_VAR;
    instr->dest = ++(func->next_temp_id);
    instr->var.name = strdup(expr->var.name);
    if (!instr->var.name) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return -1;
    }

    da_append(func->code, instr);
    return 0;
  }

  if (expr->type == EXPRESSION_BINARY) {
    HIR_instruction_t* instr = calloc(1, sizeof(HIR_instruction_t));  
    if (!instr) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return -1;
    }
    if (HIR_lower_binary_expression(expr, hir, instr, func) != 0) {
      error_report_at_position(hir->error_ctx,
          expr->source_pos,
          ERROR_SEVERITY_ERROR,
          "error while lowering binary expression");
      return 1;
    }
    instr->kind = HIR_BINARY;
    da_append(func->code, instr);
    return 0;
  }

  // TODO: make sure no error can still occurs here even after semantic analysis
  if (expr->type == EXPRESSION_ASSIGN) {
    int res = HIR_lower_expression(hir, expr->assign.rhs, func);

    if (res != 0) {
      //TODO: do we have to propagate error ?
      return 1;
    }

    HIR_instruction_t* instr = calloc(1, sizeof(HIR_instruction_t));
    if (!instr) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return -1;
    }

    instr->kind = HIR_STORE_VAR;
    instr->a = func->next_temp_id;
    // This works only if lhs in assign is a var
    // TODO: make sure this won't break as the compiler evolve
    instr->var.name = strdup(expr->assign.lhs->var.name);
    instr->var.is_init = 1;
    if (!instr->var.name) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }

    da_append(func->code, instr);
    return 0;
  }

  if (expr->type == EXPRESSION_UNARY) {
    return HIR_lower_unary_expression(hir, expr, func);
  }

  if (expr->type == EXPRESSION_CALL) {
    return HIR_lower_call_expression(hir, expr, func); 
  }

  return 1;
}

int HIR_lower_for_statement(
    HIR_parser_t* hir,
    statement_t* stmt, 
    HIR_function_t* func)
{
  int err = 0;
  if (stmt->for_stmt.init_kind == FOR_INIT_DECL) 
    err = HIR_lower_declaration(hir, stmt->for_stmt.decl_init, func);
  else
    err = HIR_lower_expression(hir, stmt->for_stmt.expr_init, func);
  if (err)
    return 1;

  char* main_chunk = calloc(2 + RAND_CHUNK_LEN, sizeof(char));
  if (!main_chunk) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  hir->gen_chunk(hir->chunk_ctx, main_chunk);

  HIR_instruction_t* main_label = calloc(1, sizeof(HIR_instruction_t));
  if (!main_label) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  main_label->kind = HIR_CHUNK;
  main_label->chunk_name = strdup(main_chunk);
  da_append(func->code, main_label);

  da_foreach(statement_t*, it, stmt->for_stmt.body) {
    int err = HIR_lower_statement(hir, *it, func);
    if (err)
     return 1; 
  }

  if (HIR_lower_expression(hir, stmt->for_stmt.loop, func) != 0)
   return 1; 

  if (HIR_lower_expression(hir, stmt->for_stmt.condition, func) != 0)
    return 1;

  HIR_instruction_t* jump = calloc(1, sizeof(HIR_instruction_t));
  if (!jump) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1;
  }
  jump->chunk_name = strdup(main_chunk);
  switch (stmt->for_stmt.condition->binary.op) {
  case BINARY_EQ:
    jump->kind = HIR_JMP_EQUAL;
    break;
  case BINARY_NEQ:
    jump->kind = HIR_JMP_NOT_EQUAL;
    break;
  case BINARY_GT:
    jump->kind = HIR_JMP_GREATER_THAN;
    break;
  case BINARY_LT:
    jump->kind = HIR_JMP_LOWER_THAN;
    break;
  case BINARY_GTE:
    jump->kind = HIR_JMP_GREATER_THAN_EQUAL;
    break;
  case BINARY_LTE:
    jump->kind = HIR_JMP_LOWER_THAN_EQUAL;
    break;
  default:
    free(jump);
    return 1;
  }
  da_append(func->code, jump);

  free(main_chunk);
  return 0;
}

int HIR_lower_while_statement(
    HIR_parser_t* hir,
    statement_t* stmt,
    HIR_function_t* func)
{
  char* condition_chunk = calloc(2 + RAND_CHUNK_LEN, sizeof(char));
  if (!condition_chunk) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1; 
  }
  hir->gen_chunk(hir->chunk_ctx, condition_chunk);

  HIR_instruction_t* condition_label = calloc(1, sizeof(HIR_instruction_t));
  if (!condition_label) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  condition_label->kind = HIR_CHUNK;
  condition_label->chunk_name = strdup(condition_chunk);
  da_append(func->code, condition_label);

  if (HIR_lower_expression(hir, stmt->while_stmt.condition, func) != 0)
    return 1;

  char* next_chunk = calloc(2 + RAND_CHUNK_LEN, sizeof(char));
  if (!next_chunk) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  hir->gen_chunk(hir->chunk_ctx, next_chunk);

  HIR_instruction_t* jump = calloc(1, sizeof(HIR_instruction_t));
  if (!jump) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1;
  }
  jump->chunk_name = strdup(next_chunk);
  switch (stmt->while_stmt.condition->binary.op) {
  case BINARY_EQ:
    jump->kind = HIR_JMP_NOT_EQUAL;
    break;
  case BINARY_NEQ:
    jump->kind = HIR_JMP_EQUAL;
    break;
  case BINARY_GT:
    jump->kind = HIR_JMP_GREATER_THAN_EQUAL;
    break;
  case BINARY_LT:
    jump->kind = HIR_JMP_LOWER_THAN_EQUAL;
    break;
  case BINARY_GTE:
    jump->kind = HIR_JMP_GREATER_THAN;
    break;
  case BINARY_LTE:
    jump->kind = HIR_JMP_LOWER_THAN;
    break;
  default:
    free(jump);
    return 1;
  }
  da_append(func->code, jump);

  da_foreach(statement_t*, it, stmt->while_stmt.body) {
    int err = HIR_lower_statement(hir, *it, func); 
    if (err)
      return 1;
  }

  HIR_instruction_t* jump_back = calloc(1, sizeof(HIR_instruction_t));
  if (!jump_back) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  jump_back->kind = HIR_JMP;
  jump_back->chunk_name = strdup(condition_chunk);
  da_append(func->code, jump_back);

  HIR_instruction_t* next_label = calloc(1, sizeof(HIR_instruction_t));
  if (!next_label) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1; 
  }
  next_label->kind = HIR_CHUNK;
  next_label->chunk_name = strdup(next_chunk);
  da_append(func->code, next_label);

  free(next_chunk);
  free(condition_chunk);
  return 0;
}

// TODO: this needs a lot of memory management to avoid leaks
int HIR_lower_if_statement(HIR_parser_t* hir,
    statement_t* stmt,
    HIR_function_t* func)
{
  int err = HIR_lower_expression(hir, stmt->if_stmt.condition, func);
  // TODO: do we have to propagate error ?
  if (err) 
    return 1;

  char* chunk = calloc(RAND_CHUNK_LEN + 2, sizeof(char));
  if (!chunk) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");  
    return 1;
  }
  hir->gen_chunk(hir->chunk_ctx, chunk);

  char* else_chunk = calloc(RAND_CHUNK_LEN + 2, sizeof(char));
  if (!else_chunk) {
    free(chunk);
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  // TODO: this is a bit ugly but whatever for now
  if (stmt->if_stmt.else_branch) {
    hir->gen_chunk(hir->chunk_ctx, else_chunk); 
  }
  else {
    free(else_chunk);
    else_chunk = NULL; 
  }

  HIR_instruction_t* jump = calloc(1, sizeof(HIR_instruction_t));
  if (!jump) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1;
  }
  if (else_chunk)
    jump->chunk_name = strdup(else_chunk);
  else
    jump->chunk_name = strdup(chunk);
  switch (stmt->if_stmt.condition->binary.op) {
  case BINARY_EQ:
    jump->kind = HIR_JMP_NOT_EQUAL;
    break;
  case BINARY_NEQ:
    jump->kind = HIR_JMP_EQUAL;
    break;
  case BINARY_GT:
    jump->kind = HIR_JMP_GREATER_THAN_EQUAL;
    break;
  case BINARY_LT:
    jump->kind = HIR_JMP_LOWER_THAN_EQUAL;
    break;
  case BINARY_GTE:
    jump->kind = HIR_JMP_GREATER_THAN;
    break;
  case BINARY_LTE:
    jump->kind = HIR_JMP_LOWER_THAN;
    break;
  default:
    free(jump);
    return 1;
  }

  da_append(func->code, jump);

  da_foreach(statement_t*, it, stmt->if_stmt.then_branch) {
    int err = HIR_lower_statement(hir, *it, func); 
    if (err)
      return 1;
  }

  if (else_chunk) {
    HIR_instruction_t* jump_else = calloc(1, sizeof(HIR_instruction_t)); 
    if (!jump_else) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }

    jump_else->kind = HIR_JMP;
    jump_else->chunk_name = strdup(chunk);
    da_append(func->code, jump_else);

    HIR_instruction_t* chunk_else_label = calloc(1, sizeof(HIR_instruction_t));
    if (!chunk_else_label) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }

    chunk_else_label->kind = HIR_CHUNK;
    chunk_else_label->chunk_name = else_chunk;
    da_append(func->code, chunk_else_label);

    da_foreach(statement_t*, it, stmt->if_stmt.else_branch) {
      int err = HIR_lower_statement(hir, *it, func);
      if (err)
       return 1; 
    }
  }

  HIR_instruction_t* chunk_label = calloc(1, sizeof(HIR_instruction_t));
  if (!chunk_label) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");  
    return 1;
  }

  chunk_label->kind = HIR_CHUNK;
  chunk_label->chunk_name = strdup(chunk);

  da_append(func->code, chunk_label);

  free(chunk);
  return 0;
}

int HIR_lower_statement(HIR_parser_t* hir, 
    statement_t* stmt,
    HIR_function_t* func)
{
  if (stmt->type == STATEMENT_EXPR) {
    return HIR_lower_expression(hir, stmt->expr_stmt.expr, func); 
  }
  if (stmt->type == STATEMENT_RETURN) {
    int res = HIR_lower_expression(hir, stmt->ret.value, func); 
    if (res != 0) {
      // Do we have to propagate error ?
      return -1; 
    }
    HIR_instruction_t* instr = calloc(1, sizeof(HIR_instruction_t));
    if (!instr) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return -1;
    }
    if (strcmp(func->name, "main") == 0) {
      instr->kind = HIR_EXIT;     
      instr->dest = func->next_temp_id; 
    }
    else {
      // TODO: what append if we return void ?
      HIR_instruction_t* return_var = calloc(1, sizeof(HIR_instruction_t));
      if (!return_var) {
        error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
        return 1;
      }
      return_var->kind = HIR_MOV;
      return_var->dest = -1;
      return_var->a = func->next_temp_id;
      da_append(func->code, return_var);

      instr->kind = HIR_RETURN;
    }
    da_append(func->code, instr);
    return 0;
  }
  if (stmt->type == STATEMENT_DECL) {
    return HIR_lower_declaration(hir, stmt->decl_stmt.decl, func);
  }
  if (stmt->type == STATEMENT_IF) {
    return HIR_lower_if_statement(hir, stmt, func); 
  }
  if (stmt->type == STATEMENT_WHILE) {
    return HIR_lower_while_statement(hir, stmt, func); 
  }
  if (stmt->type == STATEMENT_FOR) {
    return HIR_lower_for_statement(hir, stmt, func); 
  }

  error_report_general(ERROR_SEVERITY_NOT_IMPLEMENTED, 
      "unknown statement instruction");

  return 1;
}

int HIR_lower_function(HIR_parser_t* hir, 
    declaration_t* function) 
{
  if (function->type != DECLARATION_FUNC) {
    return 0;  
  }

  HIR_function_t* func = calloc(1, sizeof(HIR_function_t));
  if (!func) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return -1;
  } 

  func->name = strdup(function->func.name);
  if (!func->name) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return -1;
  } 

  func->next_temp_id = 0;
  func->stack_reserve_size = 0;

  func->code = calloc(1, sizeof(HIR_instruction_block));
  if (!func->code) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return -1;
  }

  // TODO: behavior is different for _start but whatever for now
  for (int i = 0; i < (int) function->func.params.count; ++i) {
    HIR_instruction_t* mov = calloc(1, sizeof(HIR_instruction_t)); 
    if (!mov) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return -1; 
    }
    mov->kind = HIR_MOV;
    mov->dest = func->next_temp_id;
    mov->a = -i - 1;
    da_append(func->code, mov);

    HIR_instruction_t* str = calloc(1, sizeof(HIR_instruction_t));
    if (!str) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return -1;
    }
    str->kind = HIR_STORE_VAR;
    str->var.name = strdup(function->func.params.items[i].ident_name);
    if (!str->var.name) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return -1;
    }
    str->var.is_init = 1;
    str->a = func->next_temp_id++;
    da_append(func->code, str);
  }

  da_foreach(statement_t*, it, function->func.body) {
    int lowering_result = HIR_lower_statement(hir, *it, func);
    if (lowering_result != 0) {
      // TODO: see if we have to propage error here or not
      return -1;
    }
  } 

  da_append(hir->hir_program, func);

  return 0;
}

char* HIR_generate_string_program(HIR_function_t* function) 
{
  string_builder_t sb = {0};
  sb_append_fmt(&sb, "Function %s\n", function->name);
  for (size_t i = 0; i < function->code->count; ++i) {
    HIR_instruction_t* instr = function->code->items[i];
    sb_append_fmt(&sb, "%zu: ", i);

    if (instr->kind == HIR_INT_CONST) {
      sb_append_fmt(&sb, "t%d = INT_CONST %d\n", instr->dest, instr->int_value);
      continue;
    }

    if (instr->kind == HIR_RETURN) {
      sb_append_fmt(&sb, "RETURN\n");
      continue;
    }

    if (instr->kind == HIR_EXIT) {
      sb_append_fmt(&sb, "EXIT t%d\n", instr->dest); 
      continue;
    }

    if (instr->kind == HIR_BINARY) {
      switch (instr->binary_op) {
        case HIR_BINARY_ADD: 
          sb_append_fmt(&sb, "ADD t%d t%d\n", instr->b, instr->a); 
        continue;
        case HIR_BINARY_SUB:
          sb_append_fmt(&sb, "SUB t%d t%d\n", instr->b, instr->a);          
          continue;
        case HIR_BINARY_MUL:
          sb_append_fmt(&sb, "MUL t%d t%d\n", instr->b, instr->a);          
          continue;
        case HIR_BINARY_CMP:
          sb_append_fmt(&sb, "CMP t%d t%d\n", instr->b, instr->a);
          continue;
        default:
          sb_append_fmt(&sb, "unknow binary op\n");
          continue;
      }
    }

    if (instr->kind == HIR_STORE_VAR) {
      if (instr->var.is_init) {
        sb_append_fmt(&sb, "STR slot(%s), t%d\n", instr->var.name, instr->a);
        continue; 
      }
      else {
        sb_append_fmt(&sb, "STR slot(%s), 0\n", instr->var.name);
        continue; 
      }
    }

    if (instr->kind == HIR_LOAD_VAR) {
      sb_append_fmt(&sb, "LOAD t%d, slot(%s)\n", instr->dest, instr->var.name); 
      continue;
    }

    if (instr->kind == HIR_MOV) {
      sb_append_fmt(&sb, "MOV t%d t%d\n", instr->dest, instr->a);  
      continue;
    }

    if (instr->kind == HIR_INC) {
      sb_append_fmt(&sb, "INC t%d\n", instr->dest);  
      continue;
    }

    if (instr->kind == HIR_DEC) {
      sb_append_fmt(&sb, "DEC t%d\n", instr->dest);  
      continue;
    }

    if (instr->kind == HIR_JMP_NOT_EQUAL) {
      sb_append_fmt(&sb, "JNE %s\n", instr->chunk_name); 
      continue;
    }

    if (instr->kind == HIR_JMP_EQUAL) {
      sb_append_fmt(&sb, "JE %s\n", instr->chunk_name); 
      continue;
    }

    if (instr->kind == HIR_JMP_GREATER_THAN) {
      sb_append_fmt(&sb, "JG %s\n", instr->chunk_name); 
      continue;
    }

    if (instr->kind == HIR_JMP_GREATER_THAN_EQUAL) {
      sb_append_fmt(&sb, "JGE %s\n", instr->chunk_name);
      continue; 
    }

    if (instr->kind == HIR_JMP_LOWER_THAN_EQUAL) {
      sb_append_fmt(&sb, "JLE %s\n", instr->chunk_name);
      continue; 
    }

    if (instr->kind == HIR_JMP_LOWER_THAN) {
      sb_append_fmt(&sb, "JL %s\n", instr->chunk_name);
      continue; 
    }

    if (instr->kind == HIR_CHUNK) {
      sb_append_fmt(&sb, "%s:\n", instr->chunk_name);
      continue; 
    }

    if (instr->kind == HIR_JMP) {
      sb_append_fmt(&sb, "JMP %s\n", instr->chunk_name);  
      continue;
    }

    if (instr->kind == HIR_CALL) {
      sb_append_fmt(&sb, "CALL %s\n", instr->func_name); 
      continue;
    }

    if (instr->kind == HIR_ALLOC) {
      sb_append_fmt(&sb, "ALLOC %zu\n", instr->alloc_size);
      continue;
    }
  }

  return sb.items;
}

void HIR_display_function(HIR_function_t* function) 
{
 char* string_program = HIR_generate_string_program(function); 
 puts(string_program);
}
