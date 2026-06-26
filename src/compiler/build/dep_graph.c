#include "compiler/build/dep_graph.h"

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

          // TODO: remove this unnecessary allocation
          char* import_name = calloc(255, sizeof(char));
          if (!import_name) return false;

          for (size_t i = 0; i < decl->import.path.count - 1; ++i) {
            if (import_name[0] == '\0') {
              import_name = 
                strcpy(import_name, decl->import.path.items[i]);
            }
            else {
              import_name = strcat(
                  strcat(import_name, "::"), 
                  decl->import.path.items[i]);
            }
          }

          dep_node_t* src_node =
            hashmap_get(graph.index, unit->module_name);
          dep_node_t* dst_node  = 
            hashmap_get(graph.index, import_name);

          if (!dst_node) {
            error_report_general(ERROR_SEVERITY_ERROR, 
               "unkown imported module %s\n", import_name); 
            return false;
          }

          da_append(src_node, dst_node);
        }
      }
       
      e = e->next;
    }
  }

  return true;
}
