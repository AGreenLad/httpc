#ifndef __REQUEST_H
#define __REQUEST_H

#include "map.h"
#include "vec.h"

typedef enum {
  HC_GET,
  HC_POST,
  HC_PUT,
  HC_DELETE,
  _HC_ERROR
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
char* httpc_get_req_header(httpc_req* req, char* key);
void _hc_req_free(httpc_req* req);

#endif