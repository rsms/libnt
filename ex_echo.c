#include "src/event_base.h"
#include "src/tcp_server.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <event.h>

/*static void on_client_accepted(nt_tcp_server *server, int fd, short ev) {
}

static void on_client_disconnected(struct ev_loop *loop, struct ev_signal *w, int revents) {
  printf("client disconnected\n");
}

static void on_client_error(struct bufferevent *bev, short what, void *arg) {
  printf("client error\n");
}*/


int main(int argc, char * const *argv) {
  nt_tcp_server *server;
  nt_event_base *base;
  char *addr = "";
  int port = 8080, rc;
  bool ipv6_enabled = true;
  bool ipv6_only = false;
  
  // parse arguments
  if (argc > 1)
    addr = argv[1];
  if (argc > 2)
    port = atoi(argv[2]);

  // ------------ server: ------------
  
  // Create server
  server = nt_tcp_server_new();
  
  // Set callbacks
  //server->on_client_accepted = &on_client_accepted;
  //server->on_client_disconnected = &on_client_disconnected;
  //server->on_client_error = &on_client_error;
  
  // bind
  if (!nt_tcp_server_bind(server, addr, port, ipv6_enabled, ipv6_only))
    exit(1);
  
  // print some info to stdout
  if (server->socket4) {
    printf("bound to %s:%d\n", nt_tcp_socket_host(server->socket4),
      nt_tcp_socket_port(server->socket4));
  }
  if (server->socket6) {
    printf("bound to [%s]:%d\n", nt_tcp_socket_host(server->socket6),
      nt_tcp_socket_port(server->socket6));
  }
  
  // ------------ base: ------------
  
  base = nt_event_base_default();
  
  // Add our server
  if (!nt_event_base_add_server(base, server, NULL))
    exit(1);
  
  // Enter event base runloop
  rc = nt_event_base_loop(base, 0);
  if (rc < 0)
    warnx("nt_event_base_loop: error");
  else if (rc > 0)
    warnx("nt_event_base_loop: no events");
  
  // release our reference created by nt_tcp_server_new
  nt_release(server);
  
  return 0;
}
