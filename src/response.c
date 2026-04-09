#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "response.h"

_hc_res _hc_res_new() {
  return (_hc_res) {
    .headers = hc_map_new(),
    .body = hc_vec_new()
  };
}

_hc_res _hc_res_with_code(int code) {
  _hc_res res = _hc_res_new();
  res.code = code;
  return res;
}

// most necessary function ever
int _hc_res_code(_hc_res* res, int code) {
  res->code = code;
  return 1;
}

char* _hc_res_get_header(_hc_res* res, char* key) {
  return (char*) hc_map_get(&res->headers, key);
}

int _hc_res_set_header(_hc_res* res, char* key, char* val) {
  if (hc_map_set(&res->headers, key, strdup(val)) != 0)
    return 1;
  else return -1;
}


int _hc_res_str(_hc_res* res, int code, char* content, char* content_type) {
  res->code = code;
  
  char new_len[15]; // 14 digits = 100 terabytes - 1 byte, if we go over that we have already fucked up somewhere
  if (snprintf(new_len, sizeof(new_len), "%lu", res->body.length + strlen(content)) < 0) {
    perror("snprintf() in res_str for transferring length failed");
    exit(EXIT_FAILURE);
  }

  _hc_res_set_header(res, "Content-Type", strdup(content_type));
  _hc_res_set_header(res, "Content-Length", strdup(new_len));
  hc_vec_concat_str(&(res->body), content);
  return 1;
}

int _hc_res_file(_hc_res* res, int code, char* filename, char* content_type) {
  res->code = code;

  hc_vec file_vec = hc_vec_new();
  int status = hc_vec_read_file(&file_vec, filename);

  if (status == 0) {
    return 0;
  } else if (status < 0) {
    return -1;
  }

  _hc_res_set_header(res, "Content-Type", strdup(content_type));

  char filelen[15];
  if (snprintf(filelen, sizeof(filelen), "%lu", res->body.length + file_vec.length) < 0) {
    perror("snprintf() in res_file for transferring content length failed");
    exit(EXIT_FAILURE);
  }
  // add chunked encoding for bigger files?
  _hc_res_set_header(res, "Content-Length", strdup(filelen));
  hc_vec_concat_to(&(res->body), file_vec);

  return 1;
}

hc_vec _hc_res_serialize(_hc_res res) {
  hc_vec res_vec = hc_vec_new();
  hc_vec_reserve(&res_vec, KiB(2));
  
  // first line (version, status code)
  // todo add reason phrase?
  char first_line[20];
  snprintf(first_line, 20, "HTTP/1.1 %d\r\n", res.code);
  hc_vec_concat_str(&res_vec, first_line);

  // headers
  hc_map_iter headers_iter = hc_map_create_iter(&res.headers);
  hc_map_entry* current_header;
  char current_header_str[150];
  while ((current_header = hc_map_iter_next(&headers_iter))) {
    snprintf(current_header_str, 150, "%s: %s\r\n", current_header->key, (char*) current_header->val);
    hc_vec_concat_str(&res_vec, current_header_str);
  }
  
  // body
  hc_vec_concat_str(&res_vec, "\r\n");
  hc_vec_concat_to(&res_vec, res.body);

  return res_vec;
}

int _hc_res_send(_hc_res res, _hc_socket s) {
  hc_vec res_buf = _hc_res_serialize(res);
  _hc_socket_send(s, res_buf);
  return 1;
}

void _hc_res_free(_hc_res* res) {
  hc_map_free(&res->headers);
  hc_vec_free(&res->body);
}