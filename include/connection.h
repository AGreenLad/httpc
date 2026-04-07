#ifndef __HTTPC_CONNECTION_H
#define __HTTPC_CONNECTION_H
#include "socket.h"
#include "response.h"
#include "request.h"
#include "vec.h"

// abstraction of connection for library end-user

typedef struct hc_Conn {
  Socket sock;
  Request req;
  _hc_res res;
  hc_vec raw_req;
} hc_Conn;

hc_Conn hc_conn_new(Socket sock);
Request* hc_get_request(hc_Conn* conn);
int hc_set_header(hc_Conn* conn, const char* name, const char* val);
int hc_writef(hc_Conn* conn, int code, const char* content_type, const char* format, ...); // write formatted
int hc_write_file(hc_Conn* conn, int code, const char* content_type, const char* path);
int hc_send(hc_Conn* conn);
int hc_conn_free(hc_Conn* conn);

#endif