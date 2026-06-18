#include "hir.h"

static size_t min(size_t a, size_t b) {
  return a < b ? a : b;
}
  
void IR_free_instruction(IR_instruction_t* instr) {
  switch (instr->kind) {
    case IR_STORE_VAR:
      if (instr->var.name)
        free(instr->var.name);
      break;
    case IR_LOAD_VAR:
      if (instr->var.name)
        free(instr->var.name);
      break;
    case IR_JMP_NOT_EQUAL:
    case IR_JMP:
    case IR_JMP_EQUAL:
    case IR_JMP_GREATER_THAN_EQUAL:
    case IR_JMP_GREATER_THAN:
    case IR_JMP_LOWER_THAN_EQUAL:
    case IR_JMP_LOWER_THAN:
      if (instr->chunk_name)
        free(instr->chunk_name);
      break;
    case IR_CHUNK:
      if (instr->chunk_name)
        free(instr->chunk_name);
      break;
    case IR_CALL:
      if (instr->func_name)
        free(instr->func_name);
      break;
    case IR_ASM:
      for (size_t i = 0; i < instr->asm_data.string_count; i++)
        free(instr->asm_data.strings[i]);
      free(instr->asm_data.strings);
      free(instr->asm_data.args);
      break;
    default:
      break;
  }
  free(instr);
}

void IR_free_function(IR_function_t* func) 
{
  if (func->name)
    free(func->name);
    
  if (func->code) {
    da_foreach(IR_instruction_t*, it, func->code) 
      IR_free_instruction(*it); 
    free(func->code->items);
    free(func->code);
  }

  free(func);
}

int IR_lower_declaration(
    HIR_parser_t* hir,
    declaration_t* decl,
    IR_function_t* func)
{
  (void)hir;
  if (decl->type != DECLARATION_VAR) {
    error_report_general(ERROR_SEVERITY_ERROR, 
        "awaiting var declaration, getting something else");
    return -1;
  }

  IR_instruction_t* instr = calloc(1, sizeof(IR_instruction_t));
  if (!instr) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return -1;  
  }

  instr->kind = IR_STORE_VAR;
  instr->var.name = strdup(decl->var_decl.ident.ident_name);
  instr->src.size = decl->var_decl.ident.type.element_size;
  if (!instr->var.name) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return -1;
  }

  if (decl->var_decl.ident.type.kind == TYPE_CUSTOM) {
      IR_instruction_t* alloc = 
        calloc(1, sizeof(IR_instruction_t));   
      if (!alloc) {
        error_report_general(
            ERROR_SEVERITY_ERROR, "out of memory"); 
        return -1;
      }
      alloc->kind = IR_ALLOC;
      alloc->alloc_size = decl->var_decl.ident.type.element_size;

      da_append(func->code, alloc);
      instr->var.is_init = 1;
      instr->src.id = -1;

      da_append(func->code, instr);
      if (decl->var_decl.init && 
          decl->var_decl.init->composite_literal.is_initializer) {
        int err = 
          IR_lower_composite_literal_expression(hir, decl, func);
        if (err)
          return 1;
      }
      func->stack_reserve_size += 8;
  } 
  else if (decl->var_decl.ident.type.array_len > 0) {
    IR_instruction_t* alloc = 
      calloc(1, sizeof(IR_instruction_t));
    if (!alloc) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return -1;
    }
    alloc->kind = IR_ALLOC;
    alloc->alloc_size = decl->var_decl.ident.type.size;
    da_append(func->code, alloc);

    instr->var.is_init = 1;
    instr->src.id = -1;

    da_append(func->code, instr);
    if (decl->var_decl.init && 
        decl->var_decl.init->composite_literal.is_initializer) {
      int err = 
        IR_lower_composite_literal_expression(hir, decl, func);
      if (err)
        return 1;
    }
    func->stack_reserve_size += 8;
  }
  else {
    if (decl->var_decl.init) {
      instr->var.is_init = 1;
      IR_lower_expression(hir, decl->var_decl.init, func);   
      instr->src.id = func->next_temp_id;
    } else {
      if (decl->var_decl.ident.type.kind == TYPE_INT) {
        instr->var.is_init = 0; 
      }
    } 
    da_append(func->code, instr);
    func->stack_reserve_size += decl->var_decl.ident.type.element_size;
  }

  return 0;
}

