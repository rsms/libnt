#ifndef NTN_TCP_SERVER_H
#define NTN_TCP_SERVER_H

#include "obj.h"
#include "util.h"
#include "tcp_socket.h"

#include <stdbool.h>
#include <event.h>

/**
  Maximum number of accept events.
  
  Each call to nt_tcp_server_register() adds <number of bound interfaces> to 
  v_accept_evs -- if the number of accept events reach 
  NT_TCP_SERVER_MAX_ACCEPT_EVS, calls to nt_tcp_server_register() will fail.
  
  Normally you need no more than
  (<number of processors> * <number of bound interfaces>) accept events.
*/
#define NT_TCP_SERVER_MAX_ACCEPT_EVS 32

struct nt_tcp_server;

/**
  Callback for when a client connection is pending an accept() call.
*/
typedef void (nt_tcp_server_on_accept)(int client_fd, short ev, struct nt_tcp_server *server);

/**
  nt_tcp_server object.
*/
typedef struct nt_tcp_server {
  NT_OBJ_HEAD
  
  /* Listening sockets */
  nt_tcp_socket *socket4;
  nt_tcp_socket *socket6;
  
  /* Callbacks */
  nt_tcp_server_on_accept *on_accept;
  
  /* Internal use only */
  size_t n_accept_evs;
  struct event *v_accept_evs[NT_TCP_SERVER_MAX_ACCEPT_EVS];
} nt_tcp_server;

/**
  Create a new nt_tcp_server
*/
nt_tcp_server *nt_tcp_server_new();

/**
  Bind server to address and port
*/
bool nt_tcp_server_bind(nt_tcp_server *server, const char *addr, short port,
                        bool ipv6_enabled, bool ipv6_only);

/**
  Runs the servers loop. Possible flags:
    EVLOOP_NONBLOCK -- do not block/wait
    EVLOOP_ONESHOT  -- block *once* only
*/
//#define nt_tcp_server_run(server, int_flags) ev_loop((server)->loop, int_flags)

/**
  Return the IPv6 or IPv4 address in a human readable format (not thread safe).
*/
const char *nt_tcp_server_host(nt_tcp_server *server);

/**
  Copy a human readable string representing the host address of server (in v6 or v4 format).
*/
char *nt_tcp_server_hostcpy(nt_tcp_server *server, char *buf, size_t bufsize);

/**
  Port of server socket in host byte order (a normal integer)
*/
uint16_t nt_tcp_server_port(nt_tcp_server *server);


#endif
