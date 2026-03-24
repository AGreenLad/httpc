// experimental parsing w/ a lexer
#include "request.h"

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
  BODY
} TokenType;

typedef enum {
  METHOD,
  URI,
  VERSION,
} State;

typedef struct Token {
  TokenType type;
  Buffer dat;
} Token;

typedef struct Lexer {
  char* dat;
  size_t pos;
  char ch;
} Lexer;

Request parse_request(const Buffer raw_req) {
  
}