int IR_lower_composite_literal_expression(
    HIR_parser_t* hir, 
    declaration_t* decl, 
    IR_function_t* func)
{
  IR_instruction_t* load = calloc(1, sizeof(IR_instruction_t));
  if (!load) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  load->kind = IR_LOAD_VAR;
  load->dest.id = func->next_temp_id;
  // we store a pointer (64bits) so we can hardcode the size here
  load->dest.size = 8;
  load->var.is_init = 1;
  load->var.name = strdup(decl->var_decl.ident.ident_name);
  if (!load->var.name) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1;
  }

  da_append(func->code, load);

  struct_symbol_t* sym = hashmap_get(hir->struct_symbols,
      decl->var_decl.ident.type.name);
  expression_t* e = decl->var_decl.init;

  int save = func->next_temp_id;

  for (size_t i = 0; i < e->composite_literal.count; ++i) {
    expression_t* expr;
    if (e->composite_literal.values[i]->type == EXPRESSION_ASSIGN) 
      expr = e->composite_literal.values[i]->assign.rhs;
    else 
      expr = e->composite_literal.values[i];

    IR_lower_expression(hir, expr, func);
    size_t computed_place = 0;
    IR_instruction_t* mov_offset =
      calloc(1, sizeof(IR_instruction_t));
    if (!mov_offset) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }
    mov_offset->kind = IR_MOV_OFFSET;
    mov_offset->offset.timing = IR_PRE_OFFSET;
    mov_offset->dest.id = save;
    // data come from pointer so we can hardcode it here
    // this would definitely crash in 32 bits architecture
    mov_offset->dest.size = 8;
    mov_offset->src.id = func->next_temp_id;

    if (e->composite_literal.values[i]->type == 
        EXPRESSION_ASSIGN) {
      size_t j = 0;
      for (; j < sym->members_count; ++j) {
        expression_t* comparator = 
          e->composite_literal.values[i]->assign.lhs;
        if (strcmp(
              comparator->var.ident.ident_name,
              sym->members_name[j]) == 0) {
          break ; 
        } else {
          computed_place += sym->members_type[j].type.element_size;
        }
      } 
      mov_offset->src.size = 
        sym->members_type[j].type.element_size;

      mov_offset->offset.size = computed_place;
      da_append(func->code, mov_offset);
    }
    else {
      mov_offset->src.size = 
        decl->var_decl.ident.type.element_size; 
      mov_offset->offset.size = 
        decl->var_decl.ident.type.element_size * i;
      da_append(func->code, mov_offset);
    }
  }

  return 0;
}

int IR_lower_unary_expression(HIR_parser_t* hir,
    expression_t* expr,
    IR_function_t* func)
{
  (void)hir;
  size_t operand_size = 
    expr->unary.operand->var.ident.type.element_size;

  if (expr->unary.op == UNARY_POST_INC ||
      expr->unary.op == UNARY_POST_DEC) {
    IR_instruction_t* load = calloc(1, sizeof(IR_instruction_t)); 
    if (!load) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }

    load->kind = IR_LOAD_VAR;
    load->var.name = strdup(expr->unary.operand->var.ident.ident_name);
    load->dest.id = ++(func->next_temp_id);
    load->dest.size = operand_size;
    if (!load->var.name) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }
    da_append(func->code, load);  

    IR_instruction_t* mov = calloc(1, sizeof(IR_instruction_t));
    if (!mov) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");  
      return 1;
    }
    mov->kind = IR_MOV;
    mov->dest.id = func->next_temp_id + 1;
    mov->dest.size = operand_size;
    mov->src.id = func->next_temp_id;
    mov->src.size = operand_size;
    da_append(func->code, mov);

    IR_instruction_t* op = calloc(1, sizeof(IR_instruction_t));
    if (!op) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");  
      return 1;
    }
    switch (expr->unary.op) {
      case UNARY_POST_INC: op->kind = IR_INC; break;
      case UNARY_POST_DEC: op->kind = IR_DEC; break;
      default: break;
    }

    op->dest.id = func->next_temp_id;
    op->dest.size = operand_size;
    da_append(func->code, op);

    IR_instruction_t* str = calloc(1, sizeof(IR_instruction_t));
    if (!str) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }
    str->kind = IR_STORE_VAR;
    str->src.id = func->next_temp_id++;
    str->src.size = operand_size;
    str->var.name = strdup(expr->unary.operand->var.ident.ident_name);
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
    IR_instruction_t* load = calloc(1, sizeof(IR_instruction_t)); 
    if (!load) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return 1;   
    }
    load->kind = IR_LOAD_VAR;
    load->dest.id = ++(func->next_temp_id);
    load->dest.size = operand_size;
    load->var.name = strdup(expr->unary.operand->var.ident.ident_name);
    if (!load->var.name) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }
    da_append(func->code, load);

    IR_instruction_t* op = calloc(1, sizeof(IR_instruction_t));
    if (!op) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }
    switch (expr->unary.op) {
      case UNARY_PRE_INC: op->kind = IR_INC; break;
      case UNARY_PRE_DEC: op->kind = IR_DEC; break;
      default: break;  
    }
    op->dest.id = func->next_temp_id;
    op->dest.size = operand_size;
    da_append(func->code, op);

    IR_instruction_t* str = calloc(1, sizeof(IR_instruction_t));
    if (!str) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }
    str->kind = IR_STORE_VAR;
    str->src.id = func->next_temp_id;
    str->src.size = operand_size;
    str->var.name = strdup(expr->unary.operand->var.ident.ident_name);
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

