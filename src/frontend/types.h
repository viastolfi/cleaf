#ifndef TYPES_H
#define TYPES_H

typedef enum {
  TYPE_U8 = 0,
  TYPE_CHAR,
  TYPE_U16,
  TYPE_U32,
  TYPE_INT,
  TYPE_U64,
  TYPE_VAR,
  TYPE_UNTYPE,
  TYPE_COUNT,

  TYPE_CUSTOM,
  TYPE_ERROR
} types_t;

typedef struct {
  char* name;
  size_t size;
} types_ident;

// we store sizes as bytes while the syntax use bits for convinience
static const types_ident types_description[TYPE_COUNT] = {
  {"u8",      1}, 
  {"char",    1},
  {"u16",     2}, 
  {"u32",     4}, 
  {"int",     4}, 
  {"u64",     8},
  {"var",    -1},
  {"untype", -1},
};

#endif // TYPES_H
