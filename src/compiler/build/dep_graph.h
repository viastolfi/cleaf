#ifndef DEP_GRAPH_H
#define DEP_GRAPH_H

#include <stdbool.h>
#include "compiler/definition/compiler_definition.h"
#define DA_LIB_IMPLEMENTATION
#include "thirdparty/da.h"
#include "thirdparty/hashmap.h"

typedef enum {
  WHITE = 0,
  GRAY  = 1,
  BLACK = 2,
} visit_color_t;

typedef struct dep_node_t {
  char*                  module_name; // not owned
  visit_color_t          color;
  struct dep_node_t**    items;    // act as 'arcs'
  size_t                 count;    // we use items, count, capacity
  size_t                 capacity; // so we can use da.h interface 
} dep_node_t;

typedef struct {
  dep_node_t* items;
  size_t      count;
  size_t      capacity;
  hashmap_t*  index;
} dep_graph_t;

bool build_dep_graph(build_context_t* ctx);

#endif // DEP_GRAPH_H
