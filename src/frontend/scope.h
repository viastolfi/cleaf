#ifndef SCOPE_H
#define SCOPE_H

#include "../thirdparty/hashmap.h"

typedef struct scope_t
{
  hashmap_t* symbols;
  struct scope_t* parent;
} scope_t;

static inline scope_t* scope_enter(scope_t* parent) 
{ 
  scope_t* s = calloc(1, sizeof(scope_t));
  if (!s)
    return NULL;
  s->symbols = calloc(1, sizeof(hashmap_t));
  if (!s->symbols)
    return NULL;

  if (parent)
    s->parent = parent;

  return s;
}

static inline void scope_exit(scope_t* scope) {
  if (scope) {
    if (scope->symbols) {
      hashmap_free(scope->symbols, 0);
      free(scope->symbols);
    }
    free(scope);
  }
}

static inline void* scope_resolve(scope_t* scope, const char* name) {
  for (scope_t* s = scope; s != NULL; s = s->parent) {
    void* sym = hashmap_get(s->symbols, name);
    if (sym)
      return sym;
  }
  return NULL;
}

#endif // SCOPE_H
