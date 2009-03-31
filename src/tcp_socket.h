#ifndef NTN_TCP_SOCKET_H
#define NTN_TCP_SOCKET_H

#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>
#include <netdb.h>

typedef struct ntn_tcp_socket {
  struct sockaddr *addr;
  int fd;
} ntn_tcp_socket;

bool ntn_tcp_socket_init(ntn_tcp_socket *sock);
void ntn_tcp_socket_destroy(ntn_tcp_socket *sock);

// Return a human readable string representing the host address of socket.
// This is not thread safe (See ntn_tcp_socket_hostcpy() for a thread safe version).
const char *ntn_tcp_socket_host(ntn_tcp_socket *sock);

// Copy a human readable string representing the host address of socket
char *ntn_tcp_socket_hostcpy(ntn_tcp_socket *sock, char *buf, size_t bufsize);

// Port of socket address in host byte order (a normal integer)
uint16_t ntn_tcp_socket_port(ntn_tcp_socket *sock);

// Bind socket sock to address ai
bool ntn_tcp_socket_bind(ntn_tcp_socket *sock, struct addrinfo *ai, bool ipv6_only);

// Listen
bool ntn_tcp_socket_listen(ntn_tcp_socket *sock, int backlog);

#endif
