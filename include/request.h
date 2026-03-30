#ifndef __REQUEST_H
#define __REQUEST_H

#include "map.h"
#include "buffer.h"

typedef enum {
  GET,
  POST,
  PUT,
  DELETE,
  ERROR
} Method;

extern const char* method_strs[];

typedef struct {
  Method method;
  char* uri;
  char* version;
  Map headers;
  Buffer body;
} Request;

Request req_parse_request(const Buffer raw_req);
char* req_get_header(Request* req, char* key);
void req_print(Request* req);
void req_free(Request* req);

#endif