int IR_lower_lvalue(HIR_parser_t* hir,
    expression_t* expr,
    IR_function_t* func,
    lvalue_t* lv)
{
  if (expr->type == EXPRESSION_VAR) {
    lv->kind = LVALUE_VAR;
    lv->var_name = expr->var.ident.ident_name;
    lv->elem_size = expr->var.ident.type.element_size;
    return 0;
  }

  if (expr->type == EXPRESSION_INDEX) {
    size_t elem_size = expr->index.base->var.ident.type.element_size;

    if (IR_lower_expression(hir, expr->index.base, func) != 0)
      return 1;
    int base_id = func->next_temp_id;

    if (IR_lower_expression(hir, expr->index.index, func) != 0)
      return 1;
    int idx_id = func->next_temp_id;

    IR_instruction_t* mul = calloc(1, sizeof(IR_instruction_t));
    if (!mul) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return 1;
    }
    mul->kind = IR_DIRECT_MUL;
    mul->dest.id = func->next_temp_id;
    mul->dest.size = elem_size;
    mul->int_value = elem_size;
    da_append(func->code, mul);

    lv->kind = LVALUE_ELEM;
    lv->base_id = base_id;
    lv->idx_id = idx_id;
    lv->elem_size = elem_size;
    return 0;
  }

  error_report_general(ERROR_SEVERITY_NOT_IMPLEMENTED,
      "unsupported lvalue expression");
  return 1;
}

int IR_lower_index_expression(
    HIR_parser_t* hir, expression_t* expr, IR_function_t* func)
{
  lvalue_t lv = {0};
  if (IR_lower_lvalue(hir, expr, func, &lv) != 0)
    return 1;

  IR_instruction_t* load = calloc(1, sizeof(IR_instruction_t));
  if (!load) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1;
  }
  load->kind = IR_LOAD_ELEM;
  load->src.id = lv.base_id;
  load->src.size = 8;
  load->index.id = lv.idx_id;
  load->index.size = 8;
  load->dest.id = ++(func->next_temp_id);
  load->dest.size = lv.elem_size;
  da_append(func->code, load);

  return 0;
}

int IR_lower_call_expression(
    HIR_parser_t* hir, expression_t* expr, IR_function_t* func)
{
  for (int i = 0; i < (int) expr->call.arg_count; ++i) {
    IR_instruction_t* set_arg = calloc(1, sizeof(IR_instruction_t)); 
    if (!set_arg) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }
    set_arg->kind = IR_MOV;
    set_arg->dest.id = -i - 1;

    if (IR_lower_expression(hir, expr->call.args[i], func) != 0)
      return 1;

    set_arg->src.id = func->next_temp_id;
    
    da_append(func->code, set_arg);
  } 

  IR_instruction_t* call = calloc(1, sizeof(IR_instruction_t));
  if (!call) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  call->kind = IR_CALL;
  call->func_name = strdup(expr->call.callee);
  if (!call->func_name) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  da_append(func->code, call);

  IR_instruction_t* result = calloc(1, sizeof(IR_instruction_t));
  if (!result) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1;
  }
  result->kind = IR_MOV;
  result->dest.id = ++(func->next_temp_id);
  result->src.id = -1;
  da_append(func->code, result);

  return 0;
}

int IR_lower_binary_expression(expression_t* expr,
    HIR_parser_t* hir,
    IR_instruction_t* instr,
    IR_function_t* func)
{
  switch(expr->binary.op) {
    case BINARY_ADD:
      instr->binary_op = IR_BINARY_ADD;
      break;
    case BINARY_SUB:
      instr->binary_op = IR_BINARY_SUB;
      break;
    case BINARY_MUL:
      instr->binary_op = IR_BINARY_MUL;
      break;
    case BINARY_EQ:
    case BINARY_NEQ:
    case BINARY_GT:
    case BINARY_GTE:
    case BINARY_LT:
    case BINARY_LTE:
      instr->binary_op = IR_BINARY_CMP;
      break;
    default:
      return 1;
  }

  if (IR_lower_expression(hir, expr->binary.left, func) != 0)
    return -1;
  instr->src.id = func->next_temp_id;
  instr->src.size = func->code->items[func->code->count - 1]->dest.size;

  if (IR_lower_expression(hir, expr->binary.right, func) != 0)
    return -1;
  instr->dest.id = func->next_temp_id;
  instr->dest.size = func->code->items[func->code->count - 1]->dest.size;

  // promote register to the biggest one to avoid nasm compiler error
  if (instr->src.size != instr->dest.size) {
    size_t s = min(instr->src.size, instr->dest.size); 
    instr->src.size = s;
    instr->dest.size = s;
  }

  return 0;
}

int IR_lower_expr_int_lit(HIR_parser_t* hir,
    expression_t* expr,
    IR_function_t* func)
{
  (void)hir;
  IR_instruction_t* instr = calloc(1, sizeof(IR_instruction_t));
  if (!instr) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return -1;
  }
  instr->kind = IR_INT_CONST;
  instr->dest.id = ++(func->next_temp_id);
  instr->int_value = expr->int_lit.value;
  da_append(func->code, instr);
  return 0;
}

int IR_lower_expr_char_lit(HIR_parser_t* hir,
    expression_t* expr,
    IR_function_t* func)
{
  (void)hir;
  IR_instruction_t* instr = calloc(1, sizeof(IR_instruction_t));
  if (!instr) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return -1;
  }
  instr->kind = IR_INT_CONST;
  instr->dest.id = ++(func->next_temp_id);
  instr->int_value = expr->char_lit.value;
  da_append(func->code, instr);
  return 0;
}

