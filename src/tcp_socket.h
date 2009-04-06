/**
  TCP socket.
  
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
#ifndef _NT_TCP_SOCKET_H_
#define _NT_TCP_SOCKET_H_

#include "obj.h"
#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>
#include <netdb.h>

typedef struct nt_tcp_socket {
  NT_OBJ_HEAD
  int fd;
  struct sockaddr_storage addr;
} nt_tcp_socket;

nt_tcp_socket *nt_tcp_socket_new();

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

// Accept a connection from @server_fd
bool nt_tcp_socket_accept(nt_tcp_socket *self, int server_fd);

// Return the socket address family (e.g. AF_INET, AF_INET6)
NT_STATIC_INLINE sa_family_t nt_tcp_socket_family(nt_tcp_socket *self) {
  /* the family is the first member of struct sockaddr: */
  return (sa_family_t)( *((sa_family_t *)(&self->addr)) );
}

// Check if the socket is open
NT_STATIC_INLINE bool nt_tcp_socket_is_open(nt_tcp_socket *self) {
  return (self->fd != -1) ? true : false;
}

// Close
bool nt_tcp_socket_close(nt_tcp_socket *self);

#endif
