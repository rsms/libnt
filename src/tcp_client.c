/**
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
#include "tcp_client.h"
#include "tcp_server.h"
#include "event_base.h"


static void _dealloc(nt_tcp_client *self) {
  // free the bufferevent
  bufferevent_free(self->bev);
  
  // release socket. implies closing any connection
  nt_xrelease(self->socket);
  
  // finally free ourselves
  free(self);
}


nt_tcp_client *nt_tcp_client_new(nt_tcp_socket *socket) {
  nt_tcp_client *self;
  
  if ( !(self = (nt_tcp_client *)malloc(sizeof(nt_tcp_client))) )
    return NULL;
  
  nt_obj_init((nt_obj *)self, (nt_obj_destructor *)_dealloc);
  
  // Socket
  nt_xretain(socket);
  self->socket = socket;
  
  self->bev = NULL;
  self->bs = NULL;
  
  return self;
}


static void _bev_on_error(struct bufferevent *bev, short what, nt_tcp_client *client) {
  if (!(what & EVBUFFER_EOF)) {
    warn("client socket error -- disconnecting client\n");
  }
  nt_xrelease(client);
}


nt_tcp_client *nt_tcp_client_accept(nt_event_base_server *bs,
                                    int server_fd,
                                    nt_tcp_client_on_read *on_read,
                                    nt_tcp_client_on_write *on_write,
                                    nt_tcp_client_on_error *on_error)
{
  nt_tcp_socket *sock;
  nt_tcp_client *client;
  int af, evflags;
  
  af = (bs->server->socket6 && bs->server->socket6->fd == server_fd) ? AF_INET6 : AF_INET;
  
  // Accept connection
  if ( (sock = nt_tcp_socket_accept(server_fd, af)) == NULL )
    return NULL;
  
  // Set non-blocking
  if (nt_util_fd_setnonblock(sock->fd) == -1)
    return NULL;
  
  // Create a new client
  if ( (client = nt_tcp_client_new(sock)) == NULL ) {
    nt_release(sock);
    return NULL;
  }
  
  // As nt_tcp_client_new retains a reference to sock, we release ours here
  nt_release(sock);
  
  // Set pointer to base-server tuple
  client->bs = bs;
  
  // Set evflags
  evflags = 0;
  if (on_read)
    evflags |= EV_READ;
  if (on_write)
    evflags |= EV_WRITE;
  
  // Make sure the error callback is set
  if (on_error == NULL)
    on_error = &_bev_on_error;
  
  // Check so that we handle at least one event
  if (evflags == 0) {
    warnx("nt_tcp_client_accept: neither read nor write callbacks was specified");
    nt_release(client);
    return NULL;
  }
  
  // Create a new bufferevent
  client->bev = bufferevent_new(sock->fd,
    (void (*)(struct bufferevent *, void *))on_read,
    (void (*)(struct bufferevent *, void *))on_write,
    (void (*)(struct bufferevent *, short, void *))on_error,
    (void *)client);
  if (client->bev == NULL) {
    nt_release(client);
    return NULL;
  }
  
  // Set base if not NULL
  if (bs->base) {
    if (bufferevent_base_set(bs->base->ev_base, client->bev) == -1) {
      nt_release(client);
      return NULL;
    }
  }
  
  // Enable events
  if (bufferevent_enable(client->bev, evflags) != 0) {
    warnx("nt_tcp_client_accept: bufferevent_enable() failed");
    nt_release(client);
    return NULL;
  }
  
  return client;
}
