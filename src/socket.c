#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "socket.h"

#define BACKLOG 200

int _hc_server_init(_hc_server* serv, unsigned short port) {
  int serv_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (serv_fd < 0) {
    perror("[x] socket() failed");
    return -1;
  }

  int on = 1;
  if (setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) {
    perror("[x] setsockopt()");
    return -1;
  }

  if (ioctl(serv_fd, FIONBIO, (char*)&on) < 0) {
    perror("[x] ioctl()");
    return -1;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);
  socklen_t addr_size = sizeof(addr);

  if (bind(serv_fd, (struct sockaddr*) &addr, addr_size) < 0) {
    perror("[x] bind() failed");
    return -1;
  }

  if (listen(serv_fd, BACKLOG) < 0) {
    perror("[x] listen() failed");
    return -1;
  }

  serv->fd = serv_fd;

  memset(serv->client_fds, 0, sizeof(serv->client_fds));

  serv->client_fds[0].fd = serv_fd;
  serv->client_fds[0].events = POLLIN;
  
  serv->nfds = 1;

  return 1;
}

// passively listens for, accepts, and polls clients, returns socket obj when one polls
// this is a good idea surely
_hc_socket _hc_server_listen(_hc_server* serv) {
  _hc_socket sock_to_return = { .fd = -1, .pfd_entry = NULL };

  int timeout = 3 * 60 * 1000;
  bool stop = false; // when we are ready to return either a socket or error out
  bool end_serv = false; // set when we are ready to end the server
  bool socket_closed = false; // set when we have closed any sockets

  while (!stop) {
    if (poll(serv->client_fds, serv->nfds, timeout) < 0) {
      // here it is safe to return because nothing's been done to the pollfd list yet
      if (errno == EINTR) {
        puts("[!] (Ctrl-C)");
        return sock_to_return;
      }
      perror("[x] poll()");
      return sock_to_return;
    }

    for (int i = 0; i < serv->nfds; i++) {
      struct pollfd* curr_fd = &serv->client_fds[i];
      if (curr_fd->fd == -1) // a client has closed suddenly in the middle of their request
        socket_closed = true;
      
      if (curr_fd->revents == 0) continue;

      if (curr_fd->fd == serv->fd) {
        // there is a new client
        int new_fd = -1;

        do {
          int new_fd = accept(serv->fd, NULL, NULL);
          if (new_fd < 0) {
            if (errno != EWOULDBLOCK) {
              perror("[x] accept()");
              stop = true;
            }
            break;
          }
          
          serv->client_fds[serv->nfds].fd = new_fd;
          serv->client_fds[serv->nfds].events = POLLIN;
          serv->nfds++;

          printf("[i] Client fd %d added to poll\n", new_fd);
        } while (new_fd != -1);
      }
      else {
        if (curr_fd->revents & POLLHUP) { // if the client has closed connection
          printf("[i] Client fd %d has closed w/ POLLHUP\n", curr_fd->fd);
          close(curr_fd->fd);
          curr_fd->fd = -1;
          socket_closed = true;
        }
        else {
          // client has made a request or disconnected gracefully
          // printf("[i] Client fd %d has event %hd\n", curr_fd->fd, curr_fd->revents);
          // this is so hacky
          char c;
          if (recv(curr_fd->fd, &c, 1, MSG_PEEK) == 0) {
            printf("[i] Client fd %d has closed gracefully\n", curr_fd->fd);
            close(curr_fd->fd);
            curr_fd->fd = -1;
            socket_closed = true;
          } else {
            sock_to_return.fd = curr_fd->fd;
            sock_to_return.pfd_entry = curr_fd;
            curr_fd->fd = ~curr_fd->fd;
            stop = true;
            break;
          }
        }
      }
    }
  }

  // clean up time
  if (end_serv) {
    _hc_server_close(serv);
  }

  if (socket_closed) {
    for (int i = 0; i < serv->nfds; i++) {
      if (serv->client_fds[i].fd == -1) {
        for (int j = i; j < serv->nfds; j++) {
          serv->client_fds[j].fd = serv->client_fds[j + 1].fd;  
        }
        i--;
        serv->nfds--;
      }
    }
  }

  return sock_to_return; // finally done........
}

hc_vec _hc_socket_recv(_hc_socket sock) {
  hc_vec buf;
  hc_vec_init(&buf);
  hc_vec_reserve(&buf, DEFAULT_BUFFER_SIZE);

  ssize_t bytes_read = 0;

  for (;;) {
    bytes_read = recv(sock.fd, buf.data + bytes_read, buf.capacity - bytes_read, 0);
    // printf("%ld bytes read\n", bytes_read);
    
    if (bytes_read < 0) {
      if (errno != EWOULDBLOCK) {
        printf("[x] fail while reading sock w/ fd %d", sock.fd);
        perror("[x] recv() failed");
        hc_vec_free(&buf);
        exit(EXIT_FAILURE);
      }
    }

    else if (bytes_read == 0) {
      // im fucked squid game gif
      printf("[i] client %d closed suddenly\n", sock.fd);
      _hc_socket_close(&sock);

      hc_vec_free(&buf);

      return (hc_vec) {
        .length = 0,
        .data = NULL
      };
    }

    buf.length += (size_t) bytes_read; // in this case bytes_read is likely castable to size_t
    if (buf.length >= buf.capacity) {
      if (buf.length >= MAX_REQUEST_SIZE) {
        puts("[x] *********\nrequest exceeded max size, terminating!\n(implement 413)\n*********");
        hc_vec_free(&buf);
        exit(EXIT_FAILURE);
      }

      hc_vec_reserve(&buf, buf.capacity * 2);
    } else { break; }
  }
  return buf;
}

void _hc_socket_send(_hc_socket sock, hc_vec buf) {
  ssize_t bytes_sent = 0;
  ssize_t total_sent = 0;

  do {
    bytes_sent = send(sock.fd, buf.data + total_sent, buf.length - total_sent, 0);
    total_sent += bytes_sent;
    // printf("[t] sent %ld bytes\n", bytes_sent);
  } while ((size_t) total_sent > buf.length);

  // printf("[t] sent %ld bytes total\n", total_sent);

  if (bytes_sent <= -1) {
    perror("send() failed");
    exit(EXIT_FAILURE);
  }
}

void _hc_socket_ready(_hc_socket* sock) {
  sock->pfd_entry->fd = ~sock->pfd_entry->fd;
}

void _hc_server_close(_hc_server* serv) {
  close(serv->fd);
}
void _hc_socket_close(_hc_socket* sock) {
  close(sock->fd);
  sock->pfd_entry->fd = -1;
}

