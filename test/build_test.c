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
#include "../src/compiler/definition/compiler_definition.h"
#include "../src/compiler/build/registry.h"
#include "../src/compiler/build/export_table.h"
#include "../src/compiler/build/import_resolver.h"

// Loads and parses a single .clf file into a fresh module_unit_t. `path`
// must outlive the returned unit (it is not duplicated, mirroring how
// module_unit_t.file_path is a borrowed pointer in the real compiler).
static module_unit_t* load_module_unit(const char* path)
{
  FILE* f = fopen(path, "rb");
  if (!f) {
    fprintf(stderr, "error opening test file '%s'\n", path);
    abort();
  }

  module_unit_t* unit = calloc(1, sizeof(module_unit_t));
  if (!unit) abort();

  unit->file_path = (char*) path;
  unit->source = malloc(1 << 20);
  unit->source_len = (int) fread(unit->source, 1, 1 << 20, f);
  fclose(f);

  error_init(&unit->error_ctx, path, unit->source, unit->source_len);
  unit->parser.error_ctx = &unit->error_ctx;

  lexer_t lex;
  lexer_init_lexer(
      &lex, unit->source, unit->source + unit->source_len,
      malloc(4096), 4096);

  while (lexer_get_token(&lex)) {
    if (lex.token == LEXER_token_parse_error) break;
    token_t t = lexer_copy_token(&lex);
    da_append(&unit->parser, t);
  }
  free(lex.string_storage);

  unit->parser.types = calloc(1, sizeof(known_type_array));
  populate_parser_known_type(unit->parser.types);

  while ((size_t) unit->parser.pos < unit->parser.count) {
    declaration_t* decl = parse_declaration(&unit->parser);
    if (!decl) break;
    da_append(&unit->program, decl);
  }

  return unit;
}

typedef struct {
  build_context_t     ctx;
  module_unit_t*       dep_unit;
  module_unit_t*       main_unit;
  semantic_analyzer_t  analyzer;
} build_test_ctx_t;

before_each(build_test_ctx_t, tctx, char* dep_path, char* main_path)
{
  build_test_ctx_t t = {0};

  t.ctx.registry = calloc(1, sizeof(hashmap_t));
  if (!t.ctx.registry) abort();

  t.dep_unit  = load_module_unit(dep_path);
  t.main_unit = load_module_unit(main_path);

  if (!populate_module_registry(&t.ctx, t.dep_unit)) abort();
  if (!populate_module_registry(&t.ctx, t.main_unit)) abort();

  if (!semantic_build_export_table(t.dep_unit)) abort();
  if (!semantic_build_export_table(t.main_unit)) abort();

  t.analyzer.error_ctx = &t.main_unit->error_ctx;
  t.analyzer.ast = &t.main_unit->program;

  semantic_resolve_imports(&t.ctx, t.main_unit, &t.analyzer);
  semantic_analyze(&t.analyzer);

  tctx = t;
}

static void free_build_test_ctx(build_test_ctx_t* t)
{
  semantic_free_program_definition(&t->analyzer);
  module_unit_free(t->dep_unit);
  module_unit_free(t->main_unit);
  build_context_free(&t->ctx);
}

ct_test(build_import, bare_call_resolves,
    "test/build_case/math_ok.clf",
    "test/build_case/main_bare_call_ok.clf")
{
  ct_assert_eq(tctx.analyzer.error_count, 0,
      "importing and calling a plain function should not error");
  free_build_test_ctx(&tctx);
}

ct_test(build_import, qualified_call_resolves,
    "test/build_case/math_ok.clf",
    "test/build_case/main_qualified_call_ok.clf")
{
  ct_assert_eq(tctx.analyzer.error_count, 0,
      "calling an imported function through its module qualifier "
      "should not error");
  free_build_test_ctx(&tctx);
}

ct_test(build_import, alias_call_resolves,
    "test/build_case/math_ok.clf",
    "test/build_case/main_alias_call_ok.clf")
{
  ct_assert_eq(tctx.analyzer.error_count, 0,
      "calling an imported function through its alias should not error");
  free_build_test_ctx(&tctx);
}

ct_test(build_import, unknown_module_errors,
    "test/build_case/math_ok.clf",
    "test/build_case/main_unknown_module.clf")
{
  ct_assert((tctx.analyzer.error_count > 0),
      "importing from an unregistered module should error");
  free_build_test_ctx(&tctx);
}

ct_test(build_import, missing_symbol_errors,
    "test/build_case/math_ok.clf",
    "test/build_case/main_missing_symbol.clf")
{
  ct_assert((tctx.analyzer.error_count > 0),
      "importing an undefined symbol should error");
  free_build_test_ctx(&tctx);
}

ct_test(build_import, internal_violation_errors,
    "test/build_case/math_internal.clf",
    "test/build_case/main_internal_violation.clf")
{
  ct_assert((tctx.analyzer.error_count > 0),
      "importing an internal function from another module should error");
  free_build_test_ctx(&tctx);
}

ct_test(build_import, qualifier_mismatch_errors,
    "test/build_case/math_ok.clf",
    "test/build_case/main_qualifier_mismatch.clf")
{
  ct_assert((tctx.analyzer.error_count > 0),
      "calling with a qualifier that doesn't match the imported "
      "module should error");
  free_build_test_ctx(&tctx);
}
