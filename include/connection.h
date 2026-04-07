#ifndef __HTTPC_CONNECTION_H
#define __HTTPC_CONNECTION_H
#include "socket.h"
#include "response.h"
#include "request.h"
#include "vec.h"

// abstraction of connection for library end-user

typedef struct hc_conn {
  _hc_socket sock;
  hc_req req;
  _hc_res res;
  hc_vec raw_req;
} hc_conn;

hc_conn hc_conn_new(_hc_socket sock);
Request* hc_get_request(hc_conn* conn);
int hc_set_header(hc_conn* conn, const char* name, const char* val);
int hc_writef(hc_conn* conn, int code, const char* content_type, const char* format, ...); // write formatted
int hc_write_file(hc_conn* conn, int code, const char* content_type, const char* path);
int hc_send(hc_conn* conn);
int hc_conn_free(hc_conn* conn);

#endif