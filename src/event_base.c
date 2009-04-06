/**
  libevent event base nt object.
  
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
#include "event_base.h"
#include "atomic.h"
#include "mpool.h"

static void _dealloc(nt_event_base *self) {
  if (self->ev_base)
    event_base_free(self->ev_base);
  nt_free(self, sizeof(nt_event_base));
}


/* defined in libevent event.c */
extern struct event_base *current_base;

nt_event_base *nt_shared_event_base = NULL;

nt_event_base *nt_event_base_default() {
  if (!nt_shared_event_base) {
    nt_shared_event_base = nt_event_base_new();
    event_base_free(nt_shared_event_base->ev_base);
    if (!current_base) {
      event_init();
    }
    nt_shared_event_base->ev_base = current_base;
  }
  return nt_shared_event_base;
}


nt_event_base *nt_event_base_new() {
  nt_event_base *self;
  if ((self = (nt_event_base *)nt_malloc(sizeof(nt_event_base))) == NULL)
    return NULL;
  nt_obj_init((nt_obj *)self, (nt_obj_deallocator *)_dealloc);
  self->ev_base = event_base_new();
  return self;
}


bool nt_event_base_add_socket(nt_event_base *self,
                              nt_tcp_socket *sock,
                              struct event *ev,
                              int flags,
                              const struct timeval *timeout,
                              void (*cb)(int, short, void *),
                              void *cbarg)
{
  event_set(ev, sock->fd, flags, cb, cbarg);
  
  if (event_base_set(self->ev_base, ev) != 0) {
    warnx("nt_event_base_add_socket: event_base_set() failed");
    return false;
  }
  
  if (event_add(ev, timeout) != 0) {
    event_del(ev); // todo xxx really del if add failed?
    warnx("nt_event_base_add_socket: event_add() failed");
    return false;
  }
  
  return true;
}


/* helper used by nt_event_base_add_server */
static inline struct event *_mk_add_accept_ev(nt_event_base_server *bs,
                                              nt_tcp_socket *sock,
                                              const struct timeval *timeout )
{
  struct event *ev;
  bool success;
  
  ev = (struct event *)nt_malloc(sizeof(struct event));
  if (!ev) {
    /* errno == ENOMEM */
    return NULL;
  }
  
  success = nt_event_base_add_socket( bs->base, sock, ev, EV_READ|EV_PERSIST, timeout,
                                      (void (*)(int, short, void *))bs->server->on_accept,
                                      (void *)bs );
  if (!success) {
    free(ev);
    ev = NULL;
  }
  
  return ev;
}


bool nt_event_base_add_server(nt_event_base *base, nt_tcp_server *server, const struct timeval *timeout) 
{
  struct event *ev;
  nt_event_base_server *bs;
  size_t num_sockets = (server->socket4 ? 1 : 0) + (server->socket6 ? 1 : 0);
  
  if (num_sockets == 0) {
    warnx("nt_tcp_server_register: server is not bound (no sockets)");
    return false;
  }
  
  // Check if we have space for more accept events
  if ( (nt_atomic_read32(&server->n_accept_evs) + num_sockets) > NT_TCP_SERVER_MAX_ACCEPT_EVS ) {
    warnx("nt_tcp_server_register: too many accept events");
    return false;
  }
  
  // base+server conduit
  bs = (nt_event_base_server *)nt_malloc(sizeof(nt_event_base_server));
  bs->base = base;
  bs->server = server;
  server->v_bs[nt_atomic_fetch_and_inc32(&server->n_bs)] = (void *)bs;
  
  // Register IPv4 socket
  if (server->socket4) {
    if ((ev = _mk_add_accept_ev(bs, server->socket4, timeout)) == NULL)
      return false;
    server->v_accept_evs[nt_atomic_fetch_and_inc32(&server->n_accept_evs)] = ev;
  }
  
  // Register IPv6 socket
  if (server->socket6) {
    if ((ev = _mk_add_accept_ev(bs, server->socket6, timeout)) == NULL)
      return false;
    server->v_accept_evs[nt_atomic_fetch_and_inc32(&server->n_accept_evs)] = ev;
  }
  
  return true;
}
