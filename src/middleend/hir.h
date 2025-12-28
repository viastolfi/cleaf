#ifndef HIR_H
#define HIR_H

#include <string.h>

#define DA_LIB_IMPLEMENTATION
#include "../thirdparty/da.h"
#include "../thirdparty/error.h"
#include "../frontend/ast_definition.h"
#include "../thirdparty/string_builder.h"
#include "hir_definition.h"

int HIR_lower_function(HIR_parser_t* hir, 
    declaration_t* function);
int HIR_lower_statement(HIR_parser_t* hir, 
    statement_t* stmt,
    HIR_function_t* func);
int HIR_lower_expression(HIR_parser_t* hir,
    expression_t* expr,
    HIR_function_t* func);
void HIR_display_function(HIR_function_t* function);
void HIR_free_function(HIR_function_t* func);
void HIR_free_instruction(HIR_instruction_t* instr);
char* HIR_generate_string_program(HIR_function_t* function);
int HIR_lower_binary_expression(expression_t* expr,
    HIR_parser_t* hir,
    HIR_instruction_t* instr,
    HIR_function_t* func);
#endif // HIR_H
