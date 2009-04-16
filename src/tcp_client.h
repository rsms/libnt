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
**/
#ifndef _NT_TCP_CLIENT_H_
#define _NT_TCP_CLIENT_H_

#include "obj.h"
#include "sockaddr.h"
#include "tcp_server.h"
#include <event.h>

/**
  TCP client connection with buffered I/O.
  
  A client occupies 20 bytes on a 32-bit system and 36-40 bytes on a 64-bit system.
**/
typedef struct nt_tcp_client_t {
  NT_OBJ_HEAD
  int fd;                       /* Socket */
  nt_sockaddr_t addr;           /* Address */
  nt_tcp_server_runloop_t *rs;  /* runloop-server tuple */
  struct bufferevent bev;       /* Buffer event */
} nt_tcp_client_t;


/**
  Called when there is data for the client to read.
**/
typedef void (*nt_tcp_client_readcb_t)(struct bufferevent *bev, nt_tcp_client_t *client);

/**
  Called when the write buffer reaches 0.
**/
typedef void (*nt_tcp_client_writecb_t)(struct bufferevent *bev, nt_tcp_client_t *client);

/**
  Called when an error occured.
  This callback is responsible for releasing the client.
**/
typedef void (*nt_tcp_client_errorcb_t)(struct bufferevent *bev, short what, nt_tcp_client_t *client);


/**
  Create a new nt_tcp_client_t object
**/
nt_tcp_client_t *nt_tcp_client_new();

/**
  Set the file descriptor.
  
  @param fd file descriptor
**/
void nt_tcp_client_setfd(nt_tcp_client_t *self, int fd);

/**
  Set server-runloop tuple.
  
  @param rs server-runloop tuple or NULL to clear.
**/
void nt_tcp_client_setrs(nt_tcp_client_t *self, nt_tcp_server_runloop_t *rs);

/**
  Set callbacks.
  
  @param readcb called when there is something to read
  @param writecb called when there is something to write
  @param errorcb called when an error occured
**/
void nt_tcp_client_setcb( nt_tcp_client_t *self,
                          nt_tcp_client_readcb_t readcb,
                          nt_tcp_client_writecb_t writecb,
                          nt_tcp_client_errorcb_t errorcb);

/**
  Accept a client connection.
  
  @param fd file descriptor on which an accept event is pending
  @param readcb called when there is something to read
  @param writecb called when there is something to write
  @param errorcb called when an error occured
  @returns a new client
**/
nt_tcp_client_t *nt_tcp_client_accept(nt_tcp_server_runloop_t *rs,
                                      int fd,
                                      nt_tcp_client_readcb_t readcb,
                                      nt_tcp_client_writecb_t writecb,
                                      nt_tcp_client_errorcb_t errorcb);

/**
  Send data to the client.
  
  @param data bytes to send
  @param size number of bytes to send
**/
NT_STATIC_INLINE
void nt_tcp_client_write(nt_tcp_client_t *self, const void *data, size_t size) {
  AZ(bufferevent_write(&self->bev, data, size));
}

/**
  Send a buffer to the client.
  
  @param buf the evbuffer to send
**/
NT_STATIC_INLINE
void nt_tcp_client_writebuf(nt_tcp_client_t *self, struct evbuffer *buf) {
  AZ(bufferevent_write_buffer(&self->bev, buf));
}

/**
  Close a client connection.
**/
void nt_tcp_client_close(nt_tcp_client_t *self);


#endif
