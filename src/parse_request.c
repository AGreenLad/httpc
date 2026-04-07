#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "request.h"
#include "vec.h"

typedef struct hc_lexer {
  char* dat;
  size_t len;
  size_t pos;
  char chr;
} hc_lexer;

// lexer function signatures
char lexer_read_char(hc_lexer* lex);
hc_vec lexer_read_line(hc_lexer* lex);
void lexer_next_line(hc_lexer* lex);
hc_vec lexer_read_to(hc_lexer* lex, char to);
void lexer_skip_whitespace(hc_lexer* lex);
hc_vec lexer_read_to_whitespace(hc_lexer* lex);

hc_lexer lexer_from_buf(hc_vec buf) {
  return (hc_lexer) {
    .dat = (char*) buf.data,
    .len = buf.length,
    .chr = (char) *buf.data
  };
}

int parse_request_line(hc_req* req, hc_lexer* lex) {
  // rfc says to skip whitespace
  lexer_skip_whitespace(lex);

  hc_vec method = lexer_read_to(lex, ' '); // faster than read_to_whitespace
  hc_vec uri = lexer_read_to(lex, ' ');
  hc_vec version = lexer_read_line(lex);


  for (int i = 0; i < 4; i++)
    if (strncmp((char*) method.data, method_strs[i], method.length) == 0) { req->method = (hc_method) i; break; }
  if (req->method == (hc_method) -1) return -1;
  
  req->uri = hc_vec_to_str(uri);
  req->version = hc_vec_to_str(version);

  hc_vec_free(&method);
  hc_vec_free(&uri);
  hc_vec_free(&version);

  return 1;
}

int parse_request_headers(hc_req* req, hc_lexer* lex) {
  hc_map headers;
  hc_map_init(&headers);
  // checks for new line or end of request, hacky af tho
  while (*(lex->dat + lex->pos) != '\r' && lex->pos < lex->len) {
    hc_vec header_name = lexer_read_to(lex, ':');
    lexer_skip_whitespace(lex); // there will always be a space after the colon
    hc_vec header_value = lexer_read_line(lex);
    if (hc_map_set(&headers, (char*) header_name.data, (void*) hc_vec_to_str(header_value)) == 0) {
      hc_vec_free(&header_name);
      hc_vec_free(&header_value);
      return -1;
    }

    hc_vec_free(&header_name);
    hc_vec_free(&header_value);
  }
  
  req->headers = headers;
  return 1;
}

hc_req _hc_req_parse(const hc_vec raw_req) {
  hc_req req = { .method = -1 };

  hc_lexer lex = lexer_from_buf(raw_req);

  if (parse_request_line(&req, &lex) == -1) 
    return (hc_req) { .method = ERROR };

  if (parse_request_headers(&req, &lex) == -1)
    return (hc_req) { .method = ERROR };

  lexer_next_line(&lex);
  hc_vec body = hc_vec_new();
  if (lex.len > lex.pos)
    hc_vec_append(&body, lex.dat + lex.pos, lex.len - lex.pos); // just append the rest of the data

  req.body = body;
  return req;
}

// lexer functions

char lexer_read_char(hc_lexer* lex) {
  if (lex->pos >= lex->len) return 0; // null indicates eof
  lex->chr = *(lex->dat + lex->pos++);
  return lex->chr; // so we dont have to lex->chr after every char read
}

hc_vec lexer_read_line(hc_lexer* lex) {
  hc_vec line = hc_vec_new();

  while (lex->pos < lex->len) {
    lexer_read_char(lex);
    if (lex->chr == '\r') { 
      if (lexer_read_char(lex) == '\n')
        return line;
    } else hc_vec_append_one(&line, lex->chr);
  }

  return line;
}

void lexer_next_line(hc_lexer* lex) {
  while (lex->pos < lex->len) {
    if (lex->chr == '\r') { 
      if (lexer_read_char(lex) == '\n')
        return;
    }
    lexer_read_char(lex);
  }

  return;
}

hc_vec lexer_read_to(hc_lexer* lex, char to) {
  hc_vec content = hc_vec_new();
  while (lex->pos < lex->len && lexer_read_char(lex) != to) {
    hc_vec_append_one(&content, lex->chr);
  }
  return content;
}

void lexer_skip_whitespace(hc_lexer* lex) {
  while (isspace(*(lex->dat + lex->pos)) && lex->pos < lex->len) lexer_read_char(lex);
  return;
}