#include "import_resolver.h"
#include "frontend/symbols.h"
#include "thirdparty/assertion.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* join_path_segments(char** segments, size_t count)
{
  if (count == 0) return NULL;

  size_t len = 1;
  for (size_t i = 0; i < count; ++i)
    len += strlen(segments[i]) + (i > 0 ? 2 : 0);

  char* out = malloc(len);
  if (!out) return NULL;

  out[0] = '\0';
  for (size_t i = 0; i < count; ++i) {
    if (i > 0) strcat(out, "::");
    strcat(out, segments[i]);
  }

  return out;
}

static char* make_qualified_name(
    const char* qualifier, const char* symbol_name)
{
  size_t len = strlen(qualifier) + 2 + strlen(symbol_name) + 1;
  char* name = malloc(len);
  if (!name) return NULL;
  snprintf(name, len, "%s::%s", qualifier, symbol_name);
  return name;
}

static void import_module_exports(
    semantic_analyzer_t* analyzer,
    module_unit_array* units,
    const char* module_name,
    const char* qualifier,
    const char* source_pos)
{
  da_foreach(module_unit_t*, unit_it, units) {
    module_unit_t* unit = *unit_it;

    if (unit->export_funcs) {
      for (size_t i = 0; i < HASH_SIZE; ++i) {
        hashmap_entry_t* entry = unit->export_funcs->buckets[i];
        while (entry) {
          char* key = make_qualified_name(qualifier, entry->key);
          CLEAF_ASSERT(key != NULL, "out of memory");

          if (hashmap_get(analyzer->imported_functions, key)) {
            semantic_error_register(
                analyzer, source_pos,
                "module exports duplicate function name");
            free(key);
            entry = entry->next;
            continue;
          }

          imported_symbol_t* imported =
            calloc(1, sizeof(imported_symbol_t));
          CLEAF_ASSERT(imported != NULL, "out of memory");
          imported->fs = (function_symbol_t*) entry->value;
          imported->module_name = strdup(module_name);
          imported->qualifier = strdup(qualifier);
          CLEAF_ASSERT(
              imported->module_name != NULL &&
              imported->qualifier != NULL,
              "out of memory");

          hashmap_put(analyzer->imported_functions, key, imported);
          da_append(&analyzer->imported_owned, imported);
          free(key);
          entry = entry->next;
        }
      }
    }

    if (unit->export_types) {
      for (size_t i = 0; i < HASH_SIZE; ++i) {
        hashmap_entry_t* entry = unit->export_types->buckets[i];
        while (entry) {
          char* key = make_qualified_name(qualifier, entry->key);
          CLEAF_ASSERT(key != NULL, "out of memory");

          if (hashmap_get(analyzer->imported_types, key)) {
            semantic_error_register(
                analyzer, source_pos,
                "module exports duplicate type name");
          } else {
            hashmap_put(
                analyzer->imported_types, key, entry->value);
          }

          free(key);
          entry = entry->next;
        }
      }
    }
  }
}

bool semantic_resolve_imports(
    build_context_t* ctx,
    module_unit_t* unit,
    semantic_analyzer_t* analyzer)
{
  analyzer->imported_functions = calloc(1, sizeof(hashmap_t));
  analyzer->imported_modules = calloc(1, sizeof(hashmap_t));
  analyzer->imported_types = calloc(1, sizeof(hashmap_t));
  CLEAF_ASSERT(
      analyzer->imported_functions != NULL &&
      analyzer->imported_modules != NULL &&
      analyzer->imported_types != NULL,
      "out of memory");

  da_foreach(declaration_t*, it, &unit->program) {
    declaration_t* decl = *it;
    if (decl->type != DECLARATION_IMPORT) continue;

    import_path_t* path = &decl->import.path;
    char* module_name = join_path_segments(path->items, path->count);
    if (!module_name) {
      semantic_error_register(
          analyzer, decl->source_pos,
          "invalid or out of memory while resolving module import");
      continue;
    }

    module_unit_array* target_units =
      (module_unit_array*) hashmap_get(ctx->registry, module_name);
    if (!target_units || target_units->count == 0) {
      semantic_error_register(
          analyzer, decl->source_pos - 1, "unknown imported module");
      free(module_name);
      continue;
    }

    if (path->count > 1 && !decl->import.alias) {
      semantic_error_register(
          analyzer, decl->source_pos - 1,
          "nested module imports require an alias");
      free(module_name);
      continue;
    }

    const char* qualifier = decl->import.alias
      ? decl->import.alias
      : path->items[0];

    if (hashmap_get(analyzer->imported_modules, qualifier)) {
      semantic_error_register(
          analyzer, decl->source_pos - 1,
          "module import qualifier already in use");
      free(module_name);
      continue;
    }

    const char* canonical_name = target_units->items[0]->module_name;
    hashmap_put(
        analyzer->imported_modules, qualifier, (void*) canonical_name);
    import_module_exports(
        analyzer, target_units, canonical_name, qualifier,
        decl->source_pos - 1);
    free(module_name);
  }

  return true;
}
