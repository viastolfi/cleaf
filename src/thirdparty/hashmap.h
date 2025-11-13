#ifndef HASHMAP_H
#define HASHMAP_H

#include <string.h>
#include <stdio.h>

#define HASH_SIZE 211

// use if we want to store NULL value but still get true on if (hashmap_get)
static char sentinel;

typedef struct hashmap_entry_t
{
  char* key;
  void* value;
  // in case of collision
  struct hashmap_entry_t* next;
} hashmap_entry_t;

typedef struct
{
  hashmap_entry_t* buckets[HASH_SIZE];
} hashmap_t;

inline static unsigned hashmap_hash(const char* s)
{
  unsigned h = 0;
  while (*s)
   h = (h << 4) + *s++; 
  return h % HASH_SIZE;
}

inline static void hashmap_put(hashmap_t* map, const char* key, void* value)
{
  unsigned idx = hashmap_hash(key);
  hashmap_entry_t* e = malloc(sizeof(*e));
  e->key = strdup(key);
  e->value = value;
  e->next = map->buckets[idx];
  map->buckets[idx] = e;
}

inline static void* hashmap_get(hashmap_t* map, const char* key)
{
  for (hashmap_entry_t* e = map->buckets[hashmap_hash(key)]; e; e = e->next)
    if (strcmp(key, e->key) == 0)
      return e->value ? e->value : (void*) &sentinel;

  return NULL;
}

inline static void hashmap_free(hashmap_t* map)
{
  for (size_t i = 0; i < HASH_SIZE; ++i) {
    hashmap_entry_t* entry = map->buckets[i];
    while (entry) {
      free(entry->key);
      free(entry->value);
      hashmap_entry_t* e = entry->next;
      free(entry);
      entry = e;
    }  
  } 
  free(map->buckets);
}

#endif // HASHMAP_H
