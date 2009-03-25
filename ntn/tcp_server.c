#include "tcp_server.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>


static ntn_tcp_server *_tcp_server_shared = NULL;


ntn_tcp_server *ntn_tcp_server_shared() {
  static ntn_tcp_server server;
  if (_tcp_server_shared == NULL) {
    ntn_tcp_server_init(&server);
    // calling ntn_tcp_server_init sets _tcp_server_shared
  }
  return _tcp_server_shared;
}


bool ntn_tcp_server_init(ntn_tcp_server *server) {
  server->loop = NULL;
  server->socket4 = NULL;
  server->socket6 = NULL;
  server->accept_ev4 = NULL;
  server->accept_ev6 = NULL;
  server->accept_cb = NULL;
  if (_tcp_server_shared == NULL)
    _tcp_server_shared = server;
  return true;
}


void ntn_tcp_server_destroy(ntn_tcp_server *server) {
  if (server->socket4) {
    ntn_tcp_socket_destroy(server->socket4);
    free(server->socket4);
  }
  if (server->socket6) {
    ntn_tcp_socket_destroy(server->socket6);
    free(server->socket6);
  }
  if (server->accept_ev4) {
    free(server->accept_ev4);
  }
  if (server->accept_ev6) {
    free(server->accept_ev6);
  }
}


bool ntn_tcp_server_bind(ntn_tcp_server *server, const char *addr, short port, bool ipv6_enabled, bool ipv6_only) {
  struct addrinfo hints, *servinfo, *ptr;
  int rv;
  bool rb;
  char strport[12];
  
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
          server->socket6 = (ntn_tcp_socket *)malloc(sizeof(ntn_tcp_socket));
        rb = ntn_tcp_socket_bind(server->socket6, ptr, ipv6_only);
        rb = rb && ntn_tcp_socket_listen(server->socket6, SOMAXCONN);
        if (!rb) {
          freeaddrinfo(servinfo);
          return false;
        }
      }
    }
    else if(!ipv6_only) {
      // v4
      if (server->socket4 == NULL)
        server->socket4 = (ntn_tcp_socket *)malloc(sizeof(ntn_tcp_socket));
      rb = ntn_tcp_socket_bind(server->socket4, ptr, false);
      rb = rb && ntn_tcp_socket_listen(server->socket4, SOMAXCONN);
      if (!rb) {
        freeaddrinfo(servinfo);
        return false;
      }
    }
  }
  
  freeaddrinfo(servinfo);
  
  return (server->socket4 || server->socket6);
}


// kinda like a non-blocking accept
bool ntn_tcp_server_start(ntn_tcp_server *server) {
  if ( (server->socket4 == NULL || server->socket4->fd == -1) 
       && (server->socket6 == NULL || server->socket6->fd == -1) )
  {
    fprintf(stderr, "server not bound\n");
    return false;
  }
  
  if (server->accept_cb == NULL) {
    fprintf(stderr, "server->accept_cb is NULL\n");
    return false;
  }
  
  NT_LOOP_ASSURE(server->loop);
  
  if (server->socket4) {
    if (server->accept_ev4 == NULL)
      server->accept_ev4 = malloc(sizeof(ev_io));
    ev_io_init(server->accept_ev4, server->accept_cb, server->socket4->fd, EV_READ);
    ev_io_start(server->loop, server->accept_ev4);
  }
  if (server->socket6) {
    if (server->accept_ev6 == NULL)
      server->accept_ev6 = malloc(sizeof(ev_io));
    ev_io_init(server->accept_ev6, server->accept_cb, server->socket6->fd, EV_READ);
    ev_io_start(server->loop, server->accept_ev6);
  }
  
  return true;
}


const char *ntn_tcp_server_host(ntn_tcp_server *server) {
  if (server->socket6)
    return ntn_tcp_socket_host(server->socket6);
  else if (server->socket4)
    return ntn_tcp_socket_host(server->socket4);
  return NULL;
}

char *ntn_tcp_server_hostcpy(ntn_tcp_server *server, char *buf, size_t bufsize) {
  if (server->socket6)
    return ntn_tcp_socket_hostcpy(server->socket6, buf, bufsize);
  else if (server->socket4)
    return ntn_tcp_socket_hostcpy(server->socket4, buf, bufsize);
  return NULL;
}

uint16_t ntn_tcp_server_port(ntn_tcp_server *server) {
  if (server->socket6)
    return ntn_tcp_socket_port(server->socket6);
  else if (server->socket4)
    return ntn_tcp_socket_port(server->socket4);
  return 0;
}
