#ifndef __SOCKET_H
#define __SOCKET_H

#include "vec.h"

#define KiB(x) x*(1<<10)
#define MiB(x) x*(1<<20)
#define DEFAULT_BUFFER_SIZE KiB(1)
#define MAX_REQUEST_SIZE MiB(1)

struct Socket {
  int fd;
};

typedef struct Socket Socket;

int socket_init(Socket* sock, unsigned short port);
Socket socket_accept(Socket sock);
hc_vec socket_recv(Socket sock);
void socket_send(Socket sock, hc_vec buf);
void socket_close(Socket sock);
#endif