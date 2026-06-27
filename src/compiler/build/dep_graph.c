#include "compiler/build/dep_graph.h"

static void dep_graph_free(dep_graph_t* graph)
{
  da_foreach(dep_node_t, it, graph) {
    da_free(it);
  }
  da_free(graph);
  if (graph->index) {
    hashmap_free(graph->index, 0);
    free(graph->index);
    graph->index = NULL;
  }
}

static void topo_visit(build_context_t* ctx, dep_node_t* node)
{
  if (node->color == BLACK) return;

  if (node->color == GRAY) {
    error_report_general(ERROR_SEVERITY_ERROR,
        "circular dependency detected involving module '%s'",
        node->module_name);
    return;
  }

  node->color = GRAY;

  da_foreach(dep_node_t*, it, node) {
    topo_visit(ctx, (*it));
  }

  node->color = BLACK;
  module_unit_array* arr = hashmap_get(ctx->registry, node->module_name);
  if (arr) {
    da_foreach(module_unit_t*, it, arr) {
      da_append(ctx, (*it));
    }
  }
}

bool build_dep_graph(build_context_t* ctx)
{
  dep_graph_t graph = {0};

  graph.index = calloc(1, sizeof(hashmap_t));
  if (!graph.index) {
    error_report_general(ERROR_SEVERITY_ERROR, "out of memory");
    return false;
  }

  for (size_t i = 0; i < HASH_SIZE; ++i) {
    hashmap_entry_t* e = ctx->registry->buckets[i]; 

    while (e) {
      dep_node_t node = {0};
      node.module_name = e->key;
      node.color = WHITE;

      da_append(&graph, node);

      e = e->next;
    } 
  }

  da_foreach(dep_node_t, it, &graph) {
    hashmap_put(graph.index, it->module_name, it); 
  }

  for (size_t i = 0; i < HASH_SIZE; ++i) {
    hashmap_entry_t* e = ctx->registry->buckets[i];
    while (e) {
      module_unit_array* mod = (module_unit_array*) e->value;

      da_foreach(module_unit_t*, it, mod) {
        module_unit_t* unit = (*it);
        
        da_foreach(declaration_t*, it, &unit->program) {
          declaration_t* decl = (*it);

          if (decl->type != DECLARATION_IMPORT)
            continue;

          char* import_name = calloc(255, sizeof(char));
          if (!import_name) { dep_graph_free(&graph); return false; }

          for (size_t j = 0; j < decl->import.path.count - 1; ++j) {
            if (import_name[0] == '\0') {
              strcpy(import_name, decl->import.path.items[j]);
            }
            else {
              strcat(strcat(import_name, "::"), decl->import.path.items[j]);
            }
          }

          dep_node_t* src_node =
            hashmap_get(graph.index, unit->module_name);
          dep_node_t* dst_node  = 
            hashmap_get(graph.index, import_name);

          if (!dst_node) {
            error_report_general(ERROR_SEVERITY_ERROR, 
               "unkown imported module %s\n", import_name); 
            free(import_name);
            dep_graph_free(&graph);
            return false;
          }

          da_append(src_node, dst_node);
          free(import_name);
        }
      }
       
      e = e->next;
    }
  }

  da_foreach(dep_node_t, it, &graph) {
    if (strcmp(it->module_name, "main") == 0) 
      topo_visit(ctx, it);
  }

  dep_graph_free(&graph);
  return true;
}
