#ifndef IR_H
#define IR_H

#include <string.h>

#define DA_LIB_IMPLEMENTATION
#include "../thirdparty/da.h"
#include "../frontend/ast_definition.h"
#include "../thirdparty/string_builder.h"
#include "ir_definition.h"

typedef struct 
{
  error_context_t* error_ctx;
  int error_count;

  chunk_name_gen_t gen_chunk;
  void* chunk_ctx;

  hashmap_t* struct_symbols;
  IR_function_array* hir_program;
} HIR_parser_t;

int IR_lower_function(
    HIR_parser_t* hir, 
    declaration_t* function);
int IR_lower_statement(
    HIR_parser_t* hir, 
    statement_t* stmt,
    IR_function_t* func);
int IR_lower_return_statement(
    HIR_parser_t* hir,
    statement_t* stmt,
    IR_function_t* func);
int IR_lower_expression(
    HIR_parser_t* hir,
    expression_t* expr,
    IR_function_t* func);
int IR_lower_expr_int_lit(
    HIR_parser_t* hir,
    expression_t* expr,
    IR_function_t* func);
int IR_lower_expr_char_lit(
    HIR_parser_t* hir,
    expression_t* expr,
    IR_function_t* func);
int IR_lower_expr_var(
    HIR_parser_t* hir,
    expression_t* expr,
    IR_function_t* func);
int IR_lower_expr_assign(
    HIR_parser_t* hir,
    expression_t* expr,
    IR_function_t* func);
void IR_display_function(IR_function_t* function);
void IR_free_function(IR_function_t* func);
void IR_free_instruction(IR_instruction_t* instr);
char* IR_generate_string_program(IR_function_t* function);
int IR_lower_binary_expression(expression_t* expr,
    HIR_parser_t* hir,
    IR_instruction_t* instr,
    IR_function_t* func);
int IR_lower_declaration(
    HIR_parser_t* hir,
    declaration_t* decl,
    IR_function_t* func);
int IR_lower_unary_expression(
    HIR_parser_t* hir,
    expression_t* expr,
    IR_function_t* func);
int IR_lower_if_statement(
    HIR_parser_t* hir,
    statement_t* stmt,
    IR_function_t* func);
int IR_lower_while_statement(
    HIR_parser_t* hir,
    statement_t* stmt,
    IR_function_t* func);
int IR_lower_for_statement(
    HIR_parser_t* hir,
    statement_t* stmt, 
    IR_function_t* func);
int IR_lower_composite_literal_expression(
    HIR_parser_t* hir, 
    declaration_t* decl, 
    IR_function_t* func);
int IR_lower_asm_statement(
     HIR_parser_t* hir,
     statement_t* stmt,
     IR_function_t* func);
int IR_lower_free_statement(
    HIR_parser_t* hir,
    statement_t* stmt,
    IR_function_t* func);
int IR_lower_index_expression(
    HIR_parser_t* hir,
    expression_t* expr,
    IR_function_t* func);

#endif // IR_H
