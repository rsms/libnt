#ifndef NTN_TCP_SOCKET_H
#define NTN_TCP_SOCKET_H

#include "obj.h"
#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>
#include <netdb.h>

typedef struct nt_tcp_socket {
  NT_OBJ_HEAD
  struct sockaddr *addr;
  int fd;
} nt_tcp_socket;

nt_tcp_socket *nt_tcp_socket_new(int fd);

// Return a human readable string representing the host address of socket.
// This is not thread safe (See nt_tcp_socket_hostcpy() for a thread safe version).
const char *nt_tcp_socket_host(nt_tcp_socket *sock);

// Copy a human readable string representing the host address of socket
char *nt_tcp_socket_hostcpy(nt_tcp_socket *sock, char *buf, size_t bufsize);

// Port of socket address in host byte order (a normal integer)
uint16_t nt_tcp_socket_port(nt_tcp_socket *sock);

// Bind socket sock to address ai
bool nt_tcp_socket_bind(nt_tcp_socket *self, struct addrinfo *ai, bool ipv6_only, bool blocking);

// Listen
bool nt_tcp_socket_listen(nt_tcp_socket *self, int backlog);

// Accept a connection. Returns a new reference.
nt_tcp_socket *nt_tcp_socket_accept(int server_fd, int af);

// Close
bool nt_tcp_socket_close(nt_tcp_socket *self);

// Check if the socket is open
inline static bool nt_tcp_socket_is_open(nt_tcp_socket *self) {
  return (self->fd != -1) ? true : false;
}

#endif
