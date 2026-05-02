/* ***************************************************
==============================
============ HTTPC ===========
==============================

http.c
This is test code for the modules of this project and
likely will not reflect how it should be used.

-AGreenLad (am i doing this banner stuff right?)
*************************************************** */
#include <unistd.h>
#include <errno.h>
#include "map.h"
#include "vec.h"
#include "connection.h"
#include "request.h"
#include "response.h"
#include "socket.h"
#include "tpool.h"
#include "log.h"

#define TPOOL_WORKERS 24
#define _HC_LOG_MODULE "SERVER"
#define LOG_PATH "log/"
#define LOG_NAME "httpc-log"

// maybe use an arena for each request
void handle_request(_hc_socket* clsock) {

  httpc_conn conn = _hc_conn_new(clsock);

  httpc_set_header(&conn, "Server", "httpc/0.1");
  httpc_set_header(&conn, "Connection", "Keep-Alive");

  // start of proper request code
  httpc_req* req_info = httpc_get_request(&conn);

  if (strncmp(req_info->uri, "/static", 7) == 0)
    httpc_file(&conn, 200, "text/html", req_info->uri, ".");
  else
    httpc_writef(&conn, 200, "text/plain", 
      "New connection system test\n\
Route: %s\n\
User-Agent: %s\n\
Cookies: %s",
      req_info->uri,
      httpc_get_req_header(req_info, "User-Agent"),
      httpc_get_req_header(req_info, "Cookie")
    );
  // end of proper request code

  _hc_conn_send(&conn);
  _hc_conn_free(&conn);

  LOG_DEBUG("Responded successfully to client!");
  return;
}

int main(int argc, char** argv) {
  uint16_t port;
  _hc_log_init(LOG_PATH, LOG_NAME);

  if (argc > 1) port = (uint16_t) atoi(argv[1]);
  else {
    LOG_DEBUG("Defaulting to port 8080");
    port = 8080;
  }

  if (port < 1024) {
    LOG_WARN("Port is protected, binding WILL fail if not root!");
  }

  _hc_server server;
  if (_hc_server_init(&server, port) < 0) {
    LOG_FATAL("_hc_server_init() failed fatally: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  _hc_tpool_t* tpool = _hc_tpool_new(TPOOL_WORKERS);

  LOG_DEBUG("Listening...");
  while (1) {
    _hc_socket* client = _hc_server_listen(&server);

    if (client == NULL) {
      if (errno == EINTR) {
        LOG_WARN("Shutting down on interrupt...");
        break;
      } else {
        LOG_FATAL("_hc_server_listen() failed fatally");
        _hc_server_close(&server);
        _hc_tpool_free(tpool);
        return 0;
      }
    }

    LOG_DEBUG("Request made from client %hd! Handling...", client->fd);
    _hc_tpool_add_work(tpool, (_hc_threadfunc_t) handle_request, (void*) client);
  }

  _hc_server_close(&server);
  _hc_tpool_free(tpool);
  return 0;
}