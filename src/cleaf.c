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
#include "backend/codegen.h"
#include "backend/x86_64_definition.h"
#include "compiler/definition/compiler_definition.h"
#include "compiler/setup/compiler_setup.h"

int main(int argc, char** argv) 
{
  compiler_resources_t* res = NULL;

  if (argc > 1 && strcmp(argv[1], "build") == 0) {
    res = build_setup();  
  } else {
    res = single_file_setup(argc, argv);  
  }

  if (!res) return 1;

  log_phase("compiling", "%zu file(s)", res->files.count);

  da_foreach(char*, it, &res->files) {
    char* filename = *it;

    FILE* f = fopen(filename, "rb");
    if (!f) {
      error_report_general(
          ERROR_SEVERITY_ERROR, "cannot open file '%s'", filename);
      compiler_resources_free(res);
      return 1;
    }

    module_unit_t* unit = calloc(1, sizeof(module_unit_t));
    if (!unit) {
      fclose(f);
      compiler_resources_free(res);
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
      return 1;
    }
    populate_parser_known_type(unit->parser.types);

    while ((size_t) unit->parser.pos < unit->parser.count) {
      declaration_t* decl = parse_declaration(&unit->parser);
      if (!decl) {
        error_report_general(
            ERROR_SEVERITY_ERROR, "ast parse error in '%s'", filename);
        module_unit_free(unit);
        compiler_resources_free(res);
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

  compiler_resources_free(res);
  return 0;
}

