#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include "map.h"
#include "buffer.h"
#include "request.h"
#include "response.h"
#include "socket.h"
#include "buffer.h"

// maybe use an arena for each request
void* handle_request(Socket client) {
  Buffer req_buf = socket_recv(client);

  if (req_buf.length == 0) {
    puts("returning early, client suddenly closed");
    return NULL;
  }

  printf("Read %ld bytes from client\n", req_buf.length);

  Request req = req_parse(req_buf);
  req_print(&req);

  // todo: custom path handlers / middleware / whatever

  Response res = res_new(200);
  res_set_header(&res, "Server", "httpc/0.1");
  res_set_header(&res, "Connection", "close"); // should be sending this by default
  
  int res = res_file(&res, req);
  if (res == 0) 
    res_str(&res, 404, "<h1>File not found</h1>", "text/html");
  else if (res == -1)
    res_str(&res, 500, "<h1>Internal Server Error Occurred</h1>", "text/html");
  res_send(res, client);

  req_free(&req);
  res_free(&res);

  socket_close(client);
  return NULL;
}

int main(int argc, char** argv) {
  uint16_t port;

  if (argc > 1) port = (uint16_t) atoi(argv[1]);
  else {
    puts("Defaulting to port 8080");
    port = 8080;
  }

  if (port < 1024) {
    puts("Port is protected, binding WILL fail if not root!");
  }

  Socket server;
  if (socket_init(&server, port) < 0) {
    perror("socket_init() failed");
    exit(EXIT_FAILURE);
  }

  puts("Listening...");
  while (1) {
    Socket client = socket_accept(server);

    if (client.fd < 0) {
      perror("socket_accept() failed");
      exit(EXIT_FAILURE);
    }

    puts("Accepted client!");
    // todo: threads + server lasts forever
    handle_request(client);
    break;
  }

  socket_close(server);
  return 0;
}