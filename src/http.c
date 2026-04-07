#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include "map.h"
#include "vec.h"
#include "request.h"
#include "response.h"
#include "socket.h"
#include "tpool.h"

#define TPOOL_WORKERS 12

static volatile int running = 1;

void on_signal(int) {
  puts("\n****************");
  puts("**** SIGINT ****");
  puts("****************");
  running = 0;
}

// maybe use an arena for each request
void handle_request(_hc_socket* client) {
  hc_vec raw_req = _hc_socket_recv(*client);

  if (raw_req.length == 0) {
    puts("returning early, client suddenly closed");
    return;
  }

  // printf("Read %ld bytes from client\n", raw_req.length);

  hc_req req = _hc_req_parse(raw_req);
  // req_print(&req);

  // todo: custom path handlers / middleware / whatever

  _hc_res res = _hc_res_new();
  _hc_res_set_header(&res, "Server", "httpc/0.1");
  _hc_res_set_header(&res, "Connection", "close"); // should be sending this by default, this is 1.0
  
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

  _hc_socket_close(*client);
  return;
}

int main(int argc, char** argv) {
  signal(SIGINT, on_signal);
  uint16_t port;

  if (argc > 1) port = (uint16_t) atoi(argv[1]);
  else {
    puts("Defaulting to port 8080");
    port = 8080;
  }

  if (port < 1024) {
    puts("Port is protected, binding WILL fail if not root!");
  }

  _hc_socket server;
  if (_hc_socket_init(&server, port) < 0) {
    perror("_hc_socket_init() failed");
    exit(EXIT_FAILURE);
  }

  tpool_t* handlers = tpool_new(TPOOL_WORKERS);

  puts("Listening...");
  while (running) {
    _hc_socket client = _hc_socket_accept(server);

    if (client.fd < 0) {
      perror("_hc_socket_accept() failed");
      exit(EXIT_FAILURE);
    }

    puts("Accepted client!");
    tpool_add_work(handlers, (threadfunc_t) handle_request, (void*) &client);
  }

  _hc_socket_close(server);
  tpool_free(handlers);
  return 0;
}