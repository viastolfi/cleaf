#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ast_printer.h"

#define PREFIX_MAX 512

static const char* type_str(type_kind t)
{
  switch (t) {
    case TYPE_INT:    return "int";
    case TYPE_UNTYPE: return "untyped";
    case TYPE_ERROR:  return "<error>";
    case TYPE_CUSTOM: return "<custom>";
    default:          return "?";
  }
}

static void print_known_type(const known_type_t* t)
{
  const char* name = (t->kind == TYPE_CUSTOM && t->name) ? t->name : type_str(t->kind);
  printf(CLR_TYPE "%s" CLR_RESET " " CLR_SIZE "(%zu)" CLR_RESET, name, t->size);
}

static const char* binary_op_str(binary_op_kind op)
{
  switch (op) {
    case BINARY_ADD:  return "+";
    case BINARY_SUB: return "-";
    case BINARY_MUL:   return "*";
    case BINARY_DIV:   return "/";
    case BINARY_GT:    return ">";
    case BINARY_GTE:   return ">=";
    case BINARY_LT:    return "<";
    case BINARY_LTE:   return "<=";
    case BINARY_EQ:    return "==";
    case BINARY_NEQ:   return "!=";
    default:           return "?";
  }
}

static const char* unary_op_str(unary_op_kind op)
{
  switch (op) {
    case UNARY_NEGATE:   return "-";
    case UNARY_NOT:      return "!";
    case UNARY_PRE_INC:  return "++ (pre)";
    case UNARY_PRE_DEC:  return "-- (pre)";
    case UNARY_POST_INC: return "++ (post)";
    case UNARY_POST_DEC: return "-- (post)";
    default:             return "?";
  }
}

static void child_prefix(const char* prefix, bool is_last, char* out)
{
  snprintf(out, PREFIX_MAX, "%s%s", prefix, is_last ? "  " : "| ");
}

static void print_branch(const char* prefix, bool is_last)
{
  printf("%s" CLR_TREE "%s" CLR_RESET, prefix, is_last ? "`-" : "|-");
}

static void print_expression(expression_t* e, const char* prefix, bool is_last);
static void print_statement(statement_t* s, const char* prefix, bool is_last);
static void print_declaration(declaration_t* d, const char* prefix, bool is_last);
static void print_block(statement_block_t* block, const char* prefix);

static void print_compound_stmt(statement_block_t* block,
                                const char* prefix, bool is_last)
{
  char cp[PREFIX_MAX];
  child_prefix(prefix, is_last, cp);

  print_branch(prefix, is_last);
  printf(CLR_DECL "CompoundStmt\n" CLR_RESET);
  print_block(block, cp);
}

static void print_expression(expression_t* e, const char* prefix, bool is_last)
{
  if (!e) return;

  char cp[PREFIX_MAX];
  child_prefix(prefix, is_last, cp);

  print_branch(prefix, is_last);

  switch (e->type) {
    case EXPRESSION_INT_LIT:
      printf(CLR_LIT "IntegerLiteral" CLR_RESET " %d\n", e->int_lit.value);
      break;

    case EXPRESSION_VAR:
      printf(CLR_LIT "VarRef" CLR_RESET " '%s'\n",
             e->var.name ? e->var.name : "");
      break;

    case EXPRESSION_BINARY:
      printf(CLR_STMT "BinaryExpr" CLR_RESET " '%s'\n",
             binary_op_str(e->binary.op));
      print_expression(e->binary.left,  cp, false);
      print_expression(e->binary.right, cp, true);
      break;

    case EXPRESSION_ASSIGN:
      printf(CLR_STMT "AssignExpr\n" CLR_RESET);
      print_expression(e->assign.lhs, cp, false);
      print_expression(e->assign.rhs, cp, true);
      break;

    case EXPRESSION_CALL:
      printf(CLR_STMT "CallExpr" CLR_RESET " '%s'\n",
             e->call.callee ? e->call.callee : "");
      for (size_t i = 0; i < e->call.arg_count; i++)
        print_expression(e->call.args[i], cp, i == e->call.arg_count - 1);
      break;

    case EXPRESSION_UNARY:
      printf(CLR_STMT "UnaryExpr" CLR_RESET " '%s'\n",
             unary_op_str(e->unary.op));
      print_expression(e->unary.operand, cp, true);
      break;
  }
}

static void print_block(statement_block_t* block, const char* prefix)
{
  if (!block || block->count == 0) return;
  for (size_t i = 0; i < block->count; i++)
    print_statement(block->items[i], prefix, i == block->count - 1);
}

