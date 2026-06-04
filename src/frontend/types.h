#ifndef TYPES_H
#define TYPES_H

typedef enum {
  TYPE_U1 = 0,
  TYPE_U8,
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

static const types_ident types_description[TYPE_COUNT] = {
  {"u1", 1}, 
  {"u8", 8}, 
  {"u16", 16}, 
  {"u32", 32}, 
  {"int", 32}, 
  {"u64", 64},
  {"var", -1},
  {"untype", -1},
};

#endif // TYPES_H
