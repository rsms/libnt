#include "tcp_socket.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/tcp.h>


static void _dealloc(nt_tcp_socket *self) {
  if (self->addr)
    free(self->addr);
  free(self);
}


nt_tcp_socket *nt_tcp_socket_new(struct sockaddr *addr, int fd) {
  nt_tcp_socket *self;
  
  if ( !(self = (nt_tcp_socket *)malloc(sizeof(nt_tcp_socket))) )
    return NULL;
  
  nt_obj_init((nt_obj *)self, (nt_obj_destructor *)_dealloc);
  
  self->addr = addr;
  self->fd = fd;
  
  return self;
}


const char *nt_tcp_socket_host(nt_tcp_socket *socket) {
  static char *buf[100];
  buf[0] = '\0';
  if(nt_tcp_socket_hostcpy(socket, (char *)&buf, 100) == NULL) {
    return NULL;
  }
  return (const char *)buf;
}


char *nt_tcp_socket_hostcpy(nt_tcp_socket *socket, char *buf, size_t bufsize) {
  return nt_util_sockaddr_hostcpy(socket->addr, buf, bufsize);
}


uint16_t nt_tcp_socket_port(nt_tcp_socket *socket) {
  return nt_util_sockaddr_port(socket->addr);
}


bool nt_tcp_socket_bind(nt_tcp_socket *sock, struct addrinfo *ptr, bool ipv6_only) {
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


bool nt_tcp_socket_listen(nt_tcp_socket *sock, int backlog) {
  if (listen(sock->fd, backlog) != 0) {
		perror("listen");
    return false;
	}
  return true;
}
