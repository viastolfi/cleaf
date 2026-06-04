#define CTEST_BEFORE_EACH
#define CTEST_LIB_IMPLEMENTATION
#include "ctest.h"

#define LEXER_LIB_IMPLEMENTATION
#include "../src/frontend/lexer.h"
#define DA_LIB_IMPLEMENTATION
#include "../src/thirdparty/da.h"

#include "../src/frontend/ast_definition.h"
#include "../src/frontend/ast.h"
#include "../src/frontend/semantic.h"
#include "../src/thirdparty/error.h"

before_each(semantic_analyzer_t, analyzer, char* file_path)
{
  FILE *f = fopen(file_path, "rb");
  if (f == NULL) {
    fprintf(stderr, "error on test file test/semantic_file/%s\n", file_path);
    abort();
  }

  char* text = (char*) malloc(1 << 20);
  int len = f ? (int) fread(text, 1, 1<<20, f) : -1;

  if (len < 0) {
    fprintf(stderr, "error while reading %s\n", file_path);
    free(text);
    fclose(f);
    abort();
  }
  fclose(f);

  parser_t p = {0};
  lexer_t lex;
  semantic_analyzer_t a = {0};

  error_context_t* error_ctx = calloc(1, sizeof(error_context_t));
  if (!error_ctx) abort();
  error_init(error_ctx, file_path, text, len);

  declaration_array* program = calloc(1, sizeof(declaration_array));
  if (!program) abort();

  char* storage = malloc(255);
  if (!storage) abort();
  lexer_init_lexer(&lex,
      text,
      text + len,
      storage,
      255);

  while (lexer_get_token(&lex)) {
    if (lex.token == LEXER_token_parse_error)
      break;

    token_t t = lexer_copy_token(&lex);
    da_append(&p, t);
  }

  free(storage);

  p.types = calloc(1, sizeof(known_type_array));
  populate_parser_known_type(p.types);

  while ((size_t)p.pos < p.count) {
    declaration_t* decl = parse_declaration(&p);
    da_append(program, decl);
  }

  free(text);

  a.error_ctx    = error_ctx;
  a.ast          = program;
  a.error_count  = 0;

  semantic_analyze(&a);

  for (size_t i = 0; i < p.count; i++) {
    if (p.items[i].string_value) {
      free(p.items[i].string_value);
    }
  }
  da_free(&p);

  analyzer = a;
}

void free_analyzer(semantic_analyzer_t* analyzer)
{
  da_foreach(declaration_t*, it, analyzer->ast) {
    free_declaration(*it);
  }

  da_free(analyzer->ast);
  free(analyzer->ast);
  free(analyzer->error_ctx);

  semantic_free_program_definition(analyzer);
}


ct_test(semantic_analyze, fn_def_simple, "test/semantic_case/fn_def_simple.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for simple function definition");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, fn_def_with_params, "test/semantic_case/fn_def_with_params.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for function with params");

  function_symbol_t* fs = (function_symbol_t*) hashmap_get(analyzer.function_symbols, "main");
  ct_assert_not_null(fs, "Function symbol should be in symbol table");
  ct_assert_eq((int)fs->params_count, 2, "Function should have 2 parameters");
  ct_assert_eq(fs->params_name[0], "a", "First param name should be 'a'");
  ct_assert_eq(fs->params_type[0].kind, TYPE_INT, "First param type should be TYPE_INT");
  ct_assert_eq(fs->params_name[1], "b", "Second param name should be 'b'");
  ct_assert_eq(fs->params_type[1].kind, TYPE_INT, "Second param type should be TYPE_STRING");

  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, fn_def_with_return_type, "test/semantic_case/fn_def_with_return_type.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for function with return type");

  function_symbol_t* fs = (function_symbol_t*) hashmap_get(analyzer.function_symbols, "main");
  ct_assert_not_null(fs, "Function symbol should be in symbol table");
  ct_assert_eq(fs->return_type.kind, TYPE_INT, "Return type should be TYPE_INT");

  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, fn_duplicate_definition, "test/semantic_case/fn_duplicate_definition.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for duplicate function definition");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, fn_duplicate_params, "test/semantic_case/fn_duplicate_params.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 errors for duplicate parameter names");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, var_decl_typed, "test/semantic_case/var_decl_typed.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for typed variable declaration");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, var_decl_untyped, "test/semantic_case/var_decl_untyped.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for untyped variable declaration");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, var_redefinition, "test/semantic_case/var_redefinition.clf") {
  ct_assert_eq(analyzer.error_count, 4, "Should have 4 errors for variable redefinition");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, var_undefined_use, "test/semantic_case/var_undefined_use.clf") {
  ct_assert_eq(analyzer.error_count, 2, "Should have 2 errors for using undefined variable");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, return_correct_type, "test/semantic_case/return_correct_type.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for correct return type");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, scope_if_statement, "test/semantic_case/scope_if_statement.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for if statement scope");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, scope_if_else, "test/semantic_case/scope_if_else.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for if-else statement scopes");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, scope_while_statement, "test/semantic_case/scope_while_statement.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for while statement scope");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, scope_for_statement, "test/semantic_case/scope_for_statement.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for for statement scope");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, scope_for_var_in_init, "test/semantic_case/scope_for_var_in_init.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for using for init var in body");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, param_in_function_body, "test/semantic_case/param_in_function_body.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for using parameter in function body");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, param_redefinition_in_body, "test/semantic_case/param_redefinition_in_body.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for redefining parameter in function body");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, multiple_functions, "test/semantic_case/multiple_functions.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for multiple function definitions");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, assign_expression, "test/semantic_case/assign_expression.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for variable assignement");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, for_bad_condition, "test/semantic_case/for_bad_condition.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for undef var in for condition");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, for_bad_loop, "test/semantic_case/for_bad_loop.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for undef var in for loop");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, unary_expression, "test/semantic_case/unary.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for basic unary expression");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, unary_expression_error, "test/semantic_case/unary_expression_error.clf") {
  ct_assert_eq(analyzer.error_count, 4, "Should have 4 errors for basic unary expression errors");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, unary_undef_var, "test/semantic_case/unary_undef_var.clf") {
  ct_assert_eq(analyzer.error_count, 5, "Should have 5 errors for using unary expr on undefined var");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, function_call, "test/semantic_case/function_call.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors for basic function call");
  free_analyzer(&analyzer);
}

