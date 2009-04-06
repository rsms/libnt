/**
  Copyright (c) 2009 Notion <http://notion.se/>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
#include "tcp_socket.h"
#include "util.h"
#include "mpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/tcp.h>


static void _dealloc(nt_tcp_socket *self) {
  nt_tcp_socket_close(self);
  nt_free(self, sizeof(nt_tcp_socket));
}


nt_tcp_socket *nt_tcp_socket_new(int fd) {
  nt_tcp_socket *self;
  
  if ( !(self = (nt_tcp_socket *)nt_malloc(sizeof(nt_tcp_socket))) )
    return NULL;
  
  nt_obj_init((nt_obj *)self, (nt_obj_destructor *)_dealloc);
  
  self->fd = fd;
  memset(&self->addr, 0, sizeof(struct sockaddr_storage));
  
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
  return nt_util_sockaddr_hostcpy((struct sockaddr *)&socket->addr, buf, bufsize);
}


uint16_t nt_tcp_socket_port(nt_tcp_socket *socket) {
  return nt_util_sockaddr_port((struct sockaddr *)&socket->addr);
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
  
  if (ptr->ai_family == AF_INET6)
    memcpy((void * restrict)&self->addr, (const void * restrict)ptr->ai_addr, sizeof(struct sockaddr_in6));
  else
    memcpy((void * restrict)&self->addr, (const void * restrict)ptr->ai_addr, sizeof(struct sockaddr_in));
  
  return true;
}


bool nt_tcp_socket_listen(nt_tcp_socket *self, int backlog) {
  if (listen(self->fd, backlog) != 0) {
		perror("listen() in nt_tcp_socket_listen()");
    return false;
	}
  return true;
}


bool nt_tcp_socket_accept(nt_tcp_socket *self, int server_fd) {
  socklen_t sockaddr_len;
  
  sockaddr_len = sizeof(struct sockaddr_storage);
  self->fd = accept(server_fd, (struct sockaddr *)&self->addr, &sockaddr_len);
  
  if (self->fd == -1) {
    perror("accept() in nt_tcp_socket_accept()");
    return false;
  }
  
  return true;
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

