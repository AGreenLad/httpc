#include "request.h"

const char* _hc_method_strs[] = {
  "GET",
  "POST",
  "PUT",
  "DELETE"
};

char* httpc_req_get_header(httpc_req* req, char* key) {
  return (char*) hc_map_get(&(req->headers), key);
}

void httpc_req_print(httpc_req* req) {
  printf("Method: %s\nURI: %s\nHeaders: {\n", _hc_method_strs[req->method], req->uri);

  hc_map_iter headers_iterator = hc_map_create_iter(&(req->headers));
  hc_map_entry* current_header;
  while ((current_header = hc_map_iter_next(&headers_iterator))) {
    printf("  %s: %s\n", current_header->key, (char*) current_header->val);
  }

  printf("}\nBody Length: %ld\nBody (first 30 chars): %.30s...\n", req->body.length, req->body.data);
}

void _hc_req_free(httpc_req* req) {
  hc_map_free(&req->headers);
  hc_vec_free(&req->body);
  free(req->uri);
}