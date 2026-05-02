// this will have way more stuff when i get to it
#include "request.h"

const char* _hc_method_strs[] = {
  "GET",
  "POST",
  "PUT",
  "DELETE"
};

char* httpc_get_req_header(httpc_req* req, char* key) {
  return (char*) hc_map_get(&(req->headers), key);
}

void _hc_req_free(httpc_req* req) {
  hc_map_free(&req->headers);
  hc_vec_free(&req->body);
  free(req->uri);
}