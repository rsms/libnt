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
#include "sockserv.h"
#include "sockutil.h"
#include "mpool.h"
#include <netinet/in.h>
#include <netdb.h>

#ifndef IPPROTO_IPV4
#define IPPROTO_IPV4 IPPROTO_IPIP
#endif

static void _dealloc(nt_sockserv_t *self) {
  if (self->ev4)
    nt_free(self->ev4, sizeof(struct event));
  if (self->ev6)
    nt_free(self->ev6, sizeof(struct event));
  nt_free(self, sizeof(nt_sockserv_t));
}


nt_sockserv_t *nt_sockserv_new(nt_sockserv_on_accept_t on_accept) {
  NT_OBJ_ALLOC_INIT_self(nt_sockserv_t, &_dealloc);
  NT_OBJ_CLEAR(self, nt_sockserv_t);
  self->fd4 = -1;
  self->fd6 = -1;
  self->on_accept = on_accept;
  return self;
}


bool nt_sockserv_bindtoaddr(nt_sockserv_t *self, const nt_sockaddr_t *sa, int type) {
  int *fd;
  
  // Get fd
  fd = (sa->ss_family == AF_INET6) ? &self->fd6 : &self->fd4;
  if (*fd != -1)
    return false;
  
  // Create socket
  if ((*fd = nt_sockutil_socket(type, sa->ss_family)) == -1)
    return false;
  
  // Bind to sa
  if (nt_sockutil_bind(*fd, sa) != 0) {
    nt_fd_close(fd);
    return false;
  }
  
  // Save address
  if (sa->ss_family == AF_INET6)
    memcpy((void * restrict)&self->addr6, (const void * restrict)sa, sizeof(struct sockaddr_in6));
  else
    memcpy((void * restrict)&self->addr4, (const void * restrict)sa, sizeof(struct sockaddr_in));
  
  return true;
}


bool nt_sockserv_bind(nt_sockserv_t *self, const char *addr, int port, int type, int family) {
  struct addrinfo hints, *servinfo, *ai;
  int rv;
  char strport[12];
  
  assert(self != NULL);
  assert(addr != NULL);
  assert(self->on_accept != NULL);
  
  // Setup getaddrinfo hints
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE; // AI_CANONNAME also ?
  hints.ai_family = family;
  hints.ai_socktype = type;
  
  // getaddrinfo
  snprintf(strport, sizeof(strport), "%d", port);
  rv = getaddrinfo(addr, strport, &hints, &servinfo);
  if (servinfo == NULL || rv == -1) {
    nt_warn("getaddrinfo: %s", gai_strerror(rv));
    return false;
  }
  
  // loop through all the results and bind to the first found addr per af
  for ( ai = servinfo; ai; ai = ai->ai_next ) {
    //printf("%s %d\n", nt_sockaddr_host((const nt_sockaddr_t *)ai->ai_addr),
    //  nt_sockaddr_port((const nt_sockaddr_t *)ai->ai_addr));
    
    if (((ai->ai_family == AF_INET6) ? self->fd6 : self->fd4) != -1)
      continue;
    
    if (!nt_sockserv_bindtoaddr(self, (const nt_sockaddr_t *)ai->ai_addr, ai->ai_socktype))
      break;
  }
  
  freeaddrinfo(servinfo);
  
  return (self->fd4 != -1 || self->fd6 != -1);
}


bool nt_sockserv_listen(nt_sockserv_t *self) {
  if (self->fd4 != -1 && listen(self->fd4, SOMAXCONN) == -1)
    return false;
  if (self->fd6 != -1 && listen(self->fd6, SOMAXCONN) == -1)
    return false;
  return (self->fd4 != -1 || self->fd6 != -1);
}