int IR_lower_expr_var(HIR_parser_t* hir,
    expression_t* expr,
    IR_function_t* func)
{
  IR_instruction_t* instr = calloc(1, sizeof(IR_instruction_t));
  if (!instr) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return -1;
  }
  instr->kind = IR_LOAD_VAR;
  instr->dest.id = ++(func->next_temp_id);

  if (expr->var.ident.type.array_len > 0 ||
      expr->var.ident.type.kind == TYPE_CUSTOM) {
    instr->dest.size = 8; 
  } else {
    instr->dest.size = expr->var.ident.type.element_size;
  }

  instr->var.name = strdup(expr->var.ident.ident_name);
  if (!instr->var.name) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return -1;
  }
  da_append(func->code, instr);

  while (expr->var.member) {
    IR_instruction_t* offset_instr = calloc(1, sizeof(IR_instruction_t));
    if (!offset_instr) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return -1;
    }
    offset_instr->kind = IR_MOV_OFFSET;
    offset_instr->offset.timing = IR_POST_OFFSET;

    // sym should never be NULL after semantic
    // hence, we don't check and error report this but this is important to keep in mind in case it segfaults here
    struct_symbol_t* sym =
      hashmap_get(hir->struct_symbols, expr->var.ident.type.name);

    size_t offset = 0;
    for (size_t i = 0; i < sym->members_count; ++i) {
      if (strcmp(
            expr->var.member->var.ident.ident_name,
            sym->members_name[i]) == 0) {
          goto insert_member;
      }
      offset += sym->members_type[i].type.element_size;
    }

insert_member:
    offset_instr->offset.size = offset;
    offset_instr->src.id = func->next_temp_id;
    offset_instr->src.size = expr->var.ident.type.element_size;
    offset_instr->dest.id = ++func->next_temp_id;
    offset_instr->dest.size = expr->var.member->var.ident.type.element_size;
    da_append(func->code, offset_instr);

    expr = expr->var.member;
  }

  return 0;
}

// TODO: make sure no error can still occurs here even after semantic analysis
int IR_lower_expr_assign(HIR_parser_t* hir,
    expression_t* expr,
    IR_function_t* func)
{
  if (IR_lower_expression(hir, expr->assign.rhs, func) != 0)
    return 1;
  int rhs_temp = func->next_temp_id;

  lvalue_t lv = {0};
  if (IR_lower_lvalue(hir, expr->assign.lhs, func, &lv) != 0)
    return 1;

  IR_instruction_t* store = calloc(1, sizeof(IR_instruction_t));
  if (!store) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1;
  }

  if (lv.kind == LVALUE_VAR) {
    store->kind = IR_STORE_VAR;
    store->src.id = rhs_temp;
    store->src.size = lv.elem_size;
    store->var.name = strdup(lv.var_name);
    store->var.is_init = 1;
    if (!store->var.name) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      free(store);
      return 1;
    }
  } else {
    store->kind = IR_STORE_ELEM;
    store->src.id = rhs_temp;
    store->src.size = lv.elem_size;
    store->dest.id = lv.base_id;
    store->dest.size = 8;
    store->index.id = lv.idx_id;
    store->index.size = 8;
  }

  da_append(func->code, store);
  return 0;
}

int IR_lower_expression(HIR_parser_t* hir,
    expression_t* expr,
    IR_function_t* func)
{
  if (expr->type == EXPRESSION_INT_LIT)
    return IR_lower_expr_int_lit(hir, expr, func);

  if (expr->type == EXPRESSION_CHAR_LIT)
    return IR_lower_expr_char_lit(hir, expr, func);

  if (expr->type == EXPRESSION_VAR)
    return IR_lower_expr_var(hir, expr, func);

  if (expr->type == EXPRESSION_BINARY) {
    IR_instruction_t* instr = calloc(1, sizeof(IR_instruction_t));
    if (!instr) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return -1;
    }
    if (IR_lower_binary_expression(expr, hir, instr, func) != 0) {
      error_report_at_position(hir->error_ctx,
          expr->source_pos,
          ERROR_SEVERITY_ERROR,
          "error while lowering binary expression");
      return 1;
    }
    instr->kind = IR_BINARY;
    da_append(func->code, instr);
    return 0;
  }

  if (expr->type == EXPRESSION_ASSIGN)
    return IR_lower_expr_assign(hir, expr, func);

  if (expr->type == EXPRESSION_UNARY)
    return IR_lower_unary_expression(hir, expr, func);

  if (expr->type == EXPRESSION_CALL)
    return IR_lower_call_expression(hir, expr, func);

  if (expr->type == EXPRESSION_INDEX)
    return IR_lower_index_expression(hir, expr, func);

  return 1;
}

int IR_lower_free_statement(
    HIR_parser_t* hir,
    statement_t* stmt,
    IR_function_t* func)
{
  IR_instruction_t* instr = calloc(1, sizeof(IR_instruction_t));
  if (!instr) {
    error_report_general(ERROR_SEVERITY_ERROR, "out if memory"); 
    return 1;
  }

  instr->kind = IR_DEALLOC;
  
  IR_lower_expression(hir, stmt->free_stmt.expr, func); 

  instr->src.id = func->next_temp_id;
  instr->src.size = stmt->free_stmt.expr->var.ident.type.element_size;

  da_append(func->code, instr);
  return 0;
}

