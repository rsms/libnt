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
**/
#include "tcp_server.h"
#include "fd_tcp.h"
#include "mpool.h"
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>

#ifndef IPPROTO_IPV4
#define IPPROTO_IPV4 IPPROTO_IPIP
#endif

static void _dealloc(nt_tcp_server_t *self) {
  if (self->ev4)
    nt_free(self->ev4, sizeof(struct event));
  if (self->ev6)
    nt_free(self->ev6, sizeof(struct event));
  nt_free(self, sizeof(nt_tcp_server_t));
}


nt_tcp_server_t *nt_tcp_server_new(nt_tcp_server_on_accept_t on_accept) {
  NT_OBJ_ALLOC_INIT_self(nt_tcp_server_t, &_dealloc);
  NT_OBJ_CLEAR(self, nt_tcp_server_t);
  self->fd4 = self->fd6 = -1;
  self->on_accept = on_accept;
  return self;
}


bool nt_tcp_server_bindtoaddr(nt_tcp_server_t *server, const nt_sockaddr_t *sa, void *aiv) {
  int *fd;
  
  // Get fd
  fd = (sa->ss_family == AF_INET6) ? &server->fd6 : &server->fd4;
  if (*fd != -1)
    return false;
  
  // Create socket
  if ((*fd = nt_fd_tcp_socket(sa->ss_family)) == -1)
    return false;
  
  // Bind to sa
  if (nt_fd_tcp_bind(*fd, sa) != 0) {
    nt_fd_close(fd);
    return false;
  }
  
  // Save address
  if (sa->ss_family == AF_INET6)
    memcpy((void * restrict)&server->addr6, (const void * restrict)sa, sizeof(struct sockaddr_in6));
  else
    memcpy((void * restrict)&server->addr4, (const void * restrict)sa, sizeof(struct sockaddr_in));
  
  if (listen(*fd, SOMAXCONN) == -1)
    nt_warn("listen");
  
  return true;
}


bool nt_tcp_server_bind(nt_tcp_server_t *server, const char *addr, int port, int proto) {
  struct addrinfo hints, *servinfo, *ai;
  int rv;
  char strport[12];
  
  assert(server != NULL);
  assert(addr != NULL);
  assert(server->on_accept != NULL);
  
  // Setup getaddrinfo hints
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE; // AI_CANONNAME also ?
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = proto;
  
  // getaddrinfo
  snprintf(strport, sizeof(strport), "%d", port);
  rv = getaddrinfo(addr, strport, &hints, &servinfo);
  if (servinfo == NULL || rv == -1) {
    nt_warn("getaddrinfo: %s", gai_strerror(rv));
    return false;
  }
  
  // loop through all the results and bind to the first found addr per af
  for ( ai = servinfo; ai; ai = ai->ai_next ) {
    assert(ai->ai_socktype == SOCK_STREAM);
    assert(ai->ai_protocol == IPPROTO_TCP);
    
    if (((ai->ai_family == AF_INET6) ? server->fd6 : server->fd4) != -1)
      continue;
    
    if (!nt_tcp_server_bindtoaddr(server, (const nt_sockaddr_t *)ai->ai_addr, (void *)ai))
      break;
  }
  
  freeaddrinfo(servinfo);
  
  return (server->fd4 != -1 || server->fd6 != -1);
}
