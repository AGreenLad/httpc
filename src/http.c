#include <unistd.h>
#include "map.h"
#include "vec.h"
#include "request.h"
#include "response.h"
#include "socket.h"
#include "tpool.h"
#include "log.h"

#define TPOOL_WORKERS 24

// maybe use an arena for each request
void handle_request(_hc_socket* client) {
  hc_vec raw_req = _hc_socket_recv(*client);

  if (raw_req.length == 0) {
    LOG_WARN("returning early, client suddenly closed");
    return;
  }

  // printf("Read %ld bytes from client\n", raw_req.length);

  httpc_req req = _hc_req_parse(raw_req);
  // req_print(&req);

  // todo: custom path handlers / middleware / whatever

  _hc_res res = _hc_res_new();
  _hc_res_set_header(&res, "Server", "httpc/0.1");
  _hc_res_set_header(&res, "Connection", "close");
  // start of proper request code
  char page[500];
  snprintf(page, 500, "Your URI: %s\nYour user agent: %s\nYour cookies: %s\n",
    req.uri,
    httpc_req_get_header(&req, "User-Agent"),
    httpc_req_get_header(&req, "Cookie")
  );
  _hc_res_str(&res, 200, page, "text/plain");
  // end of proper request code

  _hc_res_send(res, *client);

  _hc_req_free(&req);
  _hc_res_free(&res);
  _hc_socket_ready(client);

  LOG_DEBUG("Responded successfully to client!");
  return;
}

int main(int argc, char** argv) {
  uint16_t port;

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

  tpool_t* handlers = tpool_new(TPOOL_WORKERS);

  LOG_DEBUG("Listening...");
  while (1) {
    _hc_socket client = _hc_server_listen(&server);

    if (client.fd < 0) {
      if (errno == EINTR) {
        LOG_WARN("Shutting down on interrupt...");
        break;
      } else {
        LOG_FATAL("_hc_server_listen() failed fatally");
        return 0;
      }
    }

    LOG_DEBUG("Request made from client %hd! Handling...", client.fd);
    tpool_add_work(handlers, (threadfunc_t) handle_request, (void*) &client);
  }

  _hc_server_close(&server);
  tpool_free(handlers);
  return 0;
}