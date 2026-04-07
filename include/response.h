#ifndef __RESPONSE_H
#define __RESPONSE_H

#include "map.h"
#include "vec.h"
#include "socket.h"

typedef struct _hc_res {
  int code;
  Map headers;
  hc_vec body;
} _hc_res;


_hc_res _hc_res_new();
_hc_res _hc_res_with_code(int code);
int _hc_res_code(_hc_res* res, int code);
int _hc_res_set_header(_hc_res* res, char* key, char* val);
int _hc_res_str(_hc_res* res, int code, char* content, char* content_type);
int _hc_res_file(_hc_res* res, int code, char* filename, char* content_type);
int _hc_res_file_fp(_hc_res* res, FILE* fp, char* content_type); // same as file but takes a FILE* obj
hc_vec _hc_res_serialize(_hc_res res);
int _hc_res_send(_hc_res res, Socket s);
void _hc_res_free(_hc_res* res);

#endif