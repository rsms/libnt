#include "ntn/tcp_server.h"
#include "ntn/util.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>

static void handle_accept(struct ev_loop *loop, struct ev_io *w, int revents) {
  int clientn_fd;
  struct sockaddr_in clientn_addr;
  socklen_t clientn_len = sizeof(clientn_addr);
  
  while (revents--) {
    // we need to accept each event or we'll get in to an loop of death. scary.
    clientn_fd = accept(w->fd, (struct sockaddr *)&clientn_addr, &clientn_len);
    close(clientn_fd); // as we won't use it anymore in this test
    printf("accepted and closed fd %d\n", clientn_fd);
  }
}

static void print_info(ntn_tcp_server *server) {
  printf("libev backend: %s\n", ntn_util_ev_strbackend(ev_backend(server->loop)));
  if (server->socket6) {
    printf("listening on [%s]:%d\n", 
      ntn_tcp_socket_host(server->socket6), ntn_tcp_socket_port(server->socket6));
  }
  if (server->socket4) {
    printf("listening on %s:%d\n",
      ntn_tcp_socket_host(server->socket4), ntn_tcp_socket_port(server->socket4));
  }
}

int main(int argc, char * const *argv) {
  // usage: prog [bindaddr [bindport]]
  ntn_tcp_server *server;
  char *addr = "";
  int port = 8080;
  
  // Command line arguments
  if (argc > 1)
    addr = argv[1];
  if (argc > 2)
    port = atoi(argv[2]);
  
  // Use the shared server
  server = ntn_tcp_server_shared();
  
  // Set our accept handler
  server->accept_cb = &handle_accept;
  
  // Bind (and listen())
  if (!ntn_tcp_server_bind(server, addr, port, true, false)) exit(1);
  
  // "Start" can be thought of as a non-blocking accept(). What happens here
  // is simply the servers I/O (accept) event watchers are added to the
  // runloop server->loop.
  if (!ntn_tcp_server_start(server)) exit(1);
  
  // Register signal handlers.
  // Passing NULL for the third argument implies using the built-in unloop-all
  // handler, effectively aborting the event runloop.
  ev_signal sigintn_w;
  ntn_util_sig_register(&sigintn_w, SIGINT, NULL);
  
  // Print info
  print_info(server);
  
  // process events
  ntn_tcp_server_run(server, 0);
  // we came all the way down here, which means unloop was called.
  
  // free tcp server members
  ntn_tcp_server_destroy(server);
  
  return 0;
}
