#ifndef EXPORT_TABLE_H
#define EXPORT_TABLE_H

#include <stdbool.h>
#include "compiler/definition/compiler_definition.h"

bool semantic_build_export_table(
    module_unit_t* unit, hashmap_t* type_symbols);

#endif // EXPORT_TABLE_H
