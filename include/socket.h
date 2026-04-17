#ifndef __SOCKET_H
#define __SOCKET_H

#include "vec.h"
#include <poll.h>

#define KiB(x) x*(1<<10)
#define MiB(x) x*(1<<20)
#define DEFAULT_BUFFER_SIZE KiB(1)
#define MAX_REQUEST_SIZE MiB(1)
#define MAX_CLIENTS 1024 // yeah sure why not

typedef struct _hc_socket {
  int fd;
  struct pollfd *pfd_entry; // to reset bit complement when done
} _hc_socket;

typedef struct _hc_server {
  int fd;
  struct pollfd client_fds[MAX_CLIENTS + 1]; // + 1 for the server
  int nfds;
} _hc_server;

int _hc_server_init(_hc_server* sock, unsigned short port);
_hc_socket _hc_server_listen(_hc_server* serv);
hc_vec _hc_socket_recv(_hc_socket sock);
void _hc_socket_send(_hc_socket sock, hc_vec buf);
void _hc_socket_ready(_hc_socket* sock);
void _hc_socket_close(_hc_socket* sock);
void _hc_server_close(_hc_server* serv);
#endif