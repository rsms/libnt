#include "tcp_server.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>

#include <err.h> /* warnx() */


static void _dealloc(nt_tcp_server *self) {
  size_t i;
  struct event *ev;
  
  // disable and free events
  for (i=0; i<self->n_accept_evs; i++) {
    ev = self->v_accept_evs[i];
    if (ev) {
      event_del(ev);
      free(ev);
    }
  }
  
  // free bs tuples
  for (i=0; i<self->n_bs; i++) {
    free(self->v_bs[i]);
  }
  
  // release sockets
  nt_xrelease(self->socket4);
  nt_xrelease(self->socket6);
  
  // finally free ourselves
  free(self);
}


nt_tcp_server *nt_tcp_server_new(nt_tcp_server_on_accept *on_accept) {
  int i;
  nt_tcp_server *self;
  
  if ( !(self = (nt_tcp_server *)malloc(sizeof(nt_tcp_server))) )
    return NULL;
  
  nt_obj_init((nt_obj *)self, (nt_obj_destructor *)_dealloc);
  
  // Listening sockets
  self->socket4 = NULL;
  self->socket6 = NULL;
  
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


bool nt_tcp_server_bind(nt_tcp_server *server, const char *addr, short port, 
                        bool ipv6_enabled, bool ipv6_only, bool blocking)
{
  struct addrinfo hints, *servinfo, *ptr;
  int rv;
  bool rb;
  char strport[12];
  
  // Check if on_accept is set
  if (server->on_accept == NULL) {
    warnx("nt_tcp_server_bind: server->on_accept is NULL");
    return false;
  }
  
  // Setup getaddrinfo hints
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP
  
  // getaddrinfo
  snprintf(strport, sizeof(strport), "%d", port);
  rv = getaddrinfo(addr, strport, &hints, &servinfo);
  if (servinfo == NULL || rv == -1) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return false;
  }
  
  // loop through all the results and bind to the first we can
  for(ptr = servinfo; ptr; ptr = ptr->ai_next) {
    if (ptr->ai_family == AF_INET6) {
      // v6
      if (ipv6_enabled) {
        if (server->socket6 == NULL)
          server->socket6 = nt_tcp_socket_new(-1);
        rb = nt_tcp_socket_bind(server->socket6, ptr, ipv6_only, blocking);
        rb = rb && nt_tcp_socket_listen(server->socket6, SOMAXCONN);
        if (!rb) {
          freeaddrinfo(servinfo);
          return false;
        }
      }
    }
    else if(!ipv6_only) {
      // v4
      if (server->socket4 == NULL)
        server->socket4 = nt_tcp_socket_new(-1);
      rb = nt_tcp_socket_bind(server->socket4, ptr, false, blocking);
      rb = rb && nt_tcp_socket_listen(server->socket4, SOMAXCONN);
      if (!rb) {
        freeaddrinfo(servinfo);
        return false;
      }
    }
  }
  
  freeaddrinfo(servinfo);
  
  return (server->socket4 || server->socket6);
}


const char *nt_tcp_server_host(nt_tcp_server *server) {
  if (server->socket6)
    return nt_tcp_socket_host(server->socket6);
  else if (server->socket4)
    return nt_tcp_socket_host(server->socket4);
  return NULL;
}

char *nt_tcp_server_hostcpy(nt_tcp_server *server, char *buf, size_t bufsize) {
  if (server->socket6)
    return nt_tcp_socket_hostcpy(server->socket6, buf, bufsize);
  else if (server->socket4)
    return nt_tcp_socket_hostcpy(server->socket4, buf, bufsize);
  return NULL;
}

uint16_t nt_tcp_server_port(nt_tcp_server *server) {
  if (server->socket6)
    return nt_tcp_socket_port(server->socket6);
  else if (server->socket4)
    return nt_tcp_socket_port(server->socket4);
  return 0;
}
