#include "tcp_socket.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/tcp.h>


static void _dealloc(nt_tcp_socket *self) {
  nt_tcp_socket_close(self);
  if (self->addr)
    free(self->addr);
  free(self);
}


nt_tcp_socket *nt_tcp_socket_new(int fd) {
  nt_tcp_socket *self;
  
  if ( !(self = (nt_tcp_socket *)malloc(sizeof(nt_tcp_socket))) )
    return NULL;
  
  nt_obj_init((nt_obj *)self, (nt_obj_destructor *)_dealloc);
  
  self->addr = NULL;
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


bool nt_tcp_socket_bind(nt_tcp_socket *self, struct addrinfo *ptr, bool ipv6_only, bool blocking) {
  static const char on = 1;
  
  if ((self->fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0) {
    perror("socket() in nt_tcp_socket_bind()");
    return false;
  }
  
  if (setsockopt(self->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0) {
    perror("setsockopt SOL_SOCKET SO_REUSEADDR 1 in nt_tcp_socket_bind()");
    nt_tcp_socket_close(self);
    return false;
  }
  
  if ( !blocking && setsockopt(self->fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(int)) < 0) {
    perror("setsockopt IPPROTO_TCP TCP_NODELAY 1 in nt_tcp_socket_bind()");
    nt_tcp_socket_close(self);
    return false;
  }
  
  if ( (ptr->ai_family == AF_INET6)
       && ipv6_only
       && (setsockopt(self->fd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(int)) < 0) )
  {
    perror("setsockopt IPPROTO_IPV6 IPV6_V6ONLY 1 in nt_tcp_socket_bind()");
    nt_tcp_socket_close(self);
    return false;
  }
  
  if (bind(self->fd, ptr->ai_addr, ptr->ai_addrlen) < 0) {
    perror("bind() in nt_tcp_socket_bind()");
    nt_tcp_socket_close(self);
    return false;
  }
  
  if ( !blocking && nt_util_fd_setnonblock(self->fd) == -1) {
    nt_tcp_socket_close(self);
    return false;
  }
  
  self->addr = ptr->ai_addr;
  ptr->ai_addr = NULL;
  
  return true;
}


bool nt_tcp_socket_listen(nt_tcp_socket *self, int backlog) {
  if (listen(self->fd, backlog) != 0) {
		perror("listen() in nt_tcp_socket_listen()");
    return false;
	}
  return true;
}


nt_tcp_socket *nt_tcp_socket_accept(int server_fd, int af) {
  int client_fd;
  struct sockaddr *client_addr;
  socklen_t sockaddr_len;
  nt_tcp_socket *sock;
  
  if (af == AF_INET6)
    sockaddr_len = sizeof(struct sockaddr_in6);
  else
    sockaddr_len = sizeof(struct sockaddr_in);
  
  if ( (client_addr = (struct sockaddr *)calloc(1, sockaddr_len)) == NULL )
    return NULL;
  
  client_fd = accept(server_fd, client_addr, &sockaddr_len);
  if (client_fd == -1) {
    perror("accept() in nt_tcp_socket_accept()");
    free(client_addr);
    return NULL;
  }
  
  if ( (sock = nt_tcp_socket_new(client_fd)) == NULL ) {
    free(client_addr);
    return NULL;
  }
  sock->addr = client_addr;
  
  return sock;
}


bool nt_tcp_socket_close(nt_tcp_socket *self) {
  if (!nt_tcp_socket_is_open(self))
    return true;
  if (close(self->fd) == -1) {
    perror("nt_tcp_socket_close: close() failed");
    return false;
  }
  self->fd = -1;
  return true;
}

