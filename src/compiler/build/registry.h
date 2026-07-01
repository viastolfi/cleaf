#ifndef REGISTRY_H
#define REGISTRY_H

#include <stdbool.h>

#include "compiler/definition/compiler_definition.h"
#include "thirdparty/hashmap.h"
#include "frontend/ast_definition.h"

bool populate_module_registry(
    build_context_t* ctx, module_unit_t* unit);

#endif // REGISTRY_H