ct_test(semantic_analyze, function_call_undef, "test/semantic_case/function_call_undef.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have one error for undef function calling");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, function_call_in_statement, "test/semantic_case/function_call_in_statement.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no error for using return value from function call in statement");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, function_call_with_argument, "test/semantic_case/function_call_with_argument.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no error for calling function with right arguments");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, function_call_with_too_much_args, "test/semantic_case/function_call_with_too_much_args.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for calling function with too much params");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, function_call_with_too_few_args, "test/semantic_case/function_call_with_too_few_args.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for calling function with too few params");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, function_call_no_arg_in_callee, "test/semantic_case/function_call_no_arg_in_callee.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have one error for calling function needing 1 param with 0");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, function_call_use_return, "test/semantic_case/function_call_use_return.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no error for using return type from a function call");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, function_declaration_reserved_keywords, "test/semantic_case/function_declaration_reserved_keywords.clf") {
  ct_assert_eq(analyzer.error_count, 6, "Should have 6 errors for different test case on function declaration with reserve keywords");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, var_decl_reserved_keywords, "test/semantic_case/var_decl_reserved_keywords.clf") {
  ct_assert_eq(analyzer.error_count, 6, "Should have 6 errors for different test case on var decl with reserved keywords");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, struct_definition, "test/semantic_case/struct_definition.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have 0 error for struct definition");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, struct_definition_errors, "test/semantic_case/struct_definition_errors.clf") {
  ct_assert_eq(analyzer.error_count, 3, "Should have 3 error for struct definition");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, struct_var_zero_init, "test/semantic_case/struct_var_zero_init.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have 0 errors for struct var with zero init");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, struct_var_designated_init, "test/semantic_case/struct_var_designated_init.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have 0 errors for struct var with valid designated init");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, struct_var_designated_init_wrong_count, "test/semantic_case/struct_var_designated_init_wrong_count.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for designated init with wrong member count");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, struct_var_designated_init_unknown_member, "test/semantic_case/struct_var_designated_init_unknown_member.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for designated init with unknown member name");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, struct_var_designated_init_reordered, "test/semantic_case/struct_var_designated_init_reordered.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have 0 errors for designated init with valid members in non-declaration order");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, struct_var_designated_init_three_fields, "test/semantic_case/struct_var_designated_init_three_fields.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have 0 errors for valid 3-field designated init");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, struct_var_designated_init_duplicate_member, "test/semantic_case/struct_var_designated_init_duplicate_member.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error for designated init with duplicate member name");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, var_inferred_type_return, "test/semantic_case/var_inferred_type_return.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have 0 errors: var inferred as int must be usable in return");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, var_for_init_inferred_condition, "test/semantic_case/var_for_init_inferred_condition.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have 0 errors: var in for-init must infer correct type for use in condition");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, struct_composite_var_used_after_init, "test/semantic_case/struct_composite_var_used_after_init.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have 0 errors: struct var from designated init must be usable without spurious TYPE_ERROR");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, typed_var_undef_no_cascade, "test/semantic_case/typed_var_undef_no_cascade.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have exactly 1 error: typed var from undefined func must not cascade spurious error on subsequent use");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, struct_member_access_undef, "test/semantic_case/struct_member_access_undef.clf") {
  ct_assert_eq(analyzer.error_count, 1, "Should have 1 error: member 'z' is not part of struct v2");
  free_analyzer(&analyzer);
}

ct_test(semantic_case, struct_member_access_valid, "test/semantic_case/struct_member_access_valid.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have 0 errors: member 'x' is a valid member of struct v2");
  free_analyzer(&analyzer);
}

// --- Type write-back and size resolution tests ---

