#ifndef HASHMAP_H
#define HASHMAP_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define HASH_SIZE 211

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
    h = (h << 4) + (unsigned char)*s++;
  return h % HASH_SIZE;
}

inline static void hashmap_put(hashmap_t* map, const char* key, void* value)
{
  if (!map || !key) return;
  unsigned idx = hashmap_hash(key);
  hashmap_entry_t* e = malloc(sizeof(*e));
  if (!e) return; 
  e->key = strdup(key);
  e->value = value;
  e->next = map->buckets[idx];
  map->buckets[idx] = e;
}

inline static void* hashmap_get(hashmap_t* map, const char* key)
{
  if (!map || !key) return NULL;

  unsigned idx = hashmap_hash(key);
  hashmap_entry_t* e = map->buckets[idx];
  if (!e) return NULL;

  for (; e; e = e->next) {
    if (strcmp(key, e->key) == 0)
      return e->value; 
  }

  return NULL;
}

inline static void hashmap_free(hashmap_t* map, int pointer_value)
{
  if (!map) return;

  for (size_t i = 0; i < HASH_SIZE; ++i) {
    hashmap_entry_t* entry = map->buckets[i];
    while (entry) {
      free(entry->key);
      if (pointer_value && entry->value) {
        free(entry->value);
      }
      hashmap_entry_t* e = entry->next;
      if (entry)
        free(entry);
      entry = e;
    }
    map->buckets[i] = NULL;
  }
}

inline static hashmap_t* hashmap_merge(hashmap_t* map1, hashmap_t* map2) {
  hashmap_t* map = calloc(1, sizeof(hashmap_t));
  if (!map) {
    free(map);
    return NULL; 
  }

  for (size_t i = 0; i < HASH_SIZE; ++i) {
    hashmap_entry_t* e = map1->buckets[i];
    while (e) 
      if (e->key)
        hashmap_put(map, e->key, e->value);
    e = map2->buckets[i];
    while(e)
      if (e->key)
        hashmap_put(map, e->key, e->value);
  }

  return map;
}

#endif // HASHMAP_H

