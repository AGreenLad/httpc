#ifndef __HTTPC_CONNECTION_H
#define __HTTPC_CONNECTION_H
#include "socket.h"
#include "response.h"
#include "request.h"
#include "vec.h"

// abstraction of connection for library end-user

typedef struct httpc_conn {
  _hc_socket sock;
  httpc_req req;
  _hc_res res;
  httpc_vec raw_req;
} httpc_conn;

httpc_conn _hc_conn_new(_hc_socket sock);
httpc_req* httpc_get_request(httpc_conn* conn);
int httpc_set_header(httpc_conn* conn, const char* name, const char* val);
int httpc_writef(httpc_conn* conn, int code, const char* content_type, const char* format, ...); // write formatted
int httpc_file(httpc_conn* conn, int code, const char* content_type, const char* path);
int _hc_conn_send(httpc_conn* conn);
int _hc_conn_free(httpc_conn* conn);

#endif