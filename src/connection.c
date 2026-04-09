#include <stdarg.h>
#include "connection.h"
#include "socket.h"
#include "response.h"
#include "request.h"
#include "vec.h"

httpc_conn _hc_conn_new(_hc_socket sock) {
  httpc_conn conn = {.sock = sock};
  conn.raw_req = _hc_socket_recv(sock);
  conn.req = _hc_req_parse(conn.raw_req);
  conn.res = _hc_res_new();
  return conn;
}

httpc_req* httpc_get_request(httpc_conn* conn) {
  return &(conn->req);
}

int httpc_set_header(httpc_conn* conn, const char* name, const char* val) {
  return _hc_res_set_header(&conn->res, name, val);
};

// rewrite to handle multiple writes w/o resetting
int httpc_writef(httpc_conn* conn, int code, const char* content_type, const char* format, ...) {
  char content[DEFAULT_BUFFER_SIZE];

  va_list args;
  va_start(args, format);
  vsnprintf(content, DEFAULT_BUFFER_SIZE, format, args);
  va_end(args);

  _hc_res_str(&conn->res, code, content);
  return 1;
}

int httpc_file(httpc_conn* conn, int code, const char* content_type, const char* path) {

}

int _hc_conn_send(httpc_conn* conn);
int _hc_conn_free(httpc_conn* conn);