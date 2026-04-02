#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "request.h"
#include "buffer.h"

typedef struct Lexer {
  char* dat;
  size_t len;
  size_t pos;
  char chr;
} Lexer;

// lexer function signatures
char lexer_read_char(Lexer* lex);
Buffer lexer_read_line(Lexer* lex);
void lexer_next_line(Lexer* lex);
Buffer lexer_read_to(Lexer* lex, char to);
void lexer_skip_whitespace(Lexer* lex);
Buffer lexer_read_to_whitespace(Lexer* lex);

Lexer lexer_from_buf(Buffer buf) {
  return (Lexer) {
    .dat = (char*) buf.data,
    .len = buf.length,
    .chr = (char) *buf.data
  };
}

int parse_request_line(Request* req, Lexer* lex) {
  // first line
  // rfc says to skip whitespace
  lexer_skip_whitespace(lex);

  Buffer method = lexer_read_to(lex, ' '); // faster than read_to_whitespace
  Buffer uri = lexer_read_to(lex, ' ');
  Buffer version = lexer_read_line(lex);


  for (int i = 0; i < 4; i++)
    if (strncmp((char*) method.data, method_strs[i], method.length) == 0) { req->method = (Method) i; break; }
  if (req->method == (Method) -1) return -1;
  
  req->uri = buf_to_str(uri);
  req->version = buf_to_str(version);

  buf_free(&method);
  buf_free(&uri);
  buf_free(&version);

  return 1;
}

int parse_request_headers(Request* req, Lexer* lex) {
  Map headers;
  map_init(&headers);
  // checks for new line or end of request, hacky af tho
  while (*(lex->dat + lex->pos) != '\r' && lex->pos < lex->len) {
    Buffer header_name = lexer_read_to(lex, ':');
    lexer_skip_whitespace(lex); // there will always be a space after the colon
    Buffer header_value = lexer_read_line(lex);
    if (map_set(&headers, (char*) header_name.data, (void*) buf_to_str(header_value)) == 0) {
      buf_free(&header_name);
      buf_free(&header_value);
      return -1;
    }

    buf_free(&header_name);
    buf_free(&header_value);
  }
  
  req->headers = headers;
  return 1;
}

Request req_parse_request(const Buffer raw_req) {
  Request req = { .method = -1 };

  Lexer lex = lexer_from_buf(raw_req);

  if (parse_request_line(&req, &lex) == -1) 
    return (Request) { .method = ERROR };

  if (parse_request_headers(&req, &lex) == -1)
    return (Request) { .method = ERROR };

  lexer_next_line(&lex);
  Buffer body = buf_new();
  if (lex.len > lex.pos)
    buf_append(&body, lex.dat + lex.pos, lex.len - lex.pos); // just append the rest of the data

  req.body = body;
  return req;
}

// lexer functions

char lexer_read_char(Lexer* lex) {
  if (lex->pos >= lex->len) return 0; // null indicates eof
  lex->chr = *(lex->dat + lex->pos++);
  return lex->chr; // so we dont have to lex->chr after every char read
}

Buffer lexer_read_line(Lexer* lex) {
  Buffer line = buf_new();

  while (lex->pos < lex->len) {
    lexer_read_char(lex);
    if (lex->chr == '\r') { 
      if (lexer_read_char(lex) == '\n')
        return line;
    } else buf_append_one(&line, lex->chr);
  }

  return line;
}

void lexer_next_line(Lexer* lex) {
  while (lex->pos < lex->len) {
    if (lex->chr == '\r') { 
      if (lexer_read_char(lex) == '\n')
        return;
    }
    lexer_read_char(lex);
  }

  return;
}

Buffer lexer_read_to(Lexer* lex, char to) {
  Buffer content = buf_new();
  while (lex->pos < lex->len && lexer_read_char(lex) != to) {
    buf_append_one(&content, lex->chr);
  }
  return content;
}

void lexer_skip_whitespace(Lexer* lex) {
  while (isspace(*(lex->dat + lex->pos)) && lex->pos < lex->len) lexer_read_char(lex);
  return;
}