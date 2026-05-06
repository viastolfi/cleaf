#include "hir.h"
  
void HIR_free_instruction(HIR_instruction_t* instr) {
  switch (instr->kind) {
    case HIR_STRING_CONST:
      if (instr->string_value)
        free(instr->string_value);
      break;
    case HIR_STORE_VAR:
      if (instr->var.name)
        free(instr->var.name);
      break;
    case HIR_CALL:
      if (instr->call.callee)
        free(instr->call.callee);
      if (instr->call.args)
        free(instr->call.args);
      break;
    case HIR_LOAD_VAR:
      if (instr->var.name)
        free(instr->var.name);
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

  if (func->params) 
    free(func->params);

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
    error_report_general(ERROR_SEVERITY_ERROR, "awaiting var declaration, getting somethign else");
    return -1;
  }

  HIR_instruction_t* instr = calloc(1, sizeof(HIR_instruction_t));
  if (!instr) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return -1;  
  }

  instr->kind = HIR_STORE_VAR;
  instr->var.name = strdup(decl->var_decl.ident.name);
  if (!instr->var.name) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return -1;
  }

  if (decl->var_decl.init) {
    HIR_lower_expression(hir, decl->var_decl.init, func);   
    instr->var.is_init = 1;
    instr->a = func->next_temp_id;
  } else {
    instr->var.is_init = 0; 
  }

  da_append(func->code, instr);
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

  return 1;
}

int HIR_lower_binary_expression(expression_t* expr,
    HIR_parser_t* hir,
    HIR_instruction_t* instr,
    HIR_function_t* func)
{
  switch(expr->binary.op) {
    case BINARY_PLUS:
      instr->binary_op = HIR_BINARY_ADD;
      break;
    case BINARY_MINUS:
      instr->binary_op = HIR_BINARY_MINUS;
      break;
    case BINARY_MUL:
      instr->binary_op = HIR_BINARY_MUL;
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

  instr->dest = ++(func->next_temp_id);
  
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
    HIR_lower_unary_expression(hir, expr, func);
  }

  return 1;
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
    instr->kind = HIR_RETURN;
    instr->dest = func->next_temp_id; 
    da_append(func->code, instr);
    return 0;
  }
  if (stmt->type == STATEMENT_DECL) {
    return HIR_lower_declaration(hir, stmt->decl_stmt.decl, func);
  }

  return 1;
}

int HIR_lower_function(HIR_parser_t* hir, 
    declaration_t* function) 
{
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

  func->return_type = function->func.return_type;

  func->params = calloc(function->func.params.count, 
      sizeof(typed_identifier_t));
  if (!func->params) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return -1;
  }
  func->param_count = function->func.params.count; 
  for (size_t i = 0; i < func->param_count; ++i) 
    func->params[i] = function->func.params.items[i];

  func->next_temp_id = 0;

  func->code = calloc(1, sizeof(HIR_instruction_block));
  if (!func->code) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return -1;
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
      sb_append_fmt(&sb, "RETURN t%d\n", instr->dest);
      continue;
    }

    if (instr->kind == HIR_BINARY) {
      switch (instr->binary_op) {
        case HIR_BINARY_ADD: 
          sb_append_fmt(&sb, "t%d = ADD t%d t%d\n", instr->dest, instr->a, instr->b); 
        continue;
        case HIR_BINARY_MINUS:
          sb_append_fmt(&sb, "t%d = MIN t%d t%d\n", instr->dest, instr->a, instr->b);          
          continue;
        case HIR_BINARY_MUL:
          sb_append_fmt(&sb, "t%d = MUL t%d t%d\n", instr->dest, instr->a, instr->b);          
          continue;
        default:
          sb_append_fmt(&sb, "unknow binary op\n");
          continue;
      }
    }

    if (instr->kind == HIR_STORE_VAR) {
      if (instr->var.is_init)
        sb_append_fmt(&sb, "STR slot(%s), t%d\n", instr->var.name, instr->a);
      else
        sb_append_fmt(&sb, "STR slot(%s), 0\n", instr->var.name);
    }

    if (instr->kind == HIR_LOAD_VAR) {
      sb_append_fmt(&sb, "LOAD t%d, slot(%s)\n", instr->dest, instr->var.name); 
    }

    if (instr->kind == HIR_MOV) {
      sb_append_fmt(&sb, "MOV t%d t%d\n", instr->dest, instr->a);  
    }

    if (instr->kind == HIR_INC) {
      sb_append_fmt(&sb, "INC t%d\n", instr->dest);  
    }

    if (instr->kind == HIR_DEC) {
      sb_append_fmt(&sb, "DEC t%d\n", instr->dest);  
    }
  }

  return sb.items;
}

void HIR_display_function(HIR_function_t* function) 
{
 char* string_program = HIR_generate_string_program(function); 
 puts(string_program);
}
