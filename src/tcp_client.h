/**
  TCP client connection.
  
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
#ifndef _NT_TCP_CLIENT_H_
#define _NT_TCP_CLIENT_H_

#include "obj.h"
#include "tcp_fd.h"
#include <event.h>

struct nt_event_base;
struct nt_event_base_server;
struct nt_tcp_server;


/**
  TCP client connection with buffered I/O.
  
  A client occupies 20 bytes on a 32-bit system and 36-40 bytes on a 64-bit system.
*/
typedef struct nt_tcp_client_t {
  NT_OBJ_HEAD
  int fd; /* Socket */
  struct sockaddr_storage addr; /* Address */
  struct nt_event_base_server *bs; /* runloop-server tuple */
  struct bufferevent *bev; /* Buffer event */
} nt_tcp_client_t;


/**
  Called when there is data for the client to read.
*/
typedef void (nt_tcp_client_on_read)(struct bufferevent *bev, nt_tcp_client_t *client);

/**
  Called when the write buffer reaches 0.
*/
typedef void (nt_tcp_client_on_write)(struct bufferevent *bev, nt_tcp_client_t *client);

/**
  Called when an error occured.
  This callback is responsible for releasing the client.
*/
typedef void (nt_tcp_client_on_error)(struct bufferevent *bev, short what, nt_tcp_client_t *client);


/**
  Create a new nt_tcp_client_t object
*/
nt_tcp_client_t *nt_tcp_client_new(nt_tcp_socket *socket);

/**
  Accept a new client.
  Returns a new reference.
*/
nt_tcp_client_t *nt_tcp_client_accept(struct nt_event_base_server *bs,
                                    int server_fd,
                                    nt_tcp_client_on_read *on_read,
                                    nt_tcp_client_on_write *on_write,
                                    nt_tcp_client_on_error *on_error);

#endif
