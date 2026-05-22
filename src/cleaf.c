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

typedef struct {
  char* text;
  parser_t parser;
  declaration_array program;
  HIR_function_array* hir_program;
} compiler_resources_t;

static void compiler_resources_free(compiler_resources_t* res)
{
  if (res->hir_program) {
    da_foreach(HIR_function_t*, it, res->hir_program) {
      HIR_free_function(*it);
    }
    da_free(res->hir_program);
    free(res->hir_program);
    res->hir_program = NULL;
  }

  da_foreach(declaration_t*, it, &res->program) {
    free_declaration(*it);
  }
  da_free(&res->program);

  for (size_t i = 0; i < res->parser.count; i++) {
    if (res->parser.items[i].string_value)
      free(res->parser.items[i].string_value);
  }
  da_free(&res->parser);

  free(res->text);
  res->text = NULL;
}

int main(int argc, char** argv) 
{
  const char* filename = NULL;
  const char* output = NULL;
  log_verbosity_t verbosity = LOG_SILENT;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-v") == 0)
      verbosity = LOG_VERBOSE;
    else if (strcmp(argv[i], "-V") == 0)
      verbosity = LOG_DUMP;
    else if (strcmp(argv[i], "-o") == 0) {
      if (++i >= argc) {
        error_report_general(ERROR_SEVERITY_ERROR, "missing argument for '-o'");
        return 1;
      }
      output = argv[i];
    }
    else if (argv[i][0] != '-')
      filename = argv[i];
    else {
      error_report_general(ERROR_SEVERITY_ERROR, "unknown flag '%s'", argv[i]);
      fprintf(stderr, "usage: %s [-v|-V] [-o <output>] <file.clf>\n", argv[0]);
      return 1;
    }
  }

  log_set_verbosity(verbosity);

  if (!filename) {
    error_report_general(ERROR_SEVERITY_ERROR, "no input file provided");
    fprintf(stderr, "usage: %s [-v|-V] [-o <output>] <file.clf>\n", argv[0]);
    return 1;
  }

  log_phase("compiling", "'%s'", filename);

  FILE *f = fopen(filename, "rb");
  if (f == NULL) {
    error_report_general(ERROR_SEVERITY_ERROR, "cannot open file '%s'", filename);
    return 1;
  }

  compiler_resources_t res = {0};
  res.text = (char *) malloc(1 << 20);
  int len = (int) fread(res.text, 1, 1 << 20, f);
  fclose(f);
  if (len < 0) {
    error_report_general(ERROR_SEVERITY_ERROR, "failed to read file '%s'", filename);
    compiler_resources_free(&res);
    return 1;
  }

  error_context_t error_ctx;
  error_init(&error_ctx, filename, res.text, len);

  lexer_t lex;
  res.parser.error_ctx = &error_ctx;

  lexer_init_lexer(&lex, res.text, res.text + len, (char*) malloc(4096), 4096);

  while (lexer_get_token(&lex)) {
    if (lex.token == LEXER_token_parse_error) {
      error_report_general(ERROR_SEVERITY_ERROR, "lexer parse error");
      free(lex.string_storage);
      compiler_resources_free(&res);
      return 1;
    }
    token_t t = lexer_copy_token(&lex);
    da_append(&res.parser, t);
  }
  free(lex.string_storage);

  log_phase("lexing", "%zu tokens", res.parser.count);

  res.parser.types = calloc(1, sizeof(known_type_array));
  if (!res.parser.types) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");  
    return 1;
  }

  populate_parser_known_type(res.parser.types);

  while ((size_t) res.parser.pos < res.parser.count) {
    declaration_t* decl = parse_declaration(&res.parser);
    if (decl == NULL) {
      error_report_general(ERROR_SEVERITY_ERROR, "ast parse error");
      compiler_resources_free(&res);
      return 1;
    }
    da_append(&res.program, decl);
  }

  log_phase("parsing", "%zu declaration(s)", res.program.count);

  if (log_is_dump()) {
    log_section_begin("AST");
    ast_print_program(&res.program);
    log_section_end();
  }

  semantic_analyzer_t analyzer = {0};
  analyzer.error_ctx = &error_ctx;
  analyzer.ast = &res.program;
  analyzer.error_count = 0;

  semantic_analyze(&analyzer);
  semantic_free_function_definition(&analyzer);

  if (analyzer.error_count > 0) {
    log_phase("semantic", "%d error(s)", analyzer.error_count);
    error_report_general(ERROR_SEVERITY_NOTE,
        "%d error(s) during semantic analysis, aborting", analyzer.error_count);
    compiler_resources_free(&res);
    return 1;
  }
  log_phase("semantic", "ok");

  res.hir_program = calloc(1, sizeof(HIR_function_array));
  if (!res.hir_program) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    compiler_resources_free(&res);
    return 1;
  }

  HIR_parser_t hir_parser = {0};
  hir_parser.error_ctx = &error_ctx;
  hir_parser.error_count = 0;
  hir_parser.hir_program = res.hir_program;
  rand_t rng;
  rand_init(&rng);
  HIR_PARSER_USE_RNG(hir_parser, &rng);

  da_foreach(declaration_t*, it, &res.program) {
    int lowering_result = HIR_lower_function(&hir_parser, *it);
    if (lowering_result != 0) {
      error_report_general(ERROR_SEVERITY_ERROR, "HIR lowering error");
      compiler_resources_free(&res);
      return 1;
    }
  }

  log_phase("HIR lowering", "%zu function(s)", res.hir_program->count);

  if (log_is_dump()) {
    log_section_begin("HIR");
    da_foreach(HIR_function_t*, it, hir_parser.hir_program) {
      HIR_display_function(*it);
    }
    log_section_end();
  }

  string_builder_t asm_prog = {0};
  da_foreach(HIR_function_t*, it, hir_parser.hir_program) {
    int err = CODEGEN_write_function(&asm_prog, *it, &x86_64_target);
    if (err) {
      da_free(&asm_prog);
      compiler_resources_free(&res);
      return 1;
    }
  }

  log_phase("codegen", "ok");

  if (log_is_dump()) {
    log_section_begin("ASM");
    printf("%s", asm_prog.items);
    log_section_end();
  }

  // Use -o name if given, otherwise default to a.out
  const char* output_name = output ? output : "a.out";

  // Derive a safe basename for the temp .o (strip path from output_name)
  const char* obj_base = strrchr(output_name, '/');
  obj_base = obj_base ? obj_base + 1 : output_name;

  // Step 1: write asm to a temp file, then assemble with nasm
  char asm_path[512];
  snprintf(asm_path, sizeof(asm_path), "/tmp/%s.asm", obj_base);
  FILE* asm_file = fopen(asm_path, "w");
  if (!asm_file) {
    error_report_general(ERROR_SEVERITY_ERROR, "failed to write temp asm file");
    da_free(&asm_prog);
    compiler_resources_free(&res);
    return 1;
  }
  fputs(asm_prog.items, asm_file);
  fclose(asm_file);
  da_free(&asm_prog);

  char nasm_cmd[1024];
  snprintf(nasm_cmd, sizeof(nasm_cmd),
      "nasm -f elf64 -o /tmp/%s.o %s", obj_base, asm_path);
  if (system(nasm_cmd) != 0) {
    error_report_general(ERROR_SEVERITY_ERROR, "nasm failed");
    compiler_resources_free(&res);
    return 1;
  }
  remove(asm_path);

  log_phase("nasm", "ok");

  // Step 2: link
  char ld_cmd[1024];
  snprintf(ld_cmd, sizeof(ld_cmd), "ld -o %s /tmp/%s.o", output_name, obj_base);
  if (system(ld_cmd) != 0) {
    error_report_general(ERROR_SEVERITY_ERROR, "ld failed");
    compiler_resources_free(&res);
    return 1;
  }

  // Clean up temp object file
  char rm_cmd[512];
  snprintf(rm_cmd, sizeof(rm_cmd), "rm /tmp/%s.o", obj_base);
  system(rm_cmd);

  log_phase("linking", "'%s'", output_name);

  compiler_resources_free(&res);
  return 0;
}
