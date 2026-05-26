#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "ast_definition.h"

typedef struct 
{
  known_type_t return_type;

  char** params_name;
  known_type_t* params_type;
  size_t params_count;
} function_symbol_t;

typedef struct {
  char** members_name;
  known_type_t* members_type;
  size_t members_count;
} struct_symbol_t;

#endif // SYMBOLS_H
