#include "compiler/build/registry.h"
#include <string.h>

static char* build_module_key(declaration_t* module_decl)
{
  char* key = NULL;
  da_foreach(char*, seg, &(module_decl->module.path)) {
    if (key) {
      size_t new_len = strlen(key) + 2 + strlen(*seg) + 1;
      key = realloc(key, new_len);
      if (!key) return NULL;
      strcat(strcat(key, "::"), *seg);
    } else {
      key = calloc(strlen(*seg) + 1, sizeof(char));
      if (!key) return NULL;
      strcpy(key, *seg);
    }
  }
  return key;
}

bool populate_module_registry(
    build_context_t* ctx, module_unit_t* unit)
{
  da_foreach(declaration_t*, it, &(unit->program)) {
    declaration_t* d = *it;
    if (d->type != DECLARATION_MODULE)
      continue;

    char* key = build_module_key(d);
    if (!key) return false;

    unit->module_name = strdup(key);
    if (!unit->module_name) { free(key); return false; }

    module_unit_array* arr = hashmap_get(ctx->registry, key);
    if (!arr) {
      arr = calloc(1, sizeof(module_unit_array));
      if (!arr) { free(key); return false; }
      hashmap_put(ctx->registry, key, arr);
    }

    da_append(arr, unit);
    free(key);
    return true;
  }

  return false;
}
