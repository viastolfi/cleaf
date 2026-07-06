#include "export_table.h"
#include "frontend/symbols.h"
#include "thirdparty/error.h"
#include "thirdparty/assertion.h"

bool semantic_build_export_table(module_unit_t* unit)
{
  unit->export_funcs = calloc(1, sizeof(hashmap_t));
  CLEAF_ASSERT(unit->export_funcs != NULL, "out of memory");

  da_foreach(declaration_t*, it, &unit->program) {
    declaration_t* decl = *it;
    if (decl->type != DECLARATION_FUNC) continue;
    if (decl->func.is_internal)        continue;

    function_symbol_t* fs = calloc(1, sizeof(function_symbol_t));
    CLEAF_ASSERT(fs != NULL, "out of memory");

    fs->return_type.kind = decl->func.return_type.kind;
    fs->params_count = decl->func.params.count;

    if (fs->params_count > 0) {
      fs->params_name = calloc(fs->params_count, sizeof(char*));
      fs->params_type = 
        calloc(fs->params_count, sizeof(variable_symbol_t));


      CLEAF_ASSERT(fs->params_name != NULL, "out of memory");
      CLEAF_ASSERT(fs->params_type != NULL, "out of memory");

      for (size_t i = 0; i < fs->params_count; ++i) {
        fs->params_name[i] = decl->func.params.items[i].ident_name;
        fs->params_type[i].type = decl->func.params.items[i].type;
        fs->params_type[i].is_constant = 
          decl->func.params.items[i].is_constant;
      }
    }

    hashmap_put(unit->export_funcs, decl->func.name, fs);
  }

  return true;
}
