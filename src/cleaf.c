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

  // TODO: implement this
  if (strcmp(argv[1], "build") == 0) {
    res = build_setup();  
    return 0;
  } else {
    res = single_file_setup(argc, argv);  
  }

  error_context_t error_ctx;
  error_init(&error_ctx, res->filename, res->text, res->len);

  lexer_t lex;
  res->parser.error_ctx = &error_ctx;

  lexer_init_lexer(
      &lex, res->text, res->text + res->len, (char*) malloc(4096), 4096);

  while (lexer_get_token(&lex)) {
    if (lex.token == LEXER_token_parse_error) {
      error_report_general(
          ERROR_SEVERITY_ERROR, "lexer parse error");
      free(lex.string_storage);
      compiler_resources_free(res);
      return 1;
    }
    token_t t = lexer_copy_token(&lex);
    lexer_print_token(&lex);
    printf("  ");
    da_append(&res->parser, t);
  }
  free(lex.string_storage);

  log_phase("lexing", "%zu tokens", res->parser.count);

  res->parser.types = calloc(1, sizeof(known_type_array));
  if (!res->parser.types) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");  
    return 1;
  }

  populate_parser_known_type(res->parser.types);

  while ((size_t) res->parser.pos < res->parser.count) {
    declaration_t* decl = parse_declaration(&res->parser);
    if (decl == NULL) {
      error_report_general(ERROR_SEVERITY_ERROR, "ast parse error");
      compiler_resources_free(res);
      return 1;
    }
    da_append(&res->program, decl);
  }

  log_phase("parsing", "%zu declaration(s)", res->program.count);

  if (log_is_dump()) {
    log_section_begin("AST");
    ast_print_program(&res->program);
    log_section_end();
  }

  semantic_analyzer_t analyzer = {0};
  analyzer.error_ctx = &error_ctx;
  analyzer.ast = &res->program;
  analyzer.error_count = 0;

  semantic_analyze(&analyzer);

  if (log_is_dump()) {
    log_section_begin("AST after semantic");
    ast_print_program(&res->program);
    log_section_end();
  }

  if (analyzer.error_count > 0) {
    log_phase("semantic", "%d error(s)", analyzer.error_count);
    error_report_general(
        ERROR_SEVERITY_NOTE, 
        "%d error(s) during semantic analysis, aborting", 
        analyzer.error_count);
    semantic_free_program_definition(&analyzer);
    compiler_resources_free(res);
    return 1;
  }
  log_phase("semantic", "ok");

  res->hir_program = calloc(1, sizeof(IR_function_array));
  if (!res->hir_program) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    compiler_resources_free(res);
    return 1;
  }

  HIR_parser_t hir_parser = {0};
  hir_parser.error_ctx = &error_ctx;
  hir_parser.error_count = 0;
  hir_parser.hir_program = res->hir_program;
  hir_parser.struct_symbols = analyzer.struct_symbols;
  rand_t rng;
  rand_init(&rng);
  HIR_PARSER_USE_RNG(hir_parser, &rng);

  da_foreach(declaration_t*, it, &res->program) {
    if ((*it)->type != DECLARATION_FUNC)
      continue;

    int lowering_result = IR_lower_function(&hir_parser, *it);
    if (lowering_result != 0) {
      error_report_general(
          ERROR_SEVERITY_ERROR, "HIR lowering error");
      compiler_resources_free(res);
      return 1;
    }
  }

  log_phase(
      "HIR lowering", "%zu function(s)", res->hir_program->count);

  semantic_free_program_definition(&analyzer);
  if (log_is_dump()) {
    log_section_begin("HIR");
    da_foreach(IR_function_t*, it, hir_parser.hir_program) {
      IR_display_function(*it);
    }
    log_section_end();
  }

  string_builder_t asm_prog = {0};
  da_foreach(IR_function_t*, it, hir_parser.hir_program) {
    int err = CODEGEN_write_function(&asm_prog, *it, &x86_64_target);
    if (err) {
      da_free(&asm_prog);
      compiler_resources_free(res);
      return 1;
    }
  }

  log_phase("codegen", "ok");

  if (log_is_dump()) {
    log_section_begin("ASM");
    printf("%s", asm_prog.items);
    log_section_end();
  }

  const char* output_name = res->output ? res->output : "a.out";

  const char* obj_base = strrchr(output_name, '/');
  obj_base = obj_base ? obj_base + 1 : output_name;

  char asm_path[512];
  snprintf(asm_path, sizeof(asm_path), "/tmp/%s.asm", obj_base);
  FILE* asm_file = fopen(asm_path, "w");
  if (!asm_file) {
    error_report_general(ERROR_SEVERITY_ERROR, "failed to write temp asm file");
    da_free(&asm_prog);
    compiler_resources_free(res);
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
    compiler_resources_free(res);
    return 1;
  }
  remove(asm_path);

  log_phase("nasm", "ok");

  char ld_cmd[1024];
  snprintf(
      ld_cmd, sizeof(ld_cmd), "ld -o %s /tmp/%s.o", 
      output_name, obj_base);

  if (system(ld_cmd) != 0) {
    error_report_general(ERROR_SEVERITY_ERROR, "ld failed");
    compiler_resources_free(res);
    return 1;
  }

  char rm_cmd[512];
  snprintf(rm_cmd, sizeof(rm_cmd), "rm /tmp/%s.o", obj_base);
  system(rm_cmd);

  log_phase("linking", "'%s'", output_name);

  compiler_resources_free(res);
  return 0;
}
