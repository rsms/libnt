#include "src/event_base.h"
#include "src/tcp_server.h"
#include "src/tcp_client.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <event.h>

static void on_client_connected(nt_tcp_client *client) {
  printf("client %p connected from %s\n", client, nt_tcp_socket_host(client->socket));
}

static void on_client_disconnected(nt_tcp_client *client) {
  printf("client %p disconnected\n", client);
  nt_release(client);
}

static void on_client_error(struct bufferevent *bev, short what, nt_tcp_client *client) {
  if (what & EVBUFFER_EOF) {
    on_client_disconnected(client);
  }
  else {
    printf("client %p error\n", client);
  }
  nt_release(client);
}

static void on_accept(int fd, short ev, nt_event_base_server *bs) {
  nt_tcp_client *client;
  if ( (client = nt_tcp_client_accept(bs, fd, NULL, NULL, &on_client_error)) )
    on_client_connected(client);
}


int main(int argc, char * const *argv) {
  nt_tcp_server *server;
  nt_event_base *base;
  char *addr = "";
  int port = 8080, rc;
  bool ipv6_enabled = true;
  bool ipv6_only = false;
  bool blocking = false;
  
  // parse arguments
  if (argc > 1)
    addr = argv[1];
  if (argc > 2)
    port = atoi(argv[2]);

  // ------------ server: ------------
  
  // Create server
  server = nt_tcp_server_new(&on_accept);
  
  // bind
  if (!nt_tcp_server_bind(server, addr, port, ipv6_enabled, ipv6_only, blocking))
    exit(1);
  
  // print some info to stdout
  if (server->socket4) {
    printf("listening on %s:%d\n", nt_tcp_socket_host(server->socket4),
      nt_tcp_socket_port(server->socket4));
  }
  if (server->socket6) {
    printf("listening on [%s]:%d\n", nt_tcp_socket_host(server->socket6),
      nt_tcp_socket_port(server->socket6));
  }
  
  // ------------ base: ------------
  
  base = nt_event_base_new();
  
  // Add our server
  if (!nt_event_base_add_server(base, server, NULL))
    exit(1);
  
  // Enter event base runloop
  rc = nt_event_base_loop(base, 0);
  if (rc < 0)
    warnx("nt_event_base_loop: error");
  else if (rc > 0)
    warnx("nt_event_base_loop: no events");
  
  // release our reference to base
  nt_release(base);
  
  // release our reference to server
  nt_release(server);
  
  return 0;
}
