#ifndef _NT_EVENT_BASE_H_
#define _NT_EVENT_BASE_H_

#include "obj.h"
#include "tcp_socket.h"
#include "tcp_server.h"
#include <event.h>

typedef struct nt_event_base {
  NT_OBJ_HEAD
  struct event_base *ev_base;
} nt_event_base;

typedef struct nt_event_base_server {
  nt_event_base *base;
  nt_tcp_server *server;
} nt_event_base_server;

/**
  Create a new nt_event_base
*/
nt_event_base *nt_event_base_new();

/**
  The libevent global/shared event_base
*/
nt_event_base *nt_event_base_default();

/**
  Handle events.
  
  @param flags  any combination of EVLOOP_ONCE | EVLOOP_NONBLOCK (or 0 to loop 
                until no events exist)
  @return 0 if successful, -1 if an error occurred, or 1 if no events were registered.
*/
inline static int nt_event_base_loop(nt_event_base *self, int flags) {
  return event_base_loop(self->ev_base, flags);
}

/**
  Exit the event loop after the specified time.
  
  The next event_base_loop() iteration after the given timer expires will
  complete normally (handling all queued events) then exit without
  blocking for events again.
  
  Subsequent invocations of event_base_loop() will proceed normally.
  
  @param timeout  the amount of time after which the loop should terminate or
                  NULL to wait forever.
  @return 0 if successful, or -1 if an error occurred
 */
inline static int nt_event_base_exit(nt_event_base *self, struct timeval *timeout) {
  return event_base_loopexit(self->ev_base, timeout);
}

/**
  Abort the active loop immediately.

  nt_event_base_abort() will abort the loop after the next event is completed;
  event_base_loopbreak() is typically invoked from this event's callback.
  This behavior is analogous to the "break;" statement.

  Subsequent invocations of nt_event_base_loop() will proceed normally.

  @return 0 if successful, or -1 if an error occurred
 */
inline static int nt_event_base_abort(nt_event_base *self) {
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
bool nt_event_base_add_socket(nt_event_base *self,
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
bool nt_event_base_add_server(nt_event_base *self, nt_tcp_server *server, 
                              const struct timeval *timeout);

#endif
