#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "response.h"

Response res_new() {
  return (Response) {
    .headers = map_new(),
    .body = buf_new()
  };
}

Response res_with_code(int code) {
  Response res = res_new();
  res.code = code;
  return res;
}

// most necessary function ever
int res_code(Response* res, int code) {
  res->code = code;
  return 1;
}

char* res_get_header(Response* res, char* key) {
  return (char*) map_get(&res->headers, key);
}

int res_set_header(Response* res, char* key, char* val) {
  if (map_set(&res->headers, key, strdup(val)) != 0)
    return 1;
  else return -1;
}


int res_str(Response* res, int code, char* content, char* content_type) {
  res->code = code;
  
  char len[10];
  if (snprintf(len, sizeof(len), "%lu", strlen(content)) < 0) {
    perror("snprintf() in res_str for transferring length failed");
    exit(EXIT_FAILURE);
  }
  res_set_header(res, "Content-Type", strdup(content_type));
  res_set_header(res, "Content-Length", strdup(len));
  res->body = buf_from_string(content);
  return 1;
}

int res_file(Response* res, int code, char* filename, char* content_type) {
  res->code = code;

  Buffer* res_body = &(res->body);
  int status = buf_read_file(res_body, filename);

  if (status == 0) {
    buf_clear(res_body);
    return 0;
  } else if (status < 0) {
    buf_clear(res_body);
    return -1;
  }

  res->code = 200;
  res_set_header(res, "Content-Type", strdup(content_type));

  char filelen[21];
  if (snprintf(filelen, sizeof(filelen), "%lu", res_body->length) < 0) {
    perror("snprintf() in res_file for transferring content length failed");
    exit(EXIT_FAILURE);
  }
  // add chunked encoding for bigger files?
  res_set_header(res, "Content-Length", strdup(filelen));

  return 1;
}

Buffer res_serialize(Response res) {
  Buffer res_buffer = buf_new();
  buf_reserve(&res_buffer, KiB(2));
  
  // first line (version, status code)
  // todo add reason phrase?
  char first_line[20];
  snprintf(first_line, 20, "HTTP/1.1 %d\r\n", res.code);
  buf_concat_str(&res_buffer, first_line);

  // headers
  MapIter headers_iter = map_create_iter(&res.headers);
  Entry* current_header;
  char current_header_str[150];
  while ((current_header = map_iter_next(&headers_iter))) {
    snprintf(current_header_str, 150, "%s: %s\r\n", current_header->key, (char*) current_header->val);
    buf_concat_str(&res_buffer, current_header_str);
  }
  
  // body
  buf_concat_str(&res_buffer, "\r\n");
  buf_concat_to(&res_buffer, res.body);

  return res_buffer;
}

int res_send(Response res, Socket s) {
  Buffer res_buf = res_serialize(res);
  socket_send(s, res_buf);
  return 1;
}

void res_free(Response* res) {
  map_free(&res->headers);
  buf_free(&res->body);
}