int IR_lower_asm_statement(
     HIR_parser_t* hir,
     statement_t* stmt,
     IR_function_t* func)
{
  IR_temp_id* temps = NULL;
  if (stmt->asm_stmt.arg_count > 0) {
    temps = calloc(stmt->asm_stmt.arg_count, sizeof(IR_temp_id));
    if (!temps) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return 1;
    }
    for (size_t i = 0; i < stmt->asm_stmt.arg_count; i++) {
      if (IR_lower_expression(
            hir, stmt->asm_stmt.args[i], func) != 0) {
        free(temps);
        return 1;
      }
      temps[i].id = func->next_temp_id;
      temps[i].size = 
        func->code->items[func->code->count - 1]->dest.size;
    }
  }

  IR_instruction_t* instr = calloc(1, sizeof(IR_instruction_t));
  if (!instr) {
    free(temps);
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1;
  }

  instr->asm_data.strings = 
    calloc(stmt->asm_stmt.instr_count, sizeof(char*));
  if (!instr->asm_data.strings) {
    free(temps);
    free(instr);
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1;
  }

  instr->kind = IR_ASM;
  instr->asm_data.string_count = stmt->asm_stmt.instr_count;
  instr->asm_data.arg_count = stmt->asm_stmt.arg_count;
  instr->asm_data.args = temps;

  for (size_t i = 0; i < stmt->asm_stmt.instr_count; i++) {
    instr->asm_data.strings[i] = strdup(stmt->asm_stmt.instr[i]);
    if (!instr->asm_data.strings[i]) {
      for (size_t j = 0; j < i; j++)
        free(instr->asm_data.strings[j]);
      free(instr->asm_data.strings);
      free(temps);
      free(instr);
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return 1;
    }
  }

  da_append(func->code, instr);
  return 0;
}

int IR_lower_for_statement(
    HIR_parser_t* hir,
    statement_t* stmt, 
    IR_function_t* func)
{
  int err = 0;
  if (stmt->for_stmt.init_kind == FOR_INIT_DECL) 
    err = IR_lower_declaration(hir, stmt->for_stmt.decl_init, func);
  else
    err = IR_lower_expression(hir, stmt->for_stmt.expr_init, func);
  if (err)
    return 1;

  char* main_chunk = calloc(2 + RAND_CHUNK_LEN, sizeof(char));
  if (!main_chunk) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  hir->gen_chunk(hir->chunk_ctx, main_chunk);

  IR_instruction_t* main_label = calloc(1, sizeof(IR_instruction_t));
  if (!main_label) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  main_label->kind = IR_CHUNK;
  main_label->chunk_name = strdup(main_chunk);
  da_append(func->code, main_label);

  da_foreach(statement_t*, it, stmt->for_stmt.body) {
    int err = IR_lower_statement(hir, *it, func);
    if (err)
     return 1; 
  }

  if (IR_lower_expression(hir, stmt->for_stmt.loop, func) != 0)
   return 1; 

  if (IR_lower_expression(hir, stmt->for_stmt.condition, func) != 0)
    return 1;

  IR_instruction_t* jump = calloc(1, sizeof(IR_instruction_t));
  if (!jump) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1;
  }
  jump->chunk_name = strdup(main_chunk);
  switch (stmt->for_stmt.condition->binary.op) {
  case BINARY_EQ:
    jump->kind = IR_JMP_EQUAL;
    break;
  case BINARY_NEQ:
    jump->kind = IR_JMP_NOT_EQUAL;
    break;
  case BINARY_GT:
    jump->kind = IR_JMP_GREATER_THAN;
    break;
  case BINARY_LT:
    jump->kind = IR_JMP_LOWER_THAN;
    break;
  case BINARY_GTE:
    jump->kind = IR_JMP_GREATER_THAN_EQUAL;
    break;
  case BINARY_LTE:
    jump->kind = IR_JMP_LOWER_THAN_EQUAL;
    break;
  default:
    free(jump);
    return 1;
  }
  da_append(func->code, jump);

  free(main_chunk);
  return 0;
}

int IR_lower_while_statement(
    HIR_parser_t* hir,
    statement_t* stmt,
    IR_function_t* func)
{
  char* condition_chunk = calloc(2 + RAND_CHUNK_LEN, sizeof(char));
  if (!condition_chunk) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1; 
  }
  hir->gen_chunk(hir->chunk_ctx, condition_chunk);

  IR_instruction_t* condition_label = calloc(1, sizeof(IR_instruction_t));
  if (!condition_label) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  condition_label->kind = IR_CHUNK;
  condition_label->chunk_name = strdup(condition_chunk);
  da_append(func->code, condition_label);

  if (IR_lower_expression(hir, stmt->while_stmt.condition, func) != 0)
    return 1;

  char* next_chunk = calloc(2 + RAND_CHUNK_LEN, sizeof(char));
  if (!next_chunk) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  hir->gen_chunk(hir->chunk_ctx, next_chunk);

  IR_instruction_t* jump = calloc(1, sizeof(IR_instruction_t));
  if (!jump) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1;
  }
  jump->chunk_name = strdup(next_chunk);
  switch (stmt->while_stmt.condition->binary.op) {
  case BINARY_EQ:
    jump->kind = IR_JMP_NOT_EQUAL;
    break;
  case BINARY_NEQ:
    jump->kind = IR_JMP_EQUAL;
    break;
  case BINARY_GT:
    jump->kind = IR_JMP_GREATER_THAN_EQUAL;
    break;
  case BINARY_LT:
    jump->kind = IR_JMP_LOWER_THAN_EQUAL;
    break;
  case BINARY_GTE:
    jump->kind = IR_JMP_GREATER_THAN;
    break;
  case BINARY_LTE:
    jump->kind = IR_JMP_LOWER_THAN;
    break;
  default:
    free(jump);
    return 1;
  }
  da_append(func->code, jump);

  da_foreach(statement_t*, it, stmt->while_stmt.body) {
    int err = IR_lower_statement(hir, *it, func); 
    if (err)
      return 1;
  }

  IR_instruction_t* jump_back = calloc(1, sizeof(IR_instruction_t));
  if (!jump_back) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
    return 1;
  }
  jump_back->kind = IR_JMP;
  jump_back->chunk_name = strdup(condition_chunk);
  da_append(func->code, jump_back);

  IR_instruction_t* next_label = calloc(1, sizeof(IR_instruction_t));
  if (!next_label) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return 1; 
  }
  next_label->kind = IR_CHUNK;
  next_label->chunk_name = strdup(next_chunk);
  da_append(func->code, next_label);

  free(next_chunk);
  free(condition_chunk);
  return 0;
}

