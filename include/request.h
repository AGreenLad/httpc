#ifndef __REQUEST_H
#define __REQUEST_H

#include "map.h"
#include "vec.h"

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
  hc_vec body;
} Request;

Request _hc_req_parse(const hc_vec raw_req);
char* httpc_req_get_header(Request* req, char* key);
void httpc_req_print(Request* req);
void _hc_req_free(Request* req);

#endif