#include "import_resolver.h"
#include "frontend/symbols.h"
#include "thirdparty/error.h"
#include <string.h>
#include <stdio.h>

static char* join_path_segments(char** segs, size_t count)
{
  size_t len = 1;
  for (size_t i = 0; i < count; ++i)
    len += strlen(segs[i]) + 2;

  char* out = calloc(len, sizeof(char));
  if (!out) return NULL;

  for (size_t i = 0; i < count; ++i) {
    if (i > 0) strcat(out, "::");
    strcat(out, segs[i]);
  }

  return out;
}

static bool declares_internal_symbol(
    module_unit_t* unit, const char* name)
{
  da_foreach(declaration_t*, it, &unit->program) {
    declaration_t* d = *it;
    if (d->type == DECLARATION_FUNC &&
        d->func.is_internal &&
        strcmp(d->func.name, name) == 0)
      return true;
  }

  return false;
}

static function_symbol_t* find_exported_symbol(
    module_unit_array* units, 
    const char* name, 
    bool* out_is_internal)
{
  *out_is_internal = false;

  da_foreach(module_unit_t*, it, units) {
    module_unit_t* u = *it;

    function_symbol_t* fs =
      (function_symbol_t*) hashmap_get(u->export_funcs, name);
    if (fs) return fs;

    if (declares_internal_symbol(u, name))
      *out_is_internal = true;
  }

  return NULL;
}

bool semantic_resolve_imports(
    build_context_t* ctx,
    module_unit_t* unit,
    semantic_analyzer_t* analyzer)
{
  analyzer->imported_functions = calloc(1, sizeof(hashmap_t));
  if (!analyzer->imported_functions) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return false;
  }

  da_foreach(declaration_t*, it, &unit->program) {
    declaration_t* decl = *it;
    if (decl->type != DECLARATION_IMPORT) continue;

    import_path_t* path = &decl->import.path;
    if (path->count < 2) {
      semantic_error_register(analyzer, decl->source_pos - 1,
          "import path must reference `module::symbol`");
      continue;
    }

    const char* symbol_name = path->items[path->count - 1];
    const char* qualifier   = path->items[path->count - 2];

    char* module_name = join_path_segments(path->items, path->count - 1);
    if (!module_name) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      return false;
    }

    module_unit_array* target_units =
      (module_unit_array*) hashmap_get(ctx->registry, module_name);

    if (!target_units || target_units->count == 0) {
      semantic_error_register(analyzer, decl->source_pos - 1,
          "unknown imported module");
      free(module_name);
      continue;
    }

    bool is_internal = false;
    function_symbol_t* fs =
      find_exported_symbol(target_units, symbol_name, &is_internal);

    if (!fs) {
      semantic_error_register(analyzer, decl->source_pos - 1,
          is_internal
            ? "cannot import an internal function"
            : "undefined symbol in imported module");
      free(module_name);
      continue;
    }

    imported_symbol_t* isym = calloc(1, sizeof(imported_symbol_t));
    if (!isym) {
      error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
      free(module_name);
      return false;
    }

    isym->fs = fs;
    isym->module_name = module_name;
    isym->qualifier = strdup(qualifier);

    const char* local_name =
      decl->import.alias ? decl->import.alias : symbol_name;

    hashmap_put(analyzer->imported_functions, local_name, isym);

    size_t qk_len = strlen(qualifier) + 2 + strlen(symbol_name) + 1;
    char* qualified_key = malloc(qk_len);
    if (qualified_key) {
      snprintf(qualified_key, qk_len, "%s::%s", qualifier, symbol_name);
      hashmap_put(analyzer->imported_functions, qualified_key, isym);
      free(qualified_key);
    }

    da_append(&analyzer->imported_owned, isym);
  }

  return true;
}
