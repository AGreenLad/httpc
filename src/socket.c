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
#include "log.h"

#define BACKLOG 200
#define _HC_LOG_MODULE "NET"

int _hc_server_init(_hc_server* serv, unsigned short port) {
  int serv_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (serv_fd < 0) {
    LOG_ERROR("server socket() failed: %s", strerror(errno));
    return -1;
  }

  // set nonblocking
  int on = 1;
  if (setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) {
    LOG_ERROR("server setsockopt() failed: %s", strerror(errno));
    return -1;
  }

  if (ioctl(serv_fd, FIONBIO, (char*)&on) < 0) {
    LOG_ERROR("server ioctl() failed: %s", strerror(errno));
    return -1;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);
  socklen_t addr_size = sizeof(addr);

  if (bind(serv_fd, (struct sockaddr*) &addr, addr_size) < 0) {
    LOG_ERROR("server bind() failed: %s", strerror(errno));
    return -1;
  }

  if (listen(serv_fd, BACKLOG) < 0) {
    LOG_ERROR("server listen() failed: %s", strerror(errno));
    return -1;
  }
  serv->fd = serv_fd;
  
  memset(serv->client_fds, 0, sizeof(serv->client_fds));
  memset(serv->clients, 0, sizeof(serv->clients));

  // create a pipe for signaling to reload the fd list
  // this way when a connection ends we can preemptively remove it from the fd list
  int pipefds[2];
  if (pipe(pipefds) == -1) {
    LOG_ERROR("pipe() failed: %s", strerror(errno));
  }
  serv->client_fds[0].fd = pipefds[0]; // read end, for interrupting poll
  serv->client_fds[0].events = POLLIN;
  serv->reread_fd = pipefds[1]; // write end, to notify server when to interrupt

  serv->client_fds[1].fd = serv_fd;
  serv->client_fds[1].events = POLLIN;
  
  serv->nfds = 2;

  return 1;
}

// passively listens for, accepts, and polls clients, returns socket obj when one polls
// this is a good idea surely
_hc_socket* _hc_server_listen(_hc_server* serv) {
  int timeout = 3 * 60 * 1000;
  bool stop = false; // when we are ready to return either a socket or error out
  bool end_serv = false; // set when we are ready to end the server
  bool socket_closed = false; // set when we have closed any sockets

  _hc_socket* client;

  while (!stop) {
    if (poll(serv->client_fds, serv->nfds, timeout) < 0) {
      // here it is safe to return because nothing's been done to the pollfd list yet
      if (errno == EINTR) {
        LOG_WARN("Interrupt caught while polling");
        return NULL;
      }
      LOG_ERROR("poll() failed: %s", strerror(errno));
      return NULL;
    }

    if (serv->client_fds[0].revents == POLLIN) {
        // it is time to reload
        LOG_DEBUG("Reread fd signaled, reloading list");
        char x;
        read(serv->client_fds[0].fd, &x, 1);
        continue;
    }

    for (int i = 1; i < serv->nfds; i++) {
      struct pollfd* curr_fd = &serv->client_fds[i];
      if (curr_fd->fd == -1) { // in case we haven't deleted a closed socket yet for whatever reason
        socket_closed = true;
        LOG_WARN("closed fd found in poll list! not good");
      }

      if (curr_fd->revents == 0) continue;

      if (curr_fd->fd == serv->fd) {
        // there is a new client
        int new_fd = -1;

        do {
          int new_fd = accept(serv->fd, NULL, NULL);
          if (new_fd < 0) {
            if (errno != EWOULDBLOCK) {
              LOG_ERROR("accept() failed: %s", strerror(errno));
              stop = true;
            }
            break;
          }
          
          serv->client_fds[serv->nfds].fd = new_fd;
          serv->client_fds[serv->nfds].events = POLLIN;

          struct sockaddr new_sock_addr;
          socklen_t addr_len = sizeof(new_sock_addr); // why the HELL do i need a pointer
          if (getpeername(new_fd, &new_sock_addr, &addr_len) == -1) {
            LOG_ERROR("getpeername on new sock failed: %s", strerror(errno));
          }
          serv->clients[serv->nfds - 2] = (_hc_socket) {
            .fd = new_fd,
            .addr = new_sock_addr,
            .pfd_entry = &serv->client_fds[serv->nfds],
            .server = serv,
          };
          serv->nfds++;

          LOG_DEBUG("Client fd %d added to poll", new_fd);
        } while (new_fd != -1);
      }
      else {
        if (curr_fd->revents & POLLHUP) { // if the client has closed connection
          LOG_DEBUG("Client fd %d has closed w/ POLLHUP", curr_fd->fd);
          close(curr_fd->fd);
          curr_fd->fd = -1;
          socket_closed = true;
        }
        else {
          // client has made a request or disconnected gracefully
          // LOG_DEBUG("Client fd %d has event %hd", curr_fd->fd, curr_fd->revents);

          // to check if the socket is disconnecting, try to peek at the first byte
          // if it returns 0, then the client has dc'd; if it doesn't then we're still fine
          // because we only peeked
          // this is so hacky why cant linux just pass hangup ugghhhhhh
          char c;
          if (recv(curr_fd->fd, &c, 1, MSG_PEEK) == 0) {
            LOG_DEBUG("Client fd %d has closed gracefully", curr_fd->fd);
            close(curr_fd->fd);
            curr_fd->fd = -1;
            socket_closed = true;
          }
          else {
            client = &(serv->clients[i - 2]);
            // mark fd as in use by bit complementing it b/c poll ignores negative fds
            curr_fd->fd = ~curr_fd->fd;
            // we are ready to return
            stop = true;
            break;
          }
        }
      }
    }

    if (socket_closed) {
      for (int i = 2; i < serv->nfds; i++) {
        if (serv->client_fds[i].fd == -1) {
          for (int j = i; j < serv->nfds; j++) {
            serv->client_fds[j].fd = serv->client_fds[j + 1].fd;  
          }
          i--;
          serv->nfds--;
        }
      }
    }
  }

  // clean up time
  if (end_serv) {
    _hc_server_close(serv);
  }

  return client; // finally done........
}

