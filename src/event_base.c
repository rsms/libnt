#include "event_base.h"

static void _dealloc(nt_event_base *self) {
  if (self->ev_base)
    event_base_free(self->ev_base);
  free(self);
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
  if ((self = (nt_event_base *)malloc(sizeof(nt_event_base))) == NULL)
    return NULL;
  nt_obj_init((nt_obj *)self, (nt_obj_destructor *)_dealloc);
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
    event_del(ev);
    warnx("nt_event_base_add_socket: event_add() failed");
    return false;
  }
  
  return true;
}


/* helper used by nt_event_base_add_server */
static inline struct event *_mk_add_accept_ev(nt_event_base *base,
                                              nt_tcp_server *server,
                                              nt_tcp_socket *sock,
                                              const struct timeval *timeout )
{
  struct event *ev;
  bool success;
  
  ev = (struct event *)malloc(sizeof(struct event));
  if (!ev) {
    /* errno == ENOMEM */
    return NULL;
  }
  success = nt_event_base_add_socket( base, sock, ev, EV_READ|EV_PERSIST, timeout,
                                      (void (*)(int, short, void *))server->on_accept,
                                      (void *)server );
  if (!success) {
    free(ev);
    ev = NULL;
  }
  return ev;
}


bool nt_event_base_add_server(nt_event_base *base, nt_tcp_server *server, const struct timeval *timeout) 
{
  struct event *ev;
  size_t num_sockets = (server->socket4 ? 1 : 0) + (server->socket6 ? 1 : 0);
  
  if (num_sockets == 0) {
    warnx("nt_tcp_server_register: server is not bound (no sockets)");
    return false;
  }
  
  // Check if we have space for more accept events
  if ( (server->n_accept_evs + num_sockets) > NT_TCP_SERVER_MAX_ACCEPT_EVS ) {
    warnx("nt_tcp_server_register: too many accept events");
    return false;
  }
  
  // Register IPv4 socket
  if (server->socket4) {
    if ((ev = _mk_add_accept_ev(base, server, server->socket4, timeout)) == NULL)
      return false;
    server->v_accept_evs[server->n_accept_evs++] = ev;
  }
  
  // Register IPv4 socket
  if (server->socket6) {
    if ((ev = _mk_add_accept_ev(base, server, server->socket6, timeout)) == NULL)
      return false;
    server->v_accept_evs[server->n_accept_evs++] = ev;
  }
  
  return true;
}
