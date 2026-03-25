// experimental parsing w/ a lexer
#include <stdio.h>
#include <string.h>
#include "request.h"
#include "buffer.h"

typedef enum {
  // /r/n/r/n
  NEWLINE,
  // first line tokens
  METHOD,
  URI,
  VERSION,
  // header tokens
  HEADER_NAME,
  COLON,
  HEADER_VAL,
  // body
  BODY,
  // etc.
  END,
  ERROR,
  EOF
} TokenType;

typedef enum {
  // request line
  RL_METHOD
  RL_URI
  RL_VERSION
  // headers
  HDR_HEADER_NAME,
  HDR_HEADER_VAL,
  // ...body
  B_BODY
} State;

typedef struct Token {
  TokenType type;
  Buffer dat;
} Token;

typedef struct Lexer {
  char* dat;
  size_t len;
  size_t pos;
  char chr;
} Lexer;

// lexer function signatures
char lexer_read_char(Lexer* lex);
char lexer_peek_char(Lexer* lex);
Buffer lexer_read_line(Lexer* lex);
Buffer lexer_read_to(Lexer* lex, char to);
void lexer_skip_whitespace(Lexer* lex);
Buffer lexer_read_to_whitespace(Lexer* lex);

// fsm functions
Token lex_req_line(Lexer* lex) {
  switch
}

Token lex_headers(Lexer* lex) {
  // todo
}

Token lex_body(Lexer* lex) {
  
}

Token read_next_token(Lexer* lex, State st) {
  switch (st) {
    case RL_METHOD:
    case RL_URI:
    case RL_VERSION:
      return lex_req_line(lex, st);
    case HDR_HEADER_NAME:
    case HDR_HEADER_VAL:
      return lex_headers(lex, st);
    case B_BODY:
      return lex_body(lex, st);
    default:
      return {
        .type: ERROR
        .buf: NULL
      };
  }
}

Request parse_request(const Buffer raw_req) {
  Lexer lex = {
    .dat = raw_req.data;
    .len = raw_req.len;
    .pos = 0;
    .chr = 0;
  };

  State curr_state = FIRST_LINE;
  Token curr_token;

  while (State != END) {
    Token tok = read_next_token(lex, curr_state);
  }
  
}

// lexer functions

char lexer_read_char(Lexer* lex) {
  if (lex->pos + 1 >= lex->len) return 0; // null indicates eof
  lex->chr = *(lex->dat + lex->pos++);
  return lex->chr; // so we dont have to lex->chr after every char read
}

char lexer_peek_char(Lexer* lex) {
  return *(lex->dat + lex->pos);
}

Buffer lexer_read_line(Lexer* lex) {
  Buffer line = buf_new();

  while (lex->pos < lex->len) {
    char c = lexer_read_char(lex);
    if (c == "\r") { 
      if (lexer_read_char(lex) == "\n") return line;
    } else buf_append_one(&line, c);
  }
}

Buffer lexer_read_to(Lexer* lex, char to) {
  Buffer content = buf_new();
  while (lexer_read_char(lex) != to && lex->pos < lex->len) {
    buf_append_one(&content, lex->chr);
  }
  return content;
}

void lexer_skip_whitespace(Lexer* lex) {
  while (isspace(lex->chr) && lex->pos < lex->len) lexer_read_char(lex);
  return;
}

Buffer lexer_read_to_whitespace(Lexer* lex) {
  Buffer content = buf_new();
  while (isspace(lexer_read_char(lex)) && lex->pos < lex->len) buf_append_one(&content, lex->chr);
  return content;
}