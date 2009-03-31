#include "ntn/tcp_server.h"
#include "ntn/util.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>

static void on_accepted(ntn_tcp_server *server, ntn_tcp_client *client) {
}

static void on_disconnect(struct ev_loop *loop, struct ev_signal *w, int revents) {
  printf("client disconnected\n");
}


int main(int argc, char * const *argv) {
  ntn_tcp_server *server;
  char *addr = "";
  int port = 8080;
  
  // parse arguments
  if (argc > 1)
    addr = argv[1];
  if (argc > 2)
    port = atoi(argv[2]);
  
  // Use the shared server
  server = ntn_tcp_server_shared();
  
  // Set callbacks
  server->on_client_accepted = &on_client_accepted;
  server->on_client_disconnected = &on_client_disconnected;
  server->on_client_error = &on_client_error;
  
  // start a tcp server
  if (!ntn_tcp_server_bind(server, addr, port, true, false)) exit(1);
  if (!ntn_tcp_server_start(server)) exit(1);
  
  // process events
  ntn_tcp_server_run(server);
  
  // free tcp server members
  ntn_tcp_server_destroy(server);
  
  // unloop was called, so exit
  return 0;
}
