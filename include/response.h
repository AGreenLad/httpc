#ifndef __RESPONSE_H
#define __RESPONSE_H

#include "map.h"
#include "buffer.h"
#include "socket.h"

struct Response {
  int code;
  Map headers;
  Buffer body;
};

typedef struct Response Response;

Response res_new();
Response res_with_code(int code);
int res_code(Response* res, int code);
int res_set_header(Response* res, char* key, char* val);
int res_str(Response* res, int code, char* content, char* content_type);
int res_file(Response* res, int code, char* filename, char* content_type);
int res_file_fp(Response* res, FILE* fp, char* content_type); // same as file but takes a FILE* obj
Buffer res_serialize(Response res);
int res_send(Response res, Socket s);
void res_free(Response* res);

#endif