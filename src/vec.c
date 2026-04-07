#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "vec.h"

hc_vec hc_vec_new() {
  hc_vec b;
  hc_vec_init(&b);
  return b;
}

void hc_vec_init(hc_vec* buf) {
  buf->length = 0;
  buf->capacity = 64;
  buf->data = calloc(sizeof(uint8_t), 64);
}

void hc_vec_clear(hc_vec* buf) {
  memset(buf->data, 0, buf->length);
  buf->length = 0;
  return;
}

int hc_vec_reserve(hc_vec* buf, size_t capac) {
  if (buf->capacity >= capac) return 0;
  if ((buf->data = realloc(buf->data, capac)) == NULL) return -1;
  buf->capacity = capac;
  return 1;
}

int hc_vec_append(hc_vec* buf, void* dat, size_t n) {
  size_t new_size = buf->length + n;
  if (new_size >= buf->capacity) {
    hc_vec_reserve(buf, new_size);
  }

  memcpy(buf->data + buf->length, dat, n);
  buf->length = new_size;
  return 1;
}

int hc_vec_append_one(hc_vec* buf, uint8_t v) {
  if (buf->length >= buf->capacity) {
    hc_vec_reserve(buf, buf->capacity * 2);
  }

  buf->data[buf->length++] = v;
  return 1;
}

int hc_vec_read_file(hc_vec* buf, char* filename) {
  // this should only be called on a new buf object, never an already initialized one
  // hc_vec_from_file may be better?
  FILE* fp = fopen(filename, "rb");
  if (fp == NULL) {
    if (errno == ENOENT) {
      perror("hc_vec_read_file: no such file");
      return 0;
    }
    perror("fopen() failed");
    return -1;
  }
  
  struct stat fileinfo;
  if (fstat(fileno(fp), &fileinfo) < 0) { perror("fstat() failed"); exit(EXIT_FAILURE); }
  int length = (size_t) fileinfo.st_size;
  buf->length = length;
  buf->capacity = length;
  buf->data = malloc((size_t) length * sizeof(uint8_t));
  // maybe not a good idea for bigger file sizes
  if (fread(buf->data, sizeof(uint8_t), length, fp) != (size_t) length) { perror("fread() failed"); exit(EXIT_FAILURE); }

  fclose(fp);

  return 1;
}

hc_vec hc_vec_from_string(char* str) {
  return (hc_vec) {
    .length = strlen(str),
    .capacity = strlen(str),
    .data = (uint8_t*) strdup(str)
  };
}

hc_vec hc_vec_concat(const hc_vec b1, const hc_vec b2) {
  hc_vec buf;
  hc_vec_init(&buf);

  hc_vec_reserve(&buf, b1.length + b2.length);
  hc_vec_concat_to(&buf, b1);
  hc_vec_concat_to(&buf, b2);
  return buf;
}

int hc_vec_concat_to(hc_vec* b1, const hc_vec b2) {
  return hc_vec_append(b1, (void*) b2.data, b2.length);
}

int hc_vec_concat_str(hc_vec* buf, const char* str) {
  return hc_vec_append(buf, (void*) str, strlen(str));
}

char* hc_vec_to_str(hc_vec buf) {
  char* str = calloc(sizeof(char), buf.length + 1);
  memcpy(str, buf.data, buf.length);
  str[buf.length] = 0;
  return str;
}

void hc_vec_free(hc_vec* buf) {
  free(buf->data);
}