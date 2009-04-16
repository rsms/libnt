/**
  libevent base object.
  
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
#ifndef _NT_EVENT_BASE_H_
#define _NT_EVENT_BASE_H_

#include "obj.h"
#include "fd_tcp.h"
#include "tcp_server.h"
#include "tcp_client.h"
#include "array.h"
#include <event.h>

typedef struct nt_runloop_t {
  NT_OBJ_HEAD
  struct event_base *ev_base;
  nt_array_t *srlist; /* list of nt_tcp_server_runloop_t */
} nt_runloop_t;

/**
  Create a new nt_runloop_t
**/
nt_runloop_t *nt_runloop_new();

/**
  The libevent global/shared runloop
**/
nt_runloop_t *nt_runloop_default();

/**
  Handle events.
  
  @param flags  any combination of EVLOOP_ONCE | EVLOOP_NONBLOCK (or 0 to loop 
                until no events exist)
  @return 0 if successful, -1 if an error occurred, or 1 if no events were registered.
**/
NT_STATIC_INLINE int nt_runloop_run(nt_runloop_t *self, int flags) {
  return event_base_loop(self->ev_base, flags);
}

/**
  Exit the event loop after the specified time.
  
  The next runloop_loop() iteration after the given timer expires will
  complete normally (handling all queued events) then exit without
  blocking for events again.
  
  Subsequent invocations of runloop_loop() will proceed normally.
  
  @param timeout  the amount of time after which the loop should terminate or
                  NULL to wait forever.
  @return 0 if successful, or -1 if an error occurred
**/
NT_STATIC_INLINE int nt_runloop_exit(nt_runloop_t *self, struct timeval *timeout) {
  return event_base_loopexit(self->ev_base, timeout);
}

/**
  Abort the active loop immediately.

  nt_runloop_abort() will abort the loop after the next event is completed;
  event_base_loopbreak() is typically invoked from this event's callback.
  This behavior is analogous to the "break;" statement.

  Subsequent invocations of nt_runloop_loop() will proceed normally.

  @return 0 if successful, or -1 if an error occurred
**/
NT_STATIC_INLINE int nt_runloop_abort(nt_runloop_t *self) {
  return event_base_loopbreak(self->ev_base);
}

/**
  Add an event to the runloop.
  
  @param ev event
  @param timeout the maximum amount of time to wait for the event, or NULL to
                 wait forever.
**/
void nt_runloop_add_ev( nt_runloop_t *self, struct event *ev, const struct timeval *timeout);

/**
  Remove an event from the runloop.
  
  @param ev event
**/
NT_STATIC_INLINE void nt_runloop_remove_ev(struct event *ev) {
  AZ(event_del(ev));
}

/**
  Add a server to the runloop.
  
  @param server server
**/
bool nt_runloop_add_server( nt_runloop_t *self, nt_tcp_server_t *server);

/**
  Remove a server from the runloop.
  
  @param server server
**/
void nt_runloop_remove_server(nt_runloop_t *self, nt_tcp_server_t *server);

/**
  Add a client to the runloop.
  
  @param client client
**/
void nt_runloop_add_client(nt_runloop_t *self, nt_tcp_client_t *client);

/**
  Remove a client from the runloop.
  
  @param client client
**/
void nt_runloop_remove_client(nt_runloop_t *self, nt_tcp_client_t *client);


#endif
