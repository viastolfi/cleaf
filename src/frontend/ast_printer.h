#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include <stdio.h>

#include "ast_definition.h"

#define CLR_RESET   "\033[0m"
#define CLR_TREE    "\033[0;34m"
#define CLR_DECL    "\033[0;1;32m"
#define CLR_ADDRESS "\033[0;33m"
#define CLR_TYPE    "\033[0;32m"
#define CLR_STMT    "\033[0;35m"
#define CLR_LIT     "\033[0;36m"

static void print_statement(statement_t* s, int pos) 
{
  printf("%*s", pos, "");
  printf(CLR_TREE "`-" CLR_RESET);
  if (s->type == (statement_kind) STATEMENT_RETURN){
    printf(CLR_STMT "ReturnStmt" CLR_RESET);
    printf(CLR_ADDRESS " %p" CLR_RESET, s);
    printf(CLR_TYPE " '%s'" CLR_RESET, s->ret.type);
    printf(CLR_LIT " %d" CLR_RESET, s->ret.int_value);
    printf("\n");
  }
}

static void print_declaration(declaration_t* d) 
{
  printf(CLR_TREE "`-" CLR_RESET);
  if (d->type == (declaration_kind) DECLARATION_FUNC) {
    printf(CLR_DECL "FunctionDecl " CLR_RESET);
    printf(CLR_ADDRESS "%p " CLR_RESET, d);
    printf("%s ", d->func.name);
    printf(CLR_TYPE "%s (%s)" CLR_RESET, d->func.return_type ? d->func.return_type : "" , d->func.params ? d->func.params->type : "");
    printf("\n");

    if (d->func.body) {
      print_statement(d->func.body, 1);
    }
  }

  // TODO: implement other type of decl
  // TODO: print d->next 
}

#endif // AST_PRINTER_H
