#include "request.h"

const char* method_strs[] = {
  "GET",
  "POST",
  "PUT",
  "DELETE"
};

char* httpc_req_get_header(hc_req* req, char* key) {
  return (char*) hc_map_get(&(req->headers), key);
}

void httpc_req_print(hc_req* req) {
  printf("Method: %s\nURI: %s\nHeaders: {\n", method_strs[req->method], req->uri);

  hc_map_iter headers_iterator = hc_map_create_iter(&(req->headers));
  hc_map_entry* current_header;
  while ((current_header = hc_map_iter_next(&headers_iterator))) {
    printf("  %s: %s\n", current_header->key, (char*) current_header->val);
  }

  printf("}\nBody Length: %ld\nBody (truncated): %.15s...\n", req->body.length, req->body.data);
}

void _hc_req_free(hc_req* req) {
  hc_map_free(&req->headers);
  hc_vec_free(&req->body);
  free(req->uri);
}