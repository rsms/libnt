#include "ntn/tcp_server.h"
#include "ntn/util.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>

static bool is_gracefully_shuttind_down = false;

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

static void handle_stop(struct ev_loop *loop, struct ev_signal *w, int revents) {
  printf("stopping server\n");
  ev_unloop(loop, EVUNLOOP_ALL);
}

static void handle_graceful(struct ev_loop *loop, struct ev_signal *w, int revents) {
  printf("gracefully stopping server\n");
  // wait for client connections to disconnect
  ev_unloop(loop, EVUNLOOP_ALL);
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
  
  // start a tcp server
  server->accept_cb = &handle_accept;
  if (!ntn_tcp_server_bind(server, addr, port, true, false)) exit(1);
  if (!ntn_tcp_server_start(server)) exit(1);
  
  // register signal handlers
  ev_signal sigintn_w;
  ntn_util_sig_register(&sigintn_w, SIGINT, handle_stop);
  ev_signal sigintn_w;
  ntn_util_sig_register(&sigintn_w, SIGUSR1, handle_graceful);
  
  // Print info
  print_info(server);
  
  // process events
  ntn_tcp_server_run(server, 0);
  
  // free tcp server members
  ntn_tcp_server_destroy(server);
  
  // unloop was called, so exit
  return 0;
}
