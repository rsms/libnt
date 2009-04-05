/**
  TCP server.
  
  Copyright (c) 2009 Notion <http://notion.se/>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
#ifndef _NT_TCP_SERVER_H_
#define _NT_TCP_SERVER_H_

#include "obj.h"
#include "util.h"
#include "tcp_socket.h"

#include <stdbool.h>
#include <stdint.h>
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

struct nt_event_base;
struct nt_event_base_server;
struct nt_tcp_server;

/**
  Callback for when a client connection is pending an accept() call.
*/
typedef void (nt_tcp_server_on_accept)(int fd, short ev, struct nt_event_base_server *bs);

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
  volatile int32_t n_accept_evs;
  struct event *v_accept_evs[NT_TCP_SERVER_MAX_ACCEPT_EVS];
  volatile int32_t n_bs;
  struct nt_event_base_server *v_bs[NT_TCP_SERVER_MAX_ACCEPT_EVS];
} nt_tcp_server;

/**
  Create a new nt_tcp_server
  
  @param  on_accept  accept handler
*/
nt_tcp_server *nt_tcp_server_new(nt_tcp_server_on_accept *on_accept);

/**
  Bind server to address and port
*/
bool nt_tcp_server_bind(nt_tcp_server *server, const char *addr, short port,
                        bool ipv6_enabled, bool ipv6_only, bool blocking);

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
