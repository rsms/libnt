#ifndef NTN_TCP_SERVER_H
#define NTN_TCP_SERVER_H

#include "util.h"
#include "tcp_socket.h"
#include <stdbool.h>
#include <ev.h>

typedef struct nt_tcp_server {
  struct ev_loop *loop;
  nt_tcp_socket *socket4;
  nt_tcp_socket *socket6;
  ev_io *accept_ev4;
  ev_io *accept_ev6;
  void (*accept_cb)(struct ev_loop *, struct ev_io *, int);
} nt_tcp_server;

bool nt_tcp_server_init(nt_tcp_server *server);
void nt_tcp_server_destroy(nt_tcp_server *server);

// Default server instance
nt_tcp_server *nt_tcp_server_shared();

// Bind server to address and port
bool nt_tcp_server_bind(nt_tcp_server *server, const char *addr, short port, bool ipv6_enabled, bool ipv6_only);

// Start the server (adds the servers accept event to the loop)
bool nt_tcp_server_start(nt_tcp_server *server);

// Runs the servers loop. Possible flags:
//  EVLOOP_NONBLOCK -- do not block/wait
//  EVLOOP_ONESHOT  -- block *once* only
#define nt_tcp_server_run(server, int_flags) ev_loop((server)->loop, int_flags)

// Return the IPv6 or IPv4 address in a human readable format (not thread safe)
const char *nt_tcp_server_host(nt_tcp_server *server);

// Copy a human readable string representing the host address of server (in v6 or v4 format).
char *nt_tcp_server_hostcpy(nt_tcp_server *server, char *buf, size_t bufsize);

// Port of server socket in host byte order (a normal integer)
uint16_t nt_tcp_server_port(nt_tcp_server *server);


#endif
