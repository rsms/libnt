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
*/
#ifndef _NT_EVENT_BASE_H_
#define _NT_EVENT_BASE_H_

#include "obj.h"
#include "tcp_fd.h"
#include "tcp_server.h"
#include <event.h>

typedef struct nt_runloop {
  NT_OBJ_HEAD
  struct event_base *ev_base;
} nt_runloop;

/**
  Create a new nt_runloop
*/
nt_runloop *nt_runloop_new();

/**
  The libevent global/shared runloop
*/
nt_runloop *nt_runloop_default();

/**
  Handle events.
  
  @param flags  any combination of EVLOOP_ONCE | EVLOOP_NONBLOCK (or 0 to loop 
                until no events exist)
  @return 0 if successful, -1 if an error occurred, or 1 if no events were registered.
*/
NT_STATIC_INLINE int nt_runloop_run(nt_runloop *self, int flags) {
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
 */
NT_STATIC_INLINE int nt_runloop_exit(nt_runloop *self, struct timeval *timeout) {
  return event_base_loopexit(self->ev_base, timeout);
}

/**
  Abort the active loop immediately.

  nt_runloop_abort() will abort the loop after the next event is completed;
  event_base_loopbreak() is typically invoked from this event's callback.
  This behavior is analogous to the "break;" statement.

  Subsequent invocations of nt_runloop_loop() will proceed normally.

  @return 0 if successful, or -1 if an error occurred
 */
NT_STATIC_INLINE int nt_runloop_abort(nt_runloop *self) {
  return event_base_loopbreak(self->ev_base);
}

/**
  Add a socket with an event to this base.
  
  @param  sock  the socket to add
  @param  ev  the event to add
  @param  flags  see documentation of event_set()
  @param  timeout  the maximum amount of time to wait for the event, or NULL
                   to wait forever.
  @param  cb  event callback
  @param  cbarg  argument to pass to @cb
  @return true on success, otherwise false is returned
 */
bool nt_runloop_add_socket( nt_runloop *self,
                            nt_tcp_socket *sock,
                            struct event *ev,
                            int flags,
                            const struct timeval *timeout,
                            void (*cb)(int, short, void *),
                            void *cbarg);

/**
  Register the server on event base.
  
  @param server  server to add
  @param timeout  the maximum amount of time to wait for an accept event, or
                  NULL to wait forever.
*/
bool nt_runloop_add_server( nt_runloop *self, nt_tcp_server *server, 
                            const struct timeval *timeout);

#endif