static void print_statement(statement_t* s, const char* prefix, bool is_last)
{
  if (!s) return;

  char cp[PREFIX_MAX];
  child_prefix(prefix, is_last, cp);

  print_branch(prefix, is_last);

  switch (s->type) {
    case STATEMENT_RETURN:
      printf(CLR_STMT "ReturnStmt\n" CLR_RESET);
      if (s->ret.value)
        print_expression(s->ret.value, cp, true);
      break;

    case STATEMENT_DECL:
      printf(CLR_STMT "DeclStmt\n" CLR_RESET);
      if (s->decl_stmt.decl)
        print_declaration(s->decl_stmt.decl, cp, true);
      break;

    case STATEMENT_EXPR:
      printf(CLR_STMT "ExprStmt\n" CLR_RESET);
      if (s->expr_stmt.expr)
        print_expression(s->expr_stmt.expr, cp, true);
      break;

    case STATEMENT_IF: {
      printf(CLR_STMT "IfStmt\n" CLR_RESET);
      bool has_then = s->if_stmt.then_branch != NULL;
      bool has_else = s->if_stmt.else_branch != NULL;

      if (s->if_stmt.condition)
        print_expression(s->if_stmt.condition, cp, !has_then && !has_else);

      if (has_then)
        print_compound_stmt(s->if_stmt.then_branch, cp, !has_else);

      if (has_else)
        print_compound_stmt(s->if_stmt.else_branch, cp, true);
      break;
    }

    case STATEMENT_WHILE: {
      printf(CLR_STMT "WhileStmt\n" CLR_RESET);
      bool has_body = s->while_stmt.body != NULL;

      if (s->while_stmt.condition)
        print_expression(s->while_stmt.condition, cp, !has_body);

      if (has_body)
        print_compound_stmt(s->while_stmt.body, cp, true);
      break;
    }

    case STATEMENT_FOR: {
      printf(CLR_STMT "ForStmt\n" CLR_RESET);
      bool has_body = s->for_stmt.body != NULL;

      // init: either a declaration or an expression
      if (s->for_stmt.init_kind == FOR_INIT_DECL) {
        if (s->for_stmt.decl_init)
          print_declaration(s->for_stmt.decl_init, cp, false);
      } else {
        if (s->for_stmt.expr_init)
          print_expression(s->for_stmt.expr_init, cp, false);
      }

      if (s->for_stmt.condition)
        print_expression(s->for_stmt.condition, cp, false);

      if (s->for_stmt.loop)
        print_expression(s->for_stmt.loop, cp, !has_body);

      if (has_body)
        print_compound_stmt(s->for_stmt.body, cp, true);
      break;
    }
  }
}

static void print_declaration(declaration_t* d, const char* prefix, bool is_last)
{
  if (!d) return;

  char cp[PREFIX_MAX];
  child_prefix(prefix, is_last, cp);

  print_branch(prefix, is_last);

  switch (d->type) {
    case DECLARATION_FUNC: {
      printf(CLR_DECL "FunctionDecl" CLR_RESET " '%s'(",
             d->func.name ? d->func.name : "");

      for (size_t i = 0; i < d->func.params.count; i++) {
        typed_identifier_t* p = &d->func.params.items[i];
        print_known_type(&p->type);
        printf(" %s%s",
               p->ident_name ? p->ident_name : "",
               i < d->func.params.count - 1 ? ", " : "");
      }
      printf(")");

      if (d->func.return_type != TYPE_UNTYPE)
        printf(" -> " CLR_TYPE "%s" CLR_RESET, type_str(d->func.return_type));

      printf("\n");

      if (d->func.body)
        print_compound_stmt(d->func.body, cp, true);
      break;
    }

    case DECLARATION_VAR:
      printf(CLR_DECL "VarDecl" CLR_RESET " '%s': ",
             d->var_decl.ident.ident_name ? 
              d->var_decl.ident.ident_name : "");
      print_known_type(&d->var_decl.ident.type);
      printf("\n");
      if (d->var_decl.init)
        print_expression(d->var_decl.init, cp, true);
      break;

    case DECLARATION_STRUCT: {
      printf(CLR_DECL "StructDecl" CLR_RESET " '%s'\n",
             d->struc.name ? d->struc.name : "");
      for (size_t i = 0; i < d->struc.members.count; i++) {
        typed_identifier_t* m = &d->struc.members.items[i];
        bool last = (i == d->struc.members.count - 1);
        print_branch(cp, last);
        printf(CLR_DECL "FieldDecl" CLR_RESET " '%s': ",
               m->ident_name ? m->ident_name : "");
        print_known_type(&m->type);
        printf("\n");
      }
      break;
    }
  }
}

void ast_print_program(declaration_array* program)
{
  if (!program) return;

  printf(CLR_DECL "TranslationUnit\n" CLR_RESET);
  for (size_t i = 0; i < program->count; i++)
    print_declaration(program->items[i], "", i == program->count - 1);
}
