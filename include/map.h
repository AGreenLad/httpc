#ifndef __HTTPC_MAP_H
#define __HTTPC_MAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 64
#define HASH_OFFSET 14695981039346656037ul
#define HASH_PRIME 1099511628211ul

typedef struct hc_map_entry {
  char* key;
  void* val;
} hc_map_entry;

typedef struct hc_map {
  hc_map_entry* entries;
  size_t capacity;
  size_t size;
} hc_map;

typedef struct hc_map_iter {
  hc_map_entry* current_entry;
  hc_map* map;
  size_t idx;
} hc_map_iter;

hc_map hc_map_new();
int hc_map_init(hc_map* map);
void* hc_map_get(hc_map* map, const char* key);
int hc_map_set(hc_map* map, const char* key, void* val);
int hc_map_set_entry(hc_map* map, const char* key, void* val);
int hc_map_expand(hc_map* map);
hc_map_iter hc_map_create_iter(hc_map* map);
hc_map_entry* hc_map_iter_next(hc_map_iter* mi);
void hc_map_free(hc_map* map);
uint64_t hc_hash_key(const char* key);

#endif