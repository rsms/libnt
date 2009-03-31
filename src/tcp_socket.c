#include "tcp_socket.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/tcp.h>


bool ntn_tcp_socket_init(ntn_tcp_socket *socket) {
  socket->addr = NULL;
  socket->fd = -1;
  return true;
}


void ntn_tcp_socket_destroy(ntn_tcp_socket *socket) {
  free(socket->addr);
}


const char *ntn_tcp_socket_host(ntn_tcp_socket *socket) {
  static char *buf[100];
  buf[0] = '\0';
  if(ntn_tcp_socket_hostcpy(socket, (char *)&buf, 100) == NULL) {
    return NULL;
  }
  return (const char *)buf;
}


char *ntn_tcp_socket_hostcpy(ntn_tcp_socket *socket, char *buf, size_t bufsize) {
  return ntn_util_sockaddr_hostcpy(socket->addr, buf, bufsize);
}


uint16_t ntn_tcp_socket_port(ntn_tcp_socket *socket) {
  return ntn_util_sockaddr_port(socket->addr);
}


bool ntn_tcp_socket_bind(ntn_tcp_socket *sock, struct addrinfo *ptr, bool ipv6_only) {
  static const char on = 1;
  
  if ((sock->fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0) {
    perror("socket");
    return false;
  }
  if (setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0) {
    close(sock->fd);
    perror("setsockopt SOL_SOCKET SO_REUSEADDR 1");
    return false;
  }
  if (setsockopt(sock->fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(int)) < 0) {
    close(sock->fd);
    perror("setsockopt IPPROTO_TCP TCP_NODELAY 1");
    return false;
  }
  if ( (ptr->ai_family == AF_INET6)
       && ipv6_only
       && (setsockopt(sock->fd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(int)) < 0) )
  {
    close(sock->fd);
    perror("setsockopt IPPROTO_IPV6 IPV6_V6ONLY 1");
    return false;
  }
  if (bind(sock->fd, ptr->ai_addr, ptr->ai_addrlen) < 0) {
    close(sock->fd);
    perror("bind");
    return false;
  }
  
  sock->addr = ptr->ai_addr;
  ptr->ai_addr = NULL;
  
  return true;
}


bool ntn_tcp_socket_listen(ntn_tcp_socket *sock, int backlog) {
  if (listen(sock->fd, backlog) != 0) {
		perror("listen");
    return false;
	}
  return true;
}
