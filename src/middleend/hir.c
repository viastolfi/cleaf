#include "hir.h"

void HIR_free_instruction(HIR_instruction_t* instr) 
{
  //if (instr->string_value)
   //free(instr->string_value);

  //TODO: free call

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

  if (expr->type == EXPRESSION_BINARY) {
    HIR_instruction_t* instr = calloc(1, sizeof(HIR_instruction_t));  
    if (!instr) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      return -1;
    }
    instr->kind = HIR_BINARY;
    switch(expr->binary.op) {
      case BINARY_PLUS:
       instr->op = HIR_BINARY_ADD;
       break;
      default:
       return 1;
    }

    HIR_lower_expression(hir, expr->binary.left, func); 
    HIR_lower_expression(hir, expr->binary.right, func);
    instr->a = func->next_temp_id - 1;
    instr->b = func->next_temp_id;
    instr->dest = ++(func->next_temp_id);
    da_append(func->code, instr);
    return 0;
  }

  return 1;
}

int HIR_lower_statement(HIR_parser_t* hir, 
    statement_t* stmt,
    HIR_function_t* func)
{
  if (stmt->type == STATEMENT_EXPR) {
    HIR_lower_expression(hir, stmt->expr_stmt.expr, func); 
    return 0;
  }
  if (stmt->type == STATEMENT_RETURN) {
    HIR_lower_expression(hir, stmt->ret.value, func); 
    HIR_instruction_t* instr = calloc(1, sizeof(HIR_instruction_t));
    if (!instr) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return -1;
    }
    instr->kind = HIR_RETURN;
    instr->var = func->next_temp_id; 
    da_append(func->code, instr);
    return 0;
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
      sb_append_fmt(&sb, "RETURN t%d\n", instr->var);
      continue;
    }

    if (instr->kind == HIR_BINARY) {
      sb_append_fmt(&sb, "t%d = ADD t%d t%d\n", instr->dest, instr->a, instr->b); 
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
