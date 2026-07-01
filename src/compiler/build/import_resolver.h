#ifndef IMPORT_RESOLVER_H
#define IMPORT_RESOLVER_H

#include <stdbool.h>
#include "compiler/definition/compiler_definition.h"
#include "frontend/semantic.h"

bool semantic_resolve_imports(
    build_context_t* ctx,
    module_unit_t* unit,
    semantic_analyzer_t* analyzer);

#endif // IMPORT_RESOLVER_H
