#ifndef CODEGEN_H
#define CODEGEN_H

#include <string.h>

#include "../middleend/ir_definition.h"
#include "../thirdparty/string_builder.h"
#include "target.h"
#include "../thirdparty/da.h"
#include "../thirdparty/error.h"

typedef struct {
  char* name;
  int place;
} var_pair_t;

typedef struct {
  var_pair_t* items;
  size_t count;
  size_t capacity;
} var_array;

int CODEGEN_write_function(
    string_builder_t* sb,
    IR_function_t* func,
    const target_t* target);

#endif //CODEGEN_H
