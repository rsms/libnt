/**
  Socket server.
  
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
#ifndef _NT_SOCKSERV_H_
#define _NT_SOCKSERV_H_

#include "obj.h"
#include "sockaddr.h"
#include <event.h>
#include <sys/socket.h>


typedef struct nt_sockserv_runloop_t {
  struct nt_sockserv_t *server;
  struct nt_runloop_t *runloop;
} nt_sockserv_runloop_t;

/**
  Called when a client connection is pending an accept() call.
**/
typedef void (*nt_sockserv_on_accept_t)(int fd, short ev, nt_sockserv_runloop_t *bs);

/**
  Server object.
**/
typedef struct nt_sockserv_t {
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
  nt_sockserv_on_accept_t on_accept;
  struct timeval accept_timeout;
  
} nt_sockserv_t;

/**
  Create a new socket server.
  
  @param  on_accept  accept handler
**/
nt_sockserv_t *nt_sockserv_new(nt_sockserv_on_accept_t on_accept);

/**
  Bind server to a specific address.
  
  @param server server.
  @param sa     socket address.
  @param type   SOCK_STREAM or SOCK_DGRAM
  @returns boolean success.
  @see nt_sockserv_bind()
**/
bool nt_sockserv_bindtoaddr(nt_sockserv_t *self, const nt_sockaddr_t *sa, int type);

/**
  Bind server to address/hostname and port.
  
  Only for binding to TCP or UDP -- you should use nt_sockserv_bindtoaddr for
  binding to UNIX sockets.
  
  @param server server
  @param addr   address
  @param port   port number
  @param type   SOCK_STREAM or SOCK_DGRAM
  @param family set to AF_INET to bind to IPv4.
                set to AF_INET6 to bind to IPv6.
                set to AF_UNSPEC to bind to both IPv4 and IPv6
  @returns boolean success
  @see nt_sockserv_bindtoaddr()
**/
bool nt_sockserv_bind(nt_sockserv_t *self, const char *addr, int port, int type, int family);


/**
  Listen for incoming connections.
**/
bool nt_sockserv_listen(nt_sockserv_t *self);

#endif