ct_test(semantic_case, type_writeback_inferred_var, "test/semantic_case/var_decl_untyped.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors");

  declaration_t* func_decl = analyzer.ast->items[0];
  statement_t* stmt = func_decl->func.body->items[0];
  declaration_t* var_decl = stmt->decl_stmt.decl;

  ct_assert_eq(var_decl->var_decl.ident.type.kind, TYPE_INT,
      "Inferred var type should be TYPE_INT after write-back");
  ct_assert_eq((int)var_decl->var_decl.ident.type.size, 8,
      "Inferred int var size should be 8");

  free_analyzer(&analyzer);
}

ct_test(semantic_case, type_writeback_explicit_var, "test/semantic_case/var_decl_typed.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors");

  declaration_t* func_decl = analyzer.ast->items[0];
  statement_t* stmt = func_decl->func.body->items[0];
  declaration_t* var_decl = stmt->decl_stmt.decl;

  ct_assert_eq(var_decl->var_decl.ident.type.kind, TYPE_INT,
      "Explicit int var type should be TYPE_INT after write-back");
  ct_assert_eq((int)var_decl->var_decl.ident.type.size, 8,
      "Explicit int var size should be 8");

  free_analyzer(&analyzer);
}

ct_test(semantic_case, struct_var_type_and_size, "test/semantic_case/struct_member_access_valid.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors");

  // ast->items[1] is the fn main declaration (items[0] is the struct)
  declaration_t* func_decl = analyzer.ast->items[1];
  statement_t* stmt = func_decl->func.body->items[0];
  declaration_t* var_decl = stmt->decl_stmt.decl;

  ct_assert_eq(var_decl->var_decl.ident.type.kind, TYPE_CUSTOM,
      "Struct var type should be TYPE_CUSTOM after write-back");
  ct_assert_eq((int)var_decl->var_decl.ident.type.size, 16,
      "Struct var size should be 16 (2 ints x 8 bytes)");

  free_analyzer(&analyzer);
}

ct_test(semantic_case, for_init_type_writeback, "test/semantic_case/var_for_init_inferred_condition.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors");

  declaration_t* func_decl = analyzer.ast->items[0];
  statement_t* for_stmt = func_decl->func.body->items[0];
  declaration_t* init_decl = for_stmt->for_stmt.decl_init;

  ct_assert_not_null(init_decl, "For-init decl should not be null");
  ct_assert_eq(init_decl->var_decl.ident.type.kind, TYPE_INT,
      "For-init inferred var type should be TYPE_INT after write-back");
  ct_assert_eq((int)init_decl->var_decl.ident.type.size, 8,
      "For-init inferred int var size should be 8");

  free_analyzer(&analyzer);
}

ct_test(semantic_case, member_access_type_annotation, "test/semantic_case/struct_member_access_valid.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors");

  // fn main body: items[0] = var decl, items[1] = return a.x
  declaration_t* func_decl = analyzer.ast->items[1];
  statement_t* ret_stmt = func_decl->func.body->items[1];
  expression_t* expr = ret_stmt->ret.value;

  ct_assert_not_null(expr, "Return expression should not be null");
  ct_assert_not_null(expr->var.member, "Member expression should not be null");

  ct_assert_eq(expr->var.ident.type.kind, TYPE_CUSTOM,
      "Member access base 'a' should be annotated as TYPE_CUSTOM (struct)");
  ct_assert_eq((int)expr->var.ident.type.size, 16,
      "Member access base 'a' should have size 16");
  ct_assert_eq(expr->var.member->var.ident.type.kind, TYPE_INT,
      "Member access '.x' should be annotated as TYPE_INT");
  ct_assert_eq((int)expr->var.member->var.ident.type.size, 8,
      "Member access '.x' should have size 8");

  free_analyzer(&analyzer);
}

ct_test(semantic_case, struct_symbol_total_size, "test/semantic_case/struct_definition.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors");

  struct_symbol_t* sym =
      (struct_symbol_t*) hashmap_get(analyzer.struct_symbols, "v2");
  ct_assert_not_null(sym, "Struct symbol 'v2' should be in symbol table");
  ct_assert_eq((int)sym->total_size, 16,
      "Struct v2 total_size should be 16 (2 int fields x 8 bytes each)");

  free_analyzer(&analyzer);
}

ct_test(semantic_case, struct_inference_if_while_condition, "test/semantic_case/struct_inference_if_while_condition.clf") {
  ct_assert_eq(analyzer.error_count, 0, "Should have no errors");

  declaration_t* func_decl = analyzer.ast->items[1];
  statement_t* while_stmt = func_decl->func.body->items[3];
  expression_t* expr = while_stmt->while_stmt.condition;

  ct_assert_eq(expr->binary.left->var.ident.type.name, "v2", "type infence should have been applied");
  ct_assert_eq(expr->binary.left->var.member->var.ident.type.name, "int", "type inference should have been applied to function member");

  statement_t* if_stmt = func_decl->func.body->items[2];
  expr = if_stmt->if_stmt.condition;

  ct_assert_eq(expr->binary.right->var.ident.type.name, "v2", "type infence should have been applied");
  ct_assert_eq(expr->binary.right->var.member->var.ident.type.name, "int", "type inference should have been applied to function member");

  free_analyzer(&analyzer);
}
