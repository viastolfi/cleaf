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

/*
static void print_statement(statement_t* s, int pos, int depth) 
{
  printf("%*s", pos*4, "");
  if (s->next) {
    printf(CLR_TREE "|" CLR_RESET);
  } else {
    printf(CLR_TREE "`" CLR_RESET);
  }
  if (depth == 0) {
    printf(CLR_TREE "-" CLR_RESET);
  } else {
    printf("%*s", depth*2, "");
    printf(CLR_TREE "`-" CLR_RESET);
  }

  if (s->type == (statement_kind) STATEMENT_RETURN){
    printf(CLR_STMT "ReturnStmt" CLR_RESET);
    printf(CLR_ADDRESS " %p" CLR_RESET, s);
    printf(CLR_TYPE " '%s'" CLR_RESET, s->ret.type);
    printf(CLR_LIT " %d" CLR_RESET, s->ret.int_value);
    printf("\n");
  }

  if (s->type == (statement_kind) STATEMENT_DECL) {
    printf(CLR_STMT "DeclStmt" CLR_RESET);
    printf(CLR_ADDRESS " TO IMPLEMENT" CLR_RESET "\n");
  }

  if (s->next)
    print_statement(s->next, pos, depth++);
}

static void print_declaration(declaration_t* d, int pos) 
{
  printf(CLR_TREE "`-" CLR_RESET);
  if (d->type == (declaration_kind) DECLARATION_FUNC) {
    printf(CLR_DECL "FunctionDecl " CLR_RESET);
    printf(CLR_ADDRESS "%p " CLR_RESET, d);
    printf("%s ", d->func.name);
    printf(CLR_TYPE "%s (%s)" CLR_RESET, d->func.return_type ? d->func.return_type : "" , d->func.params ? d->func.params->type : "");
    printf("\n");

    if (d->func.body) {
      printf("%*s", pos * 2, "");
      printf(CLR_TREE "`-" CLR_RESET);
      printf(CLR_STMT "CompoundStmt" CLR_RESET "\n");
      print_statement(d->func.body, pos, 0);
    }
  }

  // TODO: implement other type of decl
  // TODO: print d->next 
}
*/

#endif // AST_PRINTER_H
