/**
  Socket (TCP or UNIX) connection with buffered I/O.
  
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
#ifndef _NT_SOCKCONN_H_
#define _NT_SOCKCONN_H_

#include "obj.h"
#include "sockaddr.h"
#include "sockserv.h"
#include <event.h>


typedef struct nt_sockconn_t {
  NT_OBJ_HEAD
  int fd;                       /* socket */
  nt_sockaddr_t addr;           /* address */
  nt_sockserv_runloop_t *rs;  /* runloop-server tuple */
  struct bufferevent bev;       /* buffer event */
} nt_sockconn_t;


/**
  Called when there is data for the client to read.
**/
typedef void (*nt_sockconn_readcb_t)(struct bufferevent *bev, nt_sockconn_t *client);

/**
  Called when the write buffer reaches 0.
**/
typedef void (*nt_sockconn_writecb_t)(struct bufferevent *bev, nt_sockconn_t *client);

/**
  Called when an error occured.
  This callback is responsible for releasing the client.
**/
typedef void (*nt_sockconn_errorcb_t)(struct bufferevent *bev, short what, nt_sockconn_t *client);


/**
  Create a new nt_sockconn_t object
**/
nt_sockconn_t *nt_sockconn_new();

/**
  Set the file descriptor.
  
  @param fd file descriptor
**/
void nt_sockconn_setfd(nt_sockconn_t *self, int fd);

/**
  Set server-runloop tuple.
  
  @param rs server-runloop tuple or NULL to clear.
**/
void nt_sockconn_setrs(nt_sockconn_t *self, nt_sockserv_runloop_t *rs);

/**
  Set callbacks.
  
  @param readcb called when there is something to read
  @param writecb called when there is something to write
  @param errorcb called when an error occured
**/
void nt_sockconn_setcb( nt_sockconn_t *self,
                        nt_sockconn_readcb_t readcb,
                        nt_sockconn_writecb_t writecb,
                        nt_sockconn_errorcb_t errorcb);

/**
  Set read and write timeout in seconds.
**/
NT_STATIC_INLINE
void nt_sockconn_settimeout(nt_sockconn_t *self, int read, int write) {
  self->bev.timeout_read = read;
  self->bev.timeout_write = write;
}

/**
  Accept a connection.
  
  @param fd file descriptor on which an accept event is pending
  @param readcb called when there is something to read
  @param writecb called when there is something to write
  @param errorcb called when an error occured
  @returns boolean success
**/
bool nt_sockconn_accept(nt_sockconn_t *self,
                        nt_sockserv_runloop_t *rs,
                        int fd,
                        nt_sockconn_readcb_t readcb,
                        nt_sockconn_writecb_t writecb,
                        nt_sockconn_errorcb_t errorcb);

/**
  Send data to the client.
  
  @param data bytes to send
  @param size number of bytes to send
**/
NT_STATIC_INLINE
void nt_sockconn_write(nt_sockconn_t *self, const void *data, size_t size) {
  AZ(bufferevent_write(&self->bev, data, size));
}

/**
  Send a buffer to the client.
  
  @param buf the evbuffer to send
**/
NT_STATIC_INLINE
void nt_sockconn_writebuf(nt_sockconn_t *self, struct evbuffer *buf) {
  AZ(bufferevent_write_buffer(&self->bev, buf));
}

/**
  Close a client connection.
**/
void nt_sockconn_close(nt_sockconn_t *self);

/**
  Abort a client connection.
  
  Disables lingering and calls nt_sockconn_close()
**/
void nt_sockconn_abort(nt_sockconn_t *self);


#endif
