#ifndef __SOCKET_H
#define __SOCKET_H

#include "vec.h"

#define KiB(x) x*(1<<10)
#define MiB(x) x*(1<<20)
#define DEFAULT_BUFFER_SIZE KiB(1)
#define MAX_REQUEST_SIZE MiB(1)

typedef struct _hc_socket {
  int fd;
} _hc_socket;


int _hc_socket_init(_hc_socket* sock, unsigned short port);
_hc_socket _hc_socket_accept(_hc_socket sock);
hc_vec _hc_socket_recv(_hc_socket sock);
void _hc_socket_send(_hc_socket sock, hc_vec buf);
void _hc_socket_close(_hc_socket sock);
#endif