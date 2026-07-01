#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LEXER_LIB_IMPLEMENTATION
#include "frontend/lexer.h"
#define DA_LIB_IMPLEMENTATION
#include "thirdparty/da.h"
#define LOG_LIB_IMPLEMENTATION
#include "thirdparty/log.h"

#include "frontend/ast_definition.h"
#include "frontend/ast.h"
#include "frontend/ast_printer.h"
#include "thirdparty/error.h"
#include "frontend/semantic.h"
#include "middleend/hir.h"
#include "thirdparty/rand.h"
#include "backend/codegen.h"
#include "backend/x86_64_definition.h"
#include "compiler/definition/compiler_definition.h"
#include "compiler/setup/compiler_setup.h"
#include "compiler/build/registry.h"
#include "compiler/build/dep_graph.h"
#include "compiler/build/export_table.h"
#include "compiler/build/import_resolver.h"

int main(int argc, char** argv) 
{
  compiler_resources_t* res = NULL;

  if (argc > 1 && strcmp(argv[1], "build") == 0) {
    res = build_setup();  
  } else {
    res = single_file_setup(argc, argv);  
  }

  if (!res) return 1;

  int is_build_mode = (argc > 1 && strcmp(argv[1], "build") == 0);

  log_phase("compiling", "%zu file(s)", res->files.count);

  build_context_t build_ctx = {0};
  if (is_build_mode) {
    build_ctx.registry = calloc(1, sizeof(hashmap_t));
    if (!build_ctx.registry) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory"); 
      compiler_resources_free(res);
      return 1;
    }
  }

  da_foreach(char*, it, &res->files) {
    char* filename = *it;

    FILE* f = fopen(filename, "rb");
    if (!f) {
      error_report_general(
          ERROR_SEVERITY_ERROR, "cannot open file '%s'", filename);
      compiler_resources_free(res);
      build_context_free(&build_ctx);
      return 1;
    }

    module_unit_t* unit = calloc(1, sizeof(module_unit_t));
    if (!unit) {
      fclose(f);
      compiler_resources_free(res);
      build_context_free(&build_ctx);
      return 1;
    }

    unit->file_path = filename;
    unit->source = malloc(1 << 20);
    unit->source_len = (int) fread(unit->source, 1, 1 << 20, f);
    fclose(f);

    error_init(&unit->error_ctx, filename, unit->source, unit->source_len);
    unit->parser.error_ctx = &unit->error_ctx;

    lexer_t lex;
    lexer_init_lexer(
        &lex, unit->source, unit->source + unit->source_len,
        malloc(4096), 4096);

    while (lexer_get_token(&lex)) {
      if (lex.token == LEXER_token_parse_error) {
        error_report_general(ERROR_SEVERITY_ERROR, "lexer parse error");
        free(lex.string_storage);
        module_unit_free(unit);
        compiler_resources_free(res);
        build_context_free(&build_ctx);
        return 1;
      }
      token_t t = lexer_copy_token(&lex);
      da_append(&unit->parser, t);
    }
    free(lex.string_storage);

    log_phase("lexing", "'%s': %zu tokens", filename, unit->parser.count);

    unit->parser.types = calloc(1, sizeof(known_type_array));
    if (!unit->parser.types) {
      module_unit_free(unit);
      compiler_resources_free(res);
      build_context_free(&build_ctx);
      return 1;
    }
    populate_parser_known_type(unit->parser.types);

    while ((size_t) unit->parser.pos < unit->parser.count) {
      declaration_t* decl = parse_declaration(&unit->parser);
      if (!decl) {
        error_report_general(
            ERROR_SEVERITY_ERROR, 
            "ast parse error in '%s'", filename);
        module_unit_free(unit);
        compiler_resources_free(res);
        build_context_free(&build_ctx);
        return 1;
      }
      da_append(&unit->program, decl);
    }

    log_phase("parsing", "'%s': %zu declaration(s)", filename, unit->program.count);

    if (log_is_dump()) {
      log_section_begin("AST");
      ast_print_program(&unit->program);
      log_section_end();
    }

    da_append(&res->units, unit);
  }

  if (is_build_mode) {
    da_foreach(module_unit_t*, it, &res->units) {
      if (!populate_module_registry(&build_ctx, *it)) {
        error_report_general(
            ERROR_SEVERITY_ERROR, 
            "error while building module registry");
        compiler_resources_free(res);
        build_context_free(&build_ctx);
        return 1;
      }
    }

    module_unit_array* main_units =
      (module_unit_array*) hashmap_get(build_ctx.registry, "main");

    if (!main_units || main_units->count == 0) {
      build_context_free(&build_ctx);
      error_report_general(ERROR_SEVERITY_ERROR,
         "no `main` module found");
      compiler_resources_free(res);
      return 1;
    }
  }

  if (is_build_mode) {
    if (!build_dep_graph(&build_ctx)) {
      build_context_free(&build_ctx);
      compiler_resources_free(res);
      return 1;
    }

    log_phase("topo order", "%zu module(s)", build_ctx.count);
    for (size_t i = 0; i < build_ctx.count; ++i)
      log_phase("  -->", "%s", build_ctx.items[i]->module_name);
  } else {
    da_foreach(module_unit_t*, it, &res->units) {
      da_append(&build_ctx, *it);
    }
  }

  da_foreach(module_unit_t*, it, &build_ctx) {
    if (!semantic_build_export_table(*it)) {
      build_context_free(&build_ctx);
      compiler_resources_free(res);
      return 1;
    }
  }

  res->hir_program = calloc(1, sizeof(IR_function_array));
  if (!res->hir_program) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    build_context_free(&build_ctx);
    compiler_resources_free(res);
    return 1;
  }

  rand_t chunk_rng;
  rand_init(&chunk_rng);

  int had_errors = 0;
  da_foreach(module_unit_t*, it, &build_ctx) {
    module_unit_t* unit = *it;

    semantic_analyzer_t analyzer = {0};
    analyzer.error_ctx = &unit->error_ctx;
    analyzer.ast = &unit->program;

    if (!semantic_resolve_imports(&build_ctx, unit, &analyzer)) {
      semantic_free_program_definition(&analyzer);
      build_context_free(&build_ctx);
      compiler_resources_free(res);
      return 1;
    }

    log_phase("semantic", "'%s' (module '%s')",
        unit->file_path, unit->module_name ? unit->module_name : "-");
    semantic_analyze(&analyzer);

    if (analyzer.error_count > 0) {
      had_errors = 1;
      semantic_free_program_definition(&analyzer);
      continue;
    }

    HIR_parser_t hir_parser = {0};
    hir_parser.error_ctx = &unit->error_ctx;
    hir_parser.hir_program = res->hir_program;
    hir_parser.struct_symbols = analyzer.struct_symbols;
    hir_parser.current_module = unit->module_name;
    HIR_PARSER_USE_RNG(hir_parser, &chunk_rng);

    size_t hir_before = res->hir_program->count;
    da_foreach(declaration_t*, dit, &unit->program) {
      if (IR_lower_function(&hir_parser, *dit) != 0) {
        error_report_general(
            ERROR_SEVERITY_ERROR, "hir lowering error in '%s'", unit->file_path);
        had_errors = 1;
        break;
      }
    }

    log_phase("hir", "'%s' (module '%s'): %zu function(s)",
        unit->file_path, unit->module_name ? unit->module_name : "-",
        res->hir_program->count - hir_before);

    if (log_is_dump()) {
      log_section_begin("HIR");
      for (size_t i = hir_before; i < res->hir_program->count; ++i) {
        char* hir_text = IR_generate_string_program(res->hir_program->items[i]);
        fprintf(stderr, "%s", hir_text);
        free(hir_text);
      }
      log_section_end();
    }

    semantic_free_program_definition(&analyzer);
  }

  build_context_free(&build_ctx);
  compiler_resources_free(res);
  return had_errors ? 1 : 0;
}

