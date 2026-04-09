#ifndef __REQUEST_H
#define __REQUEST_H

#include "map.h"
#include "vec.h"

typedef enum {
  HTTPC_GET,
  HTTPC_POST,
  HTTPC_PUT,
  HTTPC_DELETE,
  HTTPC_MERROR
} httpc_method;

extern const char* _hc_method_strs[];

typedef struct {
  httpc_method method;
  char* uri;
  char* version;
  hc_map headers;
  hc_vec body;
} httpc_req;

httpc_req _hc_req_parse(const hc_vec raw_req);
char* httpc_req_get_header(httpc_req* req, char* key);
void httpc_req_print(httpc_req* req);
void _hc_req_free(httpc_req* req);

#endif