#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "ast_definition.h"

typedef struct 
{
  type_kind return_type;

  char** params_name;
  type_kind* params_type;
  size_t params_count;
} function_symbol_t;

typedef struct {
  char** members_name;
  type_kind* members_type;
  size_t members_count;
} struct_symbol_t;

#endif // SYMBOLS_H
