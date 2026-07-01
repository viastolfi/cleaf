#include "compiler_definition.h"
#include "frontend/symbols.h"

void module_unit_free(module_unit_t* unit)
{
  if (!unit) return;

  if (unit->parser.types) {
    da_foreach(known_type_t, it, unit->parser.types) {
      if (it->kind == TYPE_CUSTOM && it->name)
        free(it->name);
    }
    da_free(unit->parser.types);
    free(unit->parser.types);
  }

  for (size_t i = 0; i < unit->parser.count; i++) {
    if (unit->parser.items[i].string_value)
      free(unit->parser.items[i].string_value);
  }
  da_free(&unit->parser);

  da_foreach(declaration_t*, it, &unit->program) {
    free_declaration(*it);
  }
  da_free(&unit->program);

  free(unit->module_name);
  free(unit->source);

  if (unit->export_funcs) {
    for (size_t i = 0; i < 211; ++i) {
      hashmap_entry_t* e = unit->export_funcs->buckets[i];
      while (e) {
        function_symbol_t* fs = (function_symbol_t*) e->value;
        free(fs->params_name);
        free(fs->params_type);
        e = e->next;
      }
    }
    hashmap_free(unit->export_funcs, 1);
    free(unit->export_funcs);
  }

  free(unit);
}

void compiler_resources_free(compiler_resources_t* res)
{
  if (!res) return;

  da_foreach(module_unit_t*, it, &res->units) {
    module_unit_free(*it);
  }
  da_free(&res->units);

  da_foreach(char*, it, &res->files) {
    free(*it);
  }
  da_free(&res->files);

  if (res->hir_program) {
    da_foreach(IR_function_t*, it, res->hir_program) {
      IR_free_function(*it);
    }
    da_free(res->hir_program);
    free(res->hir_program);
    res->hir_program = NULL;
  }

  free(res);
}

void build_context_free(build_context_t* ctx)
{
  if (ctx->items) {
    da_free(ctx);
    ctx->items = NULL;
  }

  if (ctx->registry) {
    for (size_t i = 0; i < HASH_SIZE; i++) {
      hashmap_entry_t* e = ctx->registry->buckets[i];
      while (e) {
        module_unit_array* arr = (module_unit_array*) e->value;
        if (arr) {
          da_free(arr);
          free(arr);
        }
        e = e->next;
      }
    }
    hashmap_free(ctx->registry, 0);
    free(ctx->registry);
    ctx->registry = NULL;
  }
}
