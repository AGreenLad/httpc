// using this internally is prolly fine but its prob for the better to
// just return a char* and length separately
// or this could all just be bad practice and i suck at c
// oh well

// todo header shadowing? or whatever its called
#ifndef __BUFFER_H
#define __BUFFER_H

#include <stdlib.h>
#include <stdint.h>

struct Buffer {
  size_t length;
  size_t capacity;
  uint8_t* data;
};

typedef struct Buffer Buffer;

Buffer buf_new();
void buf_init(Buffer* buf);
void buf_clear(Buffer* buf);
int buf_reserve(Buffer* buf, size_t capac);
int buf_append(Buffer* buf, void* dat, size_t n);
int buf_append_one(Buffer* buf, uint8_t v);
int buf_read_file(Buffer* buf, char* filename);
Buffer buf_from_string(char* str);
Buffer buf_concat(const Buffer b1, const Buffer b2);
int buf_concat_to(Buffer* b1, const Buffer b2);
int buf_concat_str(Buffer* buf, const char* str);
char* buf_to_str(Buffer buf);
void buf_free(Buffer* buf);

#endif