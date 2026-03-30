#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "socket.h"

int socket_init(Socket* sock, unsigned short port) {
  sock->fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock->fd < 0) {
    perror("socket() failed");
    return -1;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);
  socklen_t addr_size = sizeof(addr);

  if (bind(sock->fd, (struct sockaddr*) &addr, addr_size) < 0) {
    perror("bind() failed");
    return -1;
  }

  if (listen(sock->fd, 3) < 0) {
    perror("listen() failed");
    return -1;
  }

  return 1;
}

Socket socket_accept(Socket sock) {
  struct sockaddr client_addr;
  socklen_t addr_size = sizeof(struct sockaddr);
  int client_fd = accept(sock.fd, &client_addr, &addr_size);
  if (client_fd < 0) {
    perror("accept() failed");
    return (Socket) { .fd = -1 };
  }

  return (Socket) { .fd = client_fd };
}

Buffer socket_recv(Socket sock) {
  Buffer buf;
  buf_init(&buf);
  buf_reserve(&buf, DEFAULT_BUFFER_SIZE);

  ssize_t bytes_read = 0;

  for (;;) {
    bytes_read = recv(sock.fd, buf.data + bytes_read, buf.capacity - bytes_read, 0);
    printf("%ld bytes read\n", bytes_read);
    
    if (bytes_read == -1l) {
      perror("recv() failed");
      buf_free(&buf);
      exit(EXIT_FAILURE);
    }

    else if (bytes_read == 0) {
      puts("client closed suddenly");
      close(sock.fd);
      buf_free(&buf);

      return (Buffer) {
        .length = 0,
        .data = NULL
      };
    }

    buf.length += (size_t) bytes_read; // in this case bytes_read is likely castable to size_t
    if (buf.length >= buf.capacity) {
      if (buf.length >= MAX_REQUEST_SIZE) {
        puts("*********\nrequest exceeded max size, terminating!\n(implement 413)\n*********");
        buf_free(&buf);
        exit(EXIT_FAILURE);
      }

      buf_reserve(&buf, buf.capacity * 2);
    } else { break; }
  }

  return buf;
}

void socket_send(Socket sock, Buffer buf) {
  ssize_t bytes_sent = 0;
  ssize_t total_sent = 0;

  do {
    bytes_sent = send(sock.fd, buf.data + total_sent, buf.length - total_sent, 0);
    total_sent += bytes_sent;
    printf("[t] sent %ld bytes\n", bytes_sent);
  } while ((size_t) total_sent > buf.length);

  printf("[t] sent %ld bytes total\n", total_sent);

  if (bytes_sent <= -1) {
    perror("send() failed");
    exit(EXIT_FAILURE);
  }
}

void socket_close(Socket sock) {
  close(sock.fd);
}

