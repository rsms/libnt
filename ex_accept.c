#include "ntn/tcp_server.h"
#include "ntn/util.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>

void accept_handler(struct ev_loop *loop, struct ev_io *w, int revents) {
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

int main(int argc, char * const *argv) {
  ntn_tcp_server *server;
  char *addr = "";
  int port = 8080;
  
  // parse arguments
  if (argc > 1)
    addr = argv[1];
  if (argc > 2)
    port = atoi(argv[2]);
  
  // start a tcp server
  server = ntn_tcp_server_default();
  server->accept_cb = &accept_handler;
  if (!ntn_tcp_server_bind(server, addr, port, true, false)) exit(1);
  if (!ntn_tcp_server_start(server)) exit(1);
  
  // register signal handlers
  ev_signal sigintn_w;
  ntn_util_sig_register(server->loop, &sigintn_w, SIGINT, NULL);
  
  // Print info
  printf("libev backend: %s\n", ntn_util_ev_strbackend(ev_backend(server->loop)));
  if (server->socket6) {
    printf("listening on %s:%d\n", ntn_tcp_socket_host(server->socket6), 
                                   ntn_tcp_socket_port(server->socket6));
  }
  if (server->socket4) {
    printf("listening on %s:%d\n", ntn_tcp_socket_host(server->socket4), 
                                   ntn_tcp_socket_port(server->socket4));
  }
  
  // process events
  // Possible flags:
  //  EVLOOP_NONBLOCK -- do not block/wait
  //  EVLOOP_ONESHOT  -- block *once* only
  ntn_tcp_server_run(server);
  
  // free tcp server members
  ntn_tcp_server_destroy(server);
  
  // unloop was called, so exit
  return 0;
}
