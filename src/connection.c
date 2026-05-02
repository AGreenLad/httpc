#include <stdarg.h>
#include <errno.h>
#include "connection.h"
#include "socket.h"
#include "response.h"
#include "request.h"
#include "vec.h"
#include "log.h"

#define _HC_LOG_MODULE "CONN"

httpc_conn _hc_conn_new(_hc_socket* sock) {
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

int httpc_writef(httpc_conn* conn, int code, const char* content_type, const char* format, ...) {
  hc_vec body = hc_vec_new();
  hc_vec_reserve(&body, DEFAULT_BUFFER_SIZE);
  
  va_list args;
  va_start(args, format);

  int chars_read = 0;
  while ((chars_read = vsnprintf((char*) body.data + (ptrdiff_t) chars_read, DEFAULT_BUFFER_SIZE, format, args)) == DEFAULT_BUFFER_SIZE) {
    body.length += DEFAULT_BUFFER_SIZE;
    hc_vec_reserve(&body, body.capacity + DEFAULT_BUFFER_SIZE);
  }

  if (chars_read < 0) {
    LOG_ERROR("Failed to write body: %s", strerror(errno));
  }

  body.length += chars_read;
  va_end(args);
  _hc_res_str(&conn->res, code, hc_vec_to_str(body), content_type);
  return 1;
}

int httpc_file(httpc_conn* conn, int code, const char* content_type, const char* path, const char* prefix) {
  size_t filename_len = strlen(path) + strlen(prefix) + 1;
  char* filename = calloc(sizeof(char), filename_len);
  snprintf(filename, filename_len, "%s%s", prefix, path);

  int res = _hc_res_file(&(conn->res), code, filename, content_type);
  free(filename);

  if (res == 0) {
    _hc_res_str(&(conn->res), 404, "404 Not Found", "text/plain");
    return 1;
  } else if (res == -1) {
    _hc_res_str(&(conn->res), 500, "500 Internal Server Error", "text/plain");
    return -1;
  }

  return 1;
}

void _hc_conn_send(httpc_conn* conn) {
  _hc_res_send(conn->res, conn->sock);
}

void _hc_conn_free(httpc_conn* conn) {
  hc_vec_free(&conn->raw_req);
  _hc_req_free(&conn->req);
  _hc_res_free(&conn->res);
  _hc_socket_ready(conn->sock);
}