// using this internally is prolly fine but its prob for the better to just return a char* and length separately
// or this could all just be bad practice and i suck at c
// oh well

#ifndef __VEC_H
#define __VEC_H

#include <stdlib.h>
#include <stdint.h>

typedef struct hc_vec {
  size_t length;
  size_t capacity;
  uint8_t* data;
} hc_vec;

hc_vec hc_vec_new();
void hc_vec_init(hc_vec* buf);
void hc_vec_clear(hc_vec* buf);
int hc_vec_reserve(hc_vec* buf, size_t capac);
int hc_vec_append(hc_vec* buf, void* dat, size_t n);
int hc_vec_append_one(hc_vec* buf, uint8_t v);
int hc_vec_read_file(hc_vec* buf, char* filename);
hc_vec hc_vec_from_string(char* str);
hc_vec hc_vec_concat(const hc_vec b1, const hc_vec b2);
int hc_vec_concat_to(hc_vec* b1, const hc_vec b2);
int hc_vec_concat_str(hc_vec* buf, const char* str);
char* hc_vec_to_str(hc_vec buf);
void hc_vec_free(hc_vec* buf);

#endif