hc_vec _hc_socket_recv(_hc_socket* sock) {
  hc_vec buf;
  hc_vec_init(&buf);
  hc_vec_reserve(&buf, DEFAULT_BUFFER_SIZE);

  ssize_t bytes_read = 0;

  for (;;) {
    bytes_read = recv(sock->fd, buf.data + bytes_read, buf.capacity - bytes_read, 0);
    // printf("%ld bytes read\n", bytes_read);
    
    if (bytes_read < 0) {
      if (errno != EWOULDBLOCK) {
        LOG_ERROR("Error while reading sock w/ fd %d: %s", sock->fd, strerror(errno));
        hc_vec_free(&buf);
        exit(EXIT_FAILURE);
      }
    }

    else if (bytes_read == 0) {
      LOG_WARN("Client %d suddenly closed while reading\n", sock->fd);
      _hc_socket_close(sock);
      hc_vec_free(&buf);

      return (hc_vec) {
        .length = 0,
        .data = NULL
      };
    }

    buf.length += (size_t) bytes_read;
    if (buf.length >= buf.capacity) {
      if (buf.length >= MAX_REQUEST_SIZE) {
        LOG_WARN("request exceeded max size, terminating! (implement 413)");
        hc_vec_free(&buf);
        exit(EXIT_FAILURE);
      }

      hc_vec_reserve(&buf, buf.capacity * 2);
    } else { break; }
  }
  return buf;
}

void _hc_socket_send(_hc_socket* sock, hc_vec buf) {
  ssize_t bytes_sent = 0;
  ssize_t total_sent = 0;

  do {
    bytes_sent = send(sock->fd, buf.data + total_sent, buf.length - total_sent, 0);
    total_sent += bytes_sent;
    // LOG_DEBUG("sent %ld bytes\n", bytes_sent);
  } while ((size_t) total_sent > buf.length);

  // LOG_DEBUG("sent %ld bytes total\n", total_sent);

  if (bytes_sent <= -1) {
    LOG_ERROR("send() failed: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void _hc_socket_ready(_hc_socket* sock) {
  sock->pfd_entry->fd = ~sock->pfd_entry->fd;
  // write one byte to signal reread fd
  write(sock->server->reread_fd, "a", 1);
}

void _hc_server_close(_hc_server* serv) {
  close(serv->fd);
}
void _hc_socket_close(_hc_socket* sock) {
  close(sock->fd);
  sock->pfd_entry->fd = -1;
}

