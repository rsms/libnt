/**
  Runloop.
  
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
#include "runloop.h"
#include "atomic.h"
#include "mpool.h"

static void _dealloc(nt_runloop_t *self) {
  assert(self->ev_base != NULL);
  event_base_free(self->ev_base);
  // todo: iterate self->bsv and call nt_runloop_remove_server()
  nt_release(self->srlist);
  nt_free(self, sizeof(nt_runloop_t));
}


nt_runloop_t *nt_runloop_new() {
  NT_OBJ_ALLOC_INIT_self(nt_runloop_t, &_dealloc);
  self->ev_base = event_base_new();
  self->srlist = nt_array_new(1, 0);
  return self;
}


extern struct event_base *current_base; /* defined in libevent/event.c */
nt_runloop_t *nt_shared_runloop = NULL;
nt_runloop_t *nt_runloop_default() {
  if (!nt_shared_runloop) {
    nt_shared_runloop = nt_runloop_new();
    /* free the event_base create by the nt_runloop_new call */
    event_base_free(nt_shared_runloop->ev_base);
    if (!current_base) {
      event_init();
    }
    /* assign the libevent current_base to nt_shared_runloop */
    nt_shared_runloop->ev_base = current_base;
  }
  return nt_shared_runloop;
}


void nt_runloop_add_ev(nt_runloop_t *self, struct event *ev, const struct timeval *timeout) {
  if (timeout && timeout->tv_sec == 0 && timeout->tv_usec == 0)
    timeout = NULL;
  AZ(event_base_set(self->ev_base, ev));
  AZ(event_add(ev, timeout));
}


NT_STATIC_INLINE struct event *_mkacceptev( nt_tcp_server_runloop_t *sr, int fd) {
  struct event *ev;
  if ((ev = (struct event *)nt_malloc(sizeof(struct event))) == NULL)
    return NULL;
  event_set(ev, fd, EV_READ|EV_PERSIST, 
    (void (*)(int, short, void *))(sr->server->on_accept),
    (void *)sr);
  return ev;
}


bool nt_runloop_add_server(nt_runloop_t *self, nt_tcp_server_t *server) {
  nt_tcp_server_runloop_t *sr;
  
  assert((server->fd4 != -1) || (server->fd6 != -1));
  assert(server->on_accept != NULL);
  
  sr = (nt_tcp_server_runloop_t *)nt_malloc(sizeof(nt_tcp_server_runloop_t));
  sr->server = server;
  sr->runloop = self;
  
  if (server->fd4 != -1) {
    assert(server->ev4 == NULL);
    if ((server->ev4 = _mkacceptev(sr, server->fd4)) == NULL) {
      nt_runloop_remove_server(self, server);
      return false;
    }
    nt_runloop_add_ev(self, server->ev4, &server->accept_timeout);
  }
  
  if (server->fd6 != -1) {
    assert(server->ev6 == NULL);
    if ((server->ev6 = _mkacceptev(sr, server->fd6)) == NULL) {
      nt_runloop_remove_server(self, server);
      return false;
    }
    nt_runloop_add_ev(self, server->ev6, &server->accept_timeout);
  }
  
  nt_array_push(self->srlist, sr);
  
  return true;
}


void nt_runloop_add_client(nt_runloop_t *self, nt_tcp_client_t *client) {
  short evflags = 0;
  if (client->bev.readcb)
    evflags |= EV_READ;
  if (client->bev.writecb)
    evflags |= EV_WRITE;
  AZ(bufferevent_enable(&client->bev, evflags));
}


void nt_runloop_remove_client(nt_runloop_t *self, nt_tcp_client_t *client) {
  short evflags = 0;
  if (client->bev.readcb)
    evflags |= EV_READ;
  if (client->bev.writecb)
    evflags |= EV_WRITE;
  AZ(bufferevent_disable(&client->bev, evflags));
}


void nt_runloop_remove_server(nt_runloop_t *self, nt_tcp_server_t *server) {
  if (server->ev4) {
    nt_runloop_remove_ev(server->ev4);
    nt_free(server->ev4, sizeof(struct event));
    server->ev4 = NULL;
  }
  if (server->ev6) {
    nt_runloop_remove_ev(server->ev6);
    nt_free(server->ev6, sizeof(struct event));
    server->ev6 = NULL;
  }
  // todo: remove any bs in self->srlist which include @server
}
