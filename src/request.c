#include "request.h"

const char* method_strs[] = {
  "GET",
  "POST",
  "PUT",
  "DELETE"
};

// bad dont use pls
Request req_parse_old(const Buffer raw_req) {
  // get offset to body to section off headers from it
  char* header_end = strstr((char*) raw_req.data, "\r\n\r\n");
  char* body_ptr = header_end + 4;
  int body_offset = body_ptr - (char*) raw_req.data;
  int header_end_offset = body_offset - 4;

  char* req_no_body = calloc(sizeof(char), (size_t) header_end_offset + 1);
  memcpy(req_no_body, raw_req.data, (size_t) header_end_offset);
  req_no_body[header_end_offset] = 0;

  char* line = strtok(req_no_body, "\r\n");
  
  // get method and resource
  char method_str[6];
  char* uri = calloc(sizeof(char), 50);
  if (sscanf(line, "%5s %50s", method_str, uri) < 0) { perror("sscanf() failed while parsing method and URI"); exit(EXIT_FAILURE); }
  
  Method method;
  for (int i = 0; i < 4; i++) {
    if (strcmp(method_str, method_strs[i]) == 0) { method = (Method) i; break; }
  }

  // get headers
  Map headers;
  map_init(&headers);

  line = strtok(NULL, "\r\n");
  char k[128];
  char v[768];

  while (line != NULL) {
    if (sscanf(line, "%128[^:]: %768[^\r]", k, v) < 0) {
      perror("sscanf() failed while parsing headers");
      exit(EXIT_FAILURE);
    }
    
    map_set(&headers, k, strdup(v));
    line = strtok(NULL, "\r\n");
  }

  free(req_no_body);

  // get body
  Buffer body = buf_from_string(body_ptr); // pls be zero terminated :pray:

  // construct final req
  Request r = {
    .method = method,
    .uri = uri,
    .headers = headers,
    .body = body
  };

  return r;
}


char* req_get_header(Request* req, char* key) {
  return (char*) map_get(&(req->headers), key);
}

void req_print(Request* req) {
  printf("Method: %s\nURI: %s\nHeaders: {\n", method_strs[req->method], req->uri);

  MapIter headers_iterator = map_create_iter(&(req->headers));
  Entry* current_header;
  while ((current_header = map_iter_next(&headers_iterator))) {
    printf("  %s: %s\n", current_header->key, (char*) current_header->val);
  }

  printf("}\nBody Length: %ld\nBody (truncated): %.15s...\n", req->body.length, req->body.data);
}

void req_free(Request* req) {
  map_free(&req->headers);
  buf_free(&req->body);
  free(req->uri);
}