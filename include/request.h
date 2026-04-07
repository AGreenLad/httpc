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
} hc_method;

extern const char* method_strs[];

typedef struct {
  hc_method method;
  char* uri;
  char* version;
  hc_map headers;
  hc_vec body;
} hc_req;

hc_req _hc_req_parse(const hc_vec raw_req);
char* httpc_req_get_header(hc_req* req, char* key);
void httpc_req_print(hc_req* req);
void _hc_req_free(hc_req* req);

#endif