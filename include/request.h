#ifndef __REQUEST_H
#define __REQUEST_H

#include "map.h"
#include "buffer.h"

typedef enum {
  GET,
  POST,
  PUT,
  DELETE
} Method;

typedef struct {
  Method method;
  char* uri;
  Map headers;
  Buffer body;
} Request;

Request req_parse(const Buffer raw_req);
// v **** IMPLEMENT **** v
char* req_uri(Request* req);
char* req_get_header(Request* req, char* key);
Buffer* req_body(Request* req);
Method req_method(Request* req);
// ^ ******************* ^
void req_print(Request* req);
void req_free(Request* req);

#endif