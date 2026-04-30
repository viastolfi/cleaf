#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "ast_definition.h"

#define CLR_RESET   "\033[0m"
#define CLR_TREE    "\033[0;34m"
#define CLR_DECL    "\033[0;1;32m"
#define CLR_ADDRESS "\033[0;33m"
#define CLR_TYPE    "\033[0;32m"
#define CLR_STMT    "\033[0;35m"
#define CLR_LIT     "\033[0;36m"

void ast_print_program(declaration_array* program);

#endif // AST_PRINTER_H