// TODO: this needs src.id lot of memory management to avoid leaks
int IR_lower_if_statement(HIR_parser_t* hir,
    statement_t* stmt,
    IR_function_t* func)
{
  int err = IR_lower_expression(hir, stmt->if_stmt.condition, func);
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
  // TODO: this is src.id bit ugly but whatever for now
  if (stmt->if_stmt.else_branch) {
    hir->gen_chunk(hir->chunk_ctx, else_chunk); 
  }
  else {
    free(else_chunk);
    else_chunk = NULL; 
  }

  IR_instruction_t* jump = calloc(1, sizeof(IR_instruction_t));
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
    jump->kind = IR_JMP_NOT_EQUAL;
    break;
  case BINARY_NEQ:
    jump->kind = IR_JMP_EQUAL;
    break;
  case BINARY_GT:
    jump->kind = IR_JMP_GREATER_THAN_EQUAL;
    break;
  case BINARY_LT:
    jump->kind = IR_JMP_LOWER_THAN_EQUAL;
    break;
  case BINARY_GTE:
    jump->kind = IR_JMP_GREATER_THAN;
    break;
  case BINARY_LTE:
    jump->kind = IR_JMP_LOWER_THAN;
    break;
  default:
    free(jump);
    return 1;
  }

  da_append(func->code, jump);

  da_foreach(statement_t*, it, stmt->if_stmt.then_branch) {
    int err = IR_lower_statement(hir, *it, func); 
    if (err)
      return 1;
  }

  if (else_chunk) {
    IR_instruction_t* jump_else = calloc(1, sizeof(IR_instruction_t)); 
    if (!jump_else) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }

    jump_else->kind = IR_JMP;
    jump_else->chunk_name = strdup(chunk);
    da_append(func->code, jump_else);

    IR_instruction_t* chunk_else_label = calloc(1, sizeof(IR_instruction_t));
    if (!chunk_else_label) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return 1;
    }

    chunk_else_label->kind = IR_CHUNK;
    chunk_else_label->chunk_name = else_chunk;
    da_append(func->code, chunk_else_label);

    da_foreach(statement_t*, it, stmt->if_stmt.else_branch) {
      int err = IR_lower_statement(hir, *it, func);
      if (err)
       return 1; 
    }
  }

  IR_instruction_t* chunk_label = calloc(1, sizeof(IR_instruction_t));
  if (!chunk_label) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");  
    return 1;
  }

  chunk_label->kind = IR_CHUNK;
  chunk_label->chunk_name = strdup(chunk);

  da_append(func->code, chunk_label);

  free(chunk);
  return 0;
}

int IR_lower_return_statement(HIR_parser_t* hir,
    statement_t* stmt,
    IR_function_t* func)
{
  int res = IR_lower_expression(hir, stmt->ret.value, func);
  if (res != 0)
    return -1;

  IR_instruction_t* instr = calloc(1, sizeof(IR_instruction_t));
  if (!instr) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return -1;
  }

  if (strcmp(func->name, "main") == 0) {
    instr->kind = IR_EXIT;
    instr->dest.id = func->next_temp_id;
    instr->dest.size = func->code->items[func->code->count - 1]->dest.size;
  }
  else {
    // TODO: what append if we return void ?
    IR_instruction_t* return_var = calloc(1, sizeof(IR_instruction_t));
    if (!return_var) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return 1;
    }
    return_var->kind = IR_MOV;
    return_var->dest.id = -1;
    return_var->src.id = func->next_temp_id;
    return_var->src.size = func->code->items[func->code->count - 1]->dest.size;

    if (return_var->src.size != return_var->dest.size) {
      size_t s = min(return_var->src.size, return_var->dest.size);
      return_var->src.size = s;
      return_var->dest.size = s;
    }

    da_append(func->code, return_var);
    instr->kind = IR_RETURN;
  }

  da_append(func->code, instr);
  return 0;
}

