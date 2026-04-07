#include <stdarg.h>
#include "connection.h"
#include "socket.h"
#include "response.h"
#include "request.h"
#include "vec.h"

hc_conn hc_conn_new(_hc_socket sock) {
  hc_conn conn = {.sock = sock};
  conn.raw_req = _hc_socket_recv(sock);
  conn.req = req_parse(conn.raw_req);
  conn.res = _hc_res_new();
  return conn;
}

Request* hc_get_request(hc_conn* conn) {
  return &(conn->req);
}

int hc_set_header(hc_conn* conn, const char* name, const char* val) {
  return _hc_res_set_header(&conn->res, name, val);
};

// rewrite to handle multiple writes w/o resetting
int hc_writef(hc_conn* conn, int code, const char* content_type, const char* format, ...) {
  char content[DEFAULT_BUFFER_SIZE];

  va_list args;
  va_start(args, format);
  vsnprintf(content, DEFAULT_BUFFER_SIZE, format, args);
  va_end(args);

  _hc_res_str(&conn->res, code, content);
  return 1;
}

int hc_write_file(hc_conn* conn, int code, const char* content_type, const char* path) {

}

int hc_send(hc_conn* conn);
int hc_conn_free(hc_conn* conn);