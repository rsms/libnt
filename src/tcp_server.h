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
**/
#ifndef _NT_TCP_SERVER_H_
#define _NT_TCP_SERVER_H_

#include "obj.h"
#include "sockaddr.h"
#include <stdbool.h>
#include <stdint.h>
#include <event.h>

typedef struct nt_tcp_server_runloop_t {
  struct nt_tcp_server_t *server;
  struct nt_runloop_t *runloop;
} nt_tcp_server_runloop_t;

/**
  Called when a client connection is pending an accept() call.
**/
typedef void (*nt_tcp_server_on_accept_t)(int fd, short ev, nt_tcp_server_runloop_t *bs);

/**
  Server object.
**/
typedef struct nt_tcp_server_t {
  NT_OBJ_HEAD
  
  /* Listening sockets */
  int fd4;
  int fd6;
  
  /* Listening addresses */
  nt_sockaddr_t addr4;
  nt_sockaddr_t addr6;
  
  /* Accept events */
  struct event *ev4;
  struct event *ev6;
  
  /* Callbacks */
  nt_tcp_server_on_accept_t on_accept;
  struct timeval accept_timeout;
  
  /* Internal use only */
  /*volatile int32_t n_accept_evs;
  struct event *v_accept_evs[NT_TCP_SERVER_MAX_ACCEPT_EVS];
  volatile int32_t n_bs;
  struct nt_event_base_server *v_bs[NT_TCP_SERVER_MAX_ACCEPT_EVS];*/
} nt_tcp_server_t;

/**
  Create a new server.
  
  @param  on_accept  accept handler
**/
nt_tcp_server_t *nt_tcp_server_new(nt_tcp_server_on_accept_t on_accept);

/**
  Bind server to a specific address.
  
  @param server server.
  @param sa socket address.
  @returns boolean success.
  @see nt_tcp_server_bind()
**/
bool nt_tcp_server_bindtoaddr(nt_tcp_server_t *server, const nt_sockaddr_t *sa, void *ai);

/**
  Bind server to address and port.
  
  @param server server
  @param addr   address
  @param port   port number
  @param proto  set to 0 to bind to both IPv4 and/or IPv6 addresses.
                set to IPPROTO_IPV4 to only bind to IPv4 addresses.
                set to IPPROTO_IPV6 to bind to only IPv6 addresses.
  @returns boolean success
  @see nt_tcp_server_bindtoaddr()
**/
bool nt_tcp_server_bind(nt_tcp_server_t *server, const char *addr, int port, int proto);


#endif
