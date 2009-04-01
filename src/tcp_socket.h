#ifndef NTN_TCP_SOCKET_H
#define NTN_TCP_SOCKET_H

#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>
#include <netdb.h>

typedef struct nt_tcp_socket {
  struct sockaddr *addr;
  int fd;
} nt_tcp_socket;

bool nt_tcp_socket_init(nt_tcp_socket *sock);
void nt_tcp_socket_destroy(nt_tcp_socket *sock);

// Return a human readable string representing the host address of socket.
// This is not thread safe (See nt_tcp_socket_hostcpy() for a thread safe version).
const char *nt_tcp_socket_host(nt_tcp_socket *sock);

// Copy a human readable string representing the host address of socket
char *nt_tcp_socket_hostcpy(nt_tcp_socket *sock, char *buf, size_t bufsize);

// Port of socket address in host byte order (a normal integer)
uint16_t nt_tcp_socket_port(nt_tcp_socket *sock);

// Bind socket sock to address ai
bool nt_tcp_socket_bind(nt_tcp_socket *sock, struct addrinfo *ai, bool ipv6_only);

// Listen
bool nt_tcp_socket_listen(nt_tcp_socket *sock, int backlog);

#endif
