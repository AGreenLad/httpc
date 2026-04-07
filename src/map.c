#include "map.h"

hc_map hc_map_new() {
  hc_map map;
  hc_map_init(&map);
  return map;
}

int hc_map_init(hc_map* map) {
  if (map == NULL) {
    return -1;
  }

  map->capacity = INITIAL_CAPACITY;
  map->size = 0;

  map->entries = calloc(map->capacity, sizeof(hc_map_entry));

  if (map->entries == NULL) {
    return -1;
  }

  return 1;
}

// returns a pointer to either the entry of the key or the next empty entry in the list
static hc_map_entry* hc_map_probe(hc_map* map, const char* key) {
  // fixme: if an entry is set and the capac is increased, then the limiting messes up
  // this sucks
  size_t idx = (size_t) (hc_hash_key(key) & (uint64_t)(map->capacity - 1)); // hash the key and limit it to capacity

  // no bounds/limit checking bc the map should never be completely full
  while (map->entries[idx].key != NULL) { // while the entry at idx is not empty..
    if (strcmp(key, map->entries[idx].key) == 0) { // check if the entry's key is the same as ours
      return &map->entries[idx]; // if so, return that entry
    }

    idx = (idx + 1) % map->capacity; // if not, iterate idx and loop it if its above the map's capacity
  }
  return &map->entries[idx]; // when we find an empty spot, return that spot
}


void* hc_map_get(hc_map* map, const char* key) {
  hc_map_entry* ent = hc_map_probe(map, key);
  if (ent == NULL) return NULL;
  return ent->val;
}

int hc_map_set(hc_map* map, const char* key, void* val) {
  if (val == NULL) return 0;

  if (map->size >= map->capacity / 2) {
    if (!hc_map_expand(map)) return 0;
  }

    return hc_map_set_entry(map, key, val);
}

int hc_map_set_entry(hc_map* map, const char* key, void* val) {
  hc_map_entry* entry_to_set = hc_map_probe(map, key);
  if (entry_to_set->key != NULL) {
    entry_to_set->val = val;
    return 1;
  }
  
  entry_to_set->key = strdup(key);
  entry_to_set->val = val;
  map->size += 1;

  return 1;
}

int hc_map_expand(hc_map* map) {
  hc_map_entry* new_entries = (hc_map_entry*) calloc(map->capacity * 2, sizeof(hc_map_entry));
  if (new_entries == NULL) return 0;
  
  memcpy(new_entries, map->entries, map->capacity * sizeof(hc_map_entry));
  map->capacity *= 2;
  map->entries = new_entries;
  return 1;
}

hc_map_iter hc_map_create_iter(hc_map* map) {
  return (hc_map_iter) {
    .current_entry = map->entries,
    .map = map,
  };
}


hc_map_entry* hc_map_iter_next(hc_map_iter* iter) {
  hc_map* m = iter->map;
  if (iter->current_entry >= m->entries + m->capacity) return NULL; // at end of map
  
  do {
    iter->current_entry++;
  } while (!iter->current_entry->key && iter->current_entry < m->entries + m->capacity);

  if (iter->current_entry >= m->entries + m->capacity) return NULL; // reached end of map

  return iter->current_entry;
}

void hc_map_free(hc_map* map) {
  for (size_t i = 0; i < map->capacity; i++) {
    if (map->entries[i].key != NULL) {
      free(map->entries[i].key);
      free(map->entries[i].val);
    }
  }

  free(map->entries);
}

static uint64_t hash_bytes(const uint8_t* bytes, size_t length) {
  uint64_t hash = HASH_OFFSET;
    for (size_t i = 0; i < length; i++) {
        // xor on its way to carry the entirety of CS on its back
        hash ^= (uint64_t) (unsigned char) (bytes[i]);
        hash *= HASH_PRIME;
    }
    return hash;
}

uint64_t hc_hash_key(const char* key) {
  return hash_bytes((void*) key, strlen(key));
}



