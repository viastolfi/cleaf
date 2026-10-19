#ifndef TYPES_H
#define TYPES_H

typedef enum {
  TYPE_U8 = 0,
  TYPE_CHAR,
  TYPE_U16,
  TYPE_U32,
  TYPE_INT,
  TYPE_STRING,
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
  {"string",  8},
  {"u64",     8},
  {"var",    -1},
  {"untype", -1},
};

static inline int types_is_numeric(types_t kind)
{
  switch (kind) {
    case TYPE_U8:
    case TYPE_CHAR:
    case TYPE_U16:
    case TYPE_U32:
    case TYPE_INT:
    case TYPE_U64:
      return 1;
    default:
      return 0;
  }
}

#endif // TYPES_H