int IR_lower_statement(HIR_parser_t* hir, 
    statement_t* stmt,
    IR_function_t* func)
{
  if (stmt->type == STATEMENT_EXPR)
    return IR_lower_expression(hir, stmt->expr_stmt.expr, func);

  if (stmt->type == STATEMENT_RETURN)
    return IR_lower_return_statement(hir, stmt, func);

  if (stmt->type == STATEMENT_DECL)
    return IR_lower_declaration(hir, stmt->decl_stmt.decl, func);

  if (stmt->type == STATEMENT_IF)
    return IR_lower_if_statement(hir, stmt, func);

  if (stmt->type == STATEMENT_WHILE)
    return IR_lower_while_statement(hir, stmt, func);

  if (stmt->type == STATEMENT_FOR)
    return IR_lower_for_statement(hir, stmt, func);

  if (stmt->type == STATEMENT_ASM)
    return IR_lower_asm_statement(hir, stmt, func);

  if (stmt->type == STATEMENT_FREE)
    return IR_lower_free_statement(hir, stmt, func);

  error_report_general(ERROR_SEVERITY_NOT_IMPLEMENTED,
      "unknown statement instruction");

  return 1;
}

static int IR_lower_function_params(IR_function_t* func,
    declaration_t* function)
{
  for (int i = 0; i < (int) function->func.params.count; ++i) {
    IR_instruction_t* mov = calloc(1, sizeof(IR_instruction_t));
    if (!mov) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return -1;
    }
    mov->kind = IR_MOV;
    mov->dest.id = func->next_temp_id;
    mov->dest.size = function->func.params.items[i].type.size;
    mov->src.id = -i - 1;

    // This is done here and in the function return handling
    // We do this since we always store return values in `rax`
    // This might be pretty poor design and lead to bugs in the future
    // TODO: fin a better way to handle function param and return value handling
    if (mov->dest.size != mov->src.size) {
      size_t s = min(mov->dest.size, mov->src.size);
      mov->dest.size = s;
      mov->src.size  = s;
    }

    da_append(func->code, mov);

    IR_instruction_t* str = calloc(1, sizeof(IR_instruction_t));
    if (!str) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return -1;
    }
    str->kind = IR_STORE_VAR;
    str->var.name = strdup(function->func.params.items[i].ident_name);
    if (!str->var.name) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return -1;
    }
    str->var.is_init = 1;
    str->src.id = func->next_temp_id++;
    str->src.size = function->func.params.items[i].type.element_size;

    da_append(func->code, str);
  }

  return 0;
}

int IR_lower_function(HIR_parser_t* hir, 
    declaration_t* function) 
{
  if (function->type != DECLARATION_FUNC)
    return 0;

  IR_function_t* func = calloc(1, sizeof(IR_function_t));
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

  func->code = calloc(1, sizeof(IR_instruction_block));
  if (!func->code) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return -1;
  }

  // TODO: behavior is different for _start but whatever for now
  if (IR_lower_function_params(func, function) != 0)
    return -1;

  da_foreach(statement_t*, it, function->func.body) {
    int lowering_result = IR_lower_statement(hir, *it, func);
    if (lowering_result != 0) {
      // TODO: see if we have to propage error here or not
      return -1;
    }
  }

  da_append(hir->hir_program, func);

  return 0;
}

static char IR_temp_letter(size_t size) {
  switch (size) {
    case 1: return 'b';
    case 2: return 'w';
    case 4: return 'd';
    case 8: return 'q';
    default: return 't';
  }
}

#define TEMP_STR(t) IR_temp_letter((t).size), (t).id

