#ifndef __MAP_H
#define __MAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16
#define HASH_OFFSET 14695981039346656037ul
#define HASH_PRIME 1099511628211ul

struct Entry {
  char* key;
  void* val;
};

struct Map {
  struct Entry* entries;
  size_t capacity;
  size_t size;
};

struct MapIter {
  struct Entry* current_entry;
  struct Map* map;
  size_t idx;
};

typedef struct Entry Entry;
typedef struct Map Map;
typedef struct MapIter MapIter;

Map map_new();
int map_init(Map* map);
void* map_get(Map* map, const char* key);
int map_set(Map* map, const char* key, void* val);
int map_set_entry(Map* map, const char* key, void* val);
int map_expand(Map* map);
MapIter map_create_iter(Map* map);
Entry* map_iter_next(MapIter* mi);
void map_free(Map* map);
uint64_t hash_key(const char* key);

#endif