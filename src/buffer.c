#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "buffer.h"

Buffer buf_new() {
  Buffer b;
  buf_init(&b);
  return b;
}

void buf_init(Buffer* buf) {
  buf->length = 0;
  buf->capacity = 64;
  buf->data = calloc(sizeof(uint8_t), 64);
}

void buf_clear(Buffer* buf) {
  memset(buf->data, 0, buf->length);
  buf->length = 0;
  return;
}

int buf_reserve(Buffer* buf, size_t capac) {
  if (buf->capacity >= capac) return 0;
  if ((buf->data = realloc(buf->data, capac)) == NULL) return -1;
  buf->capacity = capac;
  return 1;
}

int buf_append(Buffer* buf, void* dat, size_t n) {
  size_t new_size = buf->length + n;
  if (new_size >= buf->capacity) {
    buf_reserve(buf, new_size);
  }

  memcpy(buf->data + buf->length, dat, n);
  buf->length = new_size;
  return 1;
}

int buf_append_one(Buffer* buf, uint8_t v) {
  if (buf->length >= buf->capacity) {
    buf_reserve(buf, buf->capacity * 2);
  }

  buf->data[buf->length++] = v;
  return 1;
}

int buf_read_file(Buffer* buf, char* filename) {
  // this should only be called on a new buf object, never an already initialized one
  // buf_from_file may be better?
  FILE* fp = fopen(filename, "rb");
  if (fp == NULL) {
    if (errno == ENOENT) {
      perror("buf_read_file: no such file");
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

Buffer buf_from_string(char* str) {
  return (Buffer) {
    .length = strlen(str),
    .capacity = strlen(str),
    .data = (uint8_t*) strdup(str)
  };
}

Buffer buf_concat(const Buffer b1, const Buffer b2) {
  Buffer buf;
  buf_init(&buf);

  buf_reserve(&buf, b1.length + b2.length);
  buf_concat_to(&buf, b1);
  buf_concat_to(&buf, b2);
  return buf;
}

int buf_concat_to(Buffer* b1, const Buffer b2) {
  return buf_append(b1, (void*) b2.data, b2.length);
}

int buf_concat_str(Buffer* buf, const char* str) {
  return buf_append(buf, (void*) str, strlen(str));
}

char* buf_to_str(Buffer buf) {
  char* str = calloc(sizeof(char), buf.length + 1);
  memcpy(str, buf.data, buf.length);
  str[buf.length] = 0;
  return str;
}

void buf_free(Buffer* buf) {
  free(buf->data);
}