char* IR_generate_string_program(IR_function_t* function) 
{
  string_builder_t sb = {0};
  sb_append_fmt(&sb, "Function %s\n", function->name);
  for (size_t i = 0; i < function->code->count; ++i) {
    IR_instruction_t* instr = function->code->items[i];
    sb_append_fmt(&sb, "%zu: ", i);

    if (instr->kind == IR_INT_CONST) {
      sb_append_fmt(&sb, "%c%d = INT_CONST %d\n", TEMP_STR(instr->dest), instr->int_value);
      continue;
    }

    if (instr->kind == IR_RETURN) {
      sb_append_fmt(&sb, "RETURN\n");
      continue;
    }

    if (instr->kind == IR_EXIT) {
      sb_append_fmt(&sb, "EXIT %c%d\n", TEMP_STR(instr->dest)); 
      continue;
    }

    if (instr->kind == IR_BINARY) {
      switch (instr->binary_op) {
        case IR_BINARY_ADD: 
          sb_append_fmt(&sb, "ADD %c%d %c%d\n", TEMP_STR(instr->dest), TEMP_STR(instr->src)); 
        continue;
        case IR_BINARY_SUB:
          sb_append_fmt(&sb, "SUB %c%d %c%d\n", TEMP_STR(instr->dest), TEMP_STR(instr->src));          
          continue;
        case IR_BINARY_MUL:
          sb_append_fmt(&sb, "MUL %c%d %c%d\n", TEMP_STR(instr->dest), TEMP_STR(instr->src));          
          continue;
        case IR_BINARY_CMP:
          sb_append_fmt(&sb, "CMP %c%d %c%d\n", TEMP_STR(instr->dest), TEMP_STR(instr->src));
          continue;
        default:
          sb_append_fmt(&sb, "unknow binary op\n");
          continue;
      }
    }

    if (instr->kind == IR_STORE_VAR) {
      if (instr->var.is_init) {
        sb_append_fmt(&sb, "STR slot(%s), %c%d\n", instr->var.name, TEMP_STR(instr->src));
        continue; 
      }
      else {
        sb_append_fmt(&sb, "STR slot(%s), 0\n", instr->var.name);
        continue; 
      }
    }

    if (instr->kind == IR_LOAD_VAR) {
      sb_append_fmt(&sb, "LOAD %c%d, slot(%s)\n", TEMP_STR(instr->dest), instr->var.name); 
      continue;
    }

    if (instr->kind == IR_MOV) {
      sb_append_fmt(&sb, "MOV %c%d %c%d\n", TEMP_STR(instr->dest), TEMP_STR(instr->src));  
      continue;
    }

    if (instr->kind == IR_INC) {
      sb_append_fmt(&sb, "INC %c%d\n", TEMP_STR(instr->dest));  
      continue;
    }

    if (instr->kind == IR_DEC) {
      sb_append_fmt(&sb, "DEC %c%d\n", TEMP_STR(instr->dest));  
      continue;
    }

    if (instr->kind == IR_JMP_NOT_EQUAL) {
      sb_append_fmt(&sb, "JNE %s\n", instr->chunk_name); 
      continue;
    }

    if (instr->kind == IR_JMP_EQUAL) {
      sb_append_fmt(&sb, "JE %s\n", instr->chunk_name); 
      continue;
    }

    if (instr->kind == IR_JMP_GREATER_THAN) {
      sb_append_fmt(&sb, "JG %s\n", instr->chunk_name); 
      continue;
    }

    if (instr->kind == IR_JMP_GREATER_THAN_EQUAL) {
      sb_append_fmt(&sb, "JGE %s\n", instr->chunk_name);
      continue; 
    }

    if (instr->kind == IR_JMP_LOWER_THAN_EQUAL) {
      sb_append_fmt(&sb, "JLE %s\n", instr->chunk_name);
      continue; 
    }

    if (instr->kind == IR_JMP_LOWER_THAN) {
      sb_append_fmt(&sb, "JL %s\n", instr->chunk_name);
      continue; 
    }

    if (instr->kind == IR_CHUNK) {
      sb_append_fmt(&sb, "%s:\n", instr->chunk_name);
      continue; 
    }

    if (instr->kind == IR_JMP) {
      sb_append_fmt(&sb, "JMP %s\n", instr->chunk_name);  
      continue;
    }

    if (instr->kind == IR_CALL) {
      sb_append_fmt(&sb, "CALL %s\n", instr->func_name); 
      continue;
    }

    if (instr->kind == IR_ALLOC) {
      sb_append_fmt(&sb, "ALLOC %zu\n", instr->alloc_size);
      continue;
    }

    if (instr->kind == IR_DEALLOC) {
      sb_append_fmt(&sb, "DEALLOC %c%d, %zu\n", TEMP_STR(instr->src), instr->src.size); 
    }

    if (instr->kind == IR_ASM) {
      sb_append_fmt(&sb, "ASM [");
      for (size_t j = 0; j < instr->asm_data.string_count; j++) {
        if (j > 0) sb_append_fmt(&sb, ", ");
        sb_append_fmt(&sb, "\"%s\"", instr->asm_data.strings[j]);
      }
      sb_append_fmt(&sb, "]");
      if (instr->asm_data.arg_count > 0) {
        sb_append_fmt(&sb, " (");
        for (size_t j = 0; j < instr->asm_data.arg_count; j++) {
          if (j > 0) sb_append_fmt(&sb, ", ");
          sb_append_fmt(
              &sb, "%c%d", TEMP_STR(instr->asm_data.args[j]));
        }
        sb_append_fmt(&sb, ")");
      }
      sb_append_fmt(&sb, "\n");
      continue;
    }

    if (instr->kind == IR_LOAD_ELEM) {
      sb_append_fmt(&sb, "MOV %c%d, [%c%d + %c%d]\n", TEMP_STR(instr->dest), TEMP_STR(instr->src), TEMP_STR(instr->index)); 
      continue;
    }

    if (instr->kind == IR_STORE_ELEM) {
      sb_append_fmt(&sb, "MOV [%c%d + %c%d], %c%d\n", TEMP_STR(instr->dest), TEMP_STR(instr->index), TEMP_STR(instr->src));
      continue;
    }

    if (instr->kind == IR_DIRECT_MUL) {
      sb_append_fmt(&sb, "MUL %c%d, %d\n", TEMP_STR(instr->dest), instr->int_value);
    }

    if (instr->kind == IR_MOV_OFFSET) {
      if (instr->offset.timing == IR_PRE_OFFSET) {
        sb_append_fmt(&sb, "MOV [%c%d + %zu], %c%d\n", TEMP_STR(instr->dest), instr->offset.size, TEMP_STR(instr->src));
      } else {
        sb_append_fmt(&sb, "MOV %c%d, [%c%d + %zu]\n", TEMP_STR(instr->dest), TEMP_STR(instr->src), instr->offset.size);
      }
    }
  }

  return sb.items;
}

void IR_display_function(IR_function_t* function) 
{
 char* string_program = IR_generate_string_program(function); 
 puts(string_program);
}
