#ifndef NTN_TCP_SERVER_H
#define NTN_TCP_SERVER_H

#include "util.h"
#include "tcp_socket.h"
#include <stdbool.h>
#include <ev.h>

typedef struct ntn_tcp_server {
  struct ev_loop *loop;
  ntn_tcp_socket *socket4;
  ntn_tcp_socket *socket6;
  ev_io *accept_ev4;
  ev_io *accept_ev6;
  void (*accept_cb)(struct ev_loop *, struct ev_io *, int);
} ntn_tcp_server;

bool ntn_tcp_server_init(ntn_tcp_server *server);
void ntn_tcp_server_destroy(ntn_tcp_server *server);

// Default server instance
ntn_tcp_server *ntn_tcp_server_default();

// Bind server to address and port
bool ntn_tcp_server_bind(ntn_tcp_server *server, const char *addr, short port, bool ipv6_enabled, bool ipv6_only);

// Start the server (adds the servers accept event to the loop)
bool ntn_tcp_server_start(ntn_tcp_server *server);

// Run servers ev_loop
#define ntn_tcp_server_run(server) ev_loop((server)->loop, 0)

// Return the IPv6 or IPv4 address in a human readable format (not thread safe)
const char *ntn_tcp_server_host(ntn_tcp_server *server);

// Copy a human readable string representing the host address of server (in v6 or v4 format).
char *ntn_tcp_server_hostcpy(ntn_tcp_server *server, char *buf, size_t bufsize);

// Port of server socket in host byte order (a normal integer)
uint16_t ntn_tcp_server_port(ntn_tcp_server *server);


#endif
