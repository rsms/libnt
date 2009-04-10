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
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>


static void _dealloc(nt_tcp_server_t *self) {
  size_t i;
  struct event *ev;
  
  // disable and free events
  for (i=0; i<self->n_accept_evs; i++) {
    ev = self->v_accept_evs[i];
    if (ev) {
      event_del(ev);
      nt_free(ev, sizeof(struct event));
    }
  }
  
  // free bs tuples
  for (i=0; i<self->n_bs; i++) {
    nt_free(self->v_bs[i], sizeof(nt_event_base_server));
  }
  
  // finally free ourselves
  nt_free(self, sizeof(nt_tcp_server_t));
}


nt_tcp_server_t *nt_tcp_server_new(nt_tcp_server_on_accept_t on_accept) {
  int i;
  nt_tcp_server_t *self;
  
  if ( !(self = (nt_tcp_server_t *)nt_malloc(sizeof(nt_tcp_server_t))) )
    return NULL;
  
  nt_obj_init((nt_obj *)self, (nt_obj_deallocator *)_dealloc);
  
  // Listening sockets
  self->socket4.fd = -1;
  memset(&self->socket4.addr, 0, sizeof(nt_sockaddr_t));
  self->socket6.fd = -1;
  memset(&self->socket6.addr, 0, sizeof(nt_sockaddr_t));
  
  // Callbacks
  self->on_accept = on_accept;
  
  // Init list of accept events
  self->n_accept_evs = 0U;
  for (i=0; i<NT_TCP_SERVER_MAX_ACCEPT_EVS; i++) {
    self->v_accept_evs[i] = NULL;
  }
  
  // Init list of bs tuples 
  self->n_bs = 0U;
  for (i=0; i<NT_TCP_SERVER_MAX_ACCEPT_EVS; i++) {
    self->v_bs[i] = NULL;
  }
  
  return self;
}


NT_INLINE_STATIC bool _bind_and_listen(int *fd, const struct sockaddr *sa, bool ipv6) {
  if ((*fd = nt_fd_tcp_socket(ipv6)) == -1) {
    perror("socket");
    return false;
  }
  if (nt_fd_tcp_bind(*fd, (const nt_sockaddr_t *)sa) != 0) {
    perror("bind");
    return false;
  }
  nt_fd_tcp_listen(*fd, SOMAXCONN);
  return true;
}


bool nt_tcp_server_bind(nt_tcp_server_t *server, const char *addr, int port, int proto) {
  struct addrinfo hints, *servinfo, *ai;
  int rv;
  char strport[12];
  
  assert(server->on_accept != NULL);
  assert(server->socket4.fd == -1);
  assert(server->socket6.fd == -1);
  
  // Setup getaddrinfo hints
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE; // AI_CANONNAME also ?
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = proto;
  
  // getaddrinfo
  snprintf(strport, sizeof(strport), "%d", port);
  rv = getaddrinfo(addr, strport, &hints, &servinfo);
  if (servinfo == NULL || rv == -1) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return false;
  }
  
  // loop through all the results and bind to the first found addr per af
  for ( ai = servinfo; ai; ai = ai->ai_next ) {
    if (ai->ai_protocol == IPPROTO_IPV6) {
      if (!_bind_and_listen(&server->socket6, ai->ai_addr, true))
        break;
    }
    else if (ai->ai_protocol == IPPROTO_IPV6) {
      if (!_bind_and_listen(&server->socket4, (const nt_sockaddr_t *)ai->ai_addr, false))
        break;
    }
    else {
      log_debug("unknown protocol %d", ai->ai_protocol);
    }
  }
  
  freeaddrinfo(servinfo);
  
  return (server->socket4 != -1 || server->socket6 != -1);
}
