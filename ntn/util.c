#include "util.h"
#include <fcntl.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char *ntn_util_sockaddr_hostcpy(struct sockaddr const *sa, char *buf, size_t bufsize) {
  const void *ia;
  if (sa->sa_family == AF_INET) {
    struct sockaddr_in const *sin;
    sin = (struct sockaddr_in const *)sa;
    ia = &sin->sin_addr;
  }
  else {
    struct sockaddr_in6 const *sin;
    sin = (struct sockaddr_in6 const *)sa;
    ia = &sin->sin6_addr;
  }
  
  if (inet_ntop(sa->sa_family, ia, buf, bufsize-1) == NULL) {
    perror("inet_ntop");
    return NULL;
  }
  return buf;
}

uint16_t ntn_util_sockaddr_port(struct sockaddr const *sa) {
  if (sa->sa_family == AF_INET) {
    struct sockaddr_in const *sin;
    sin = (struct sockaddr_in const *)sa;
    return ntohs(sin->sin_port);
  }
  else {
    struct sockaddr_in6 const *sin;
    sin = (struct sockaddr_in6 const *)sa;
    return ntohs(sin->sin6_port);
  }
}

const char *ntn_util_ev_strbackend(unsigned int backend) {
  switch (backend) {
    case EVBACKEND_SELECT:
      return "select";
    case EVBACKEND_POLL:
      return "poll";
    case EVBACKEND_EPOLL:
      return "epoll";
    case EVBACKEND_KQUEUE:
      return "kqueue";
    case EVBACKEND_DEVPOLL:
      return "devpoll";
    case EVBACKEND_PORT:
      return "port";
  }
  return "?";
}

// Set non-blocking flag for fd
int ntn_util_fd_setnonblock(int fd) {
  int flags = fcntl(fd, F_GETFL);
  if (flags < 0)
    return flags;
  flags |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flags) < 0) 
    return -1;
  return 0;
}

static void _timer_cb_unloopall(struct ev_loop *loop, ev_timer *w, int revents) {
  ev_unloop(loop, EVUNLOOP_ALL);
}

void ntn_util_timer_unloopall(struct ev_loop *loop, double secs) {
  static ev_timer timeout_ev;
  NT_LOOP_ASSURE(loop);
  ev_timer_init(&timeout_ev, _timer_cb_unloopall, secs, 0.0);
  ev_timer_start(loop, &timeout_ev);
}

static void _sig_cb_unloopall(struct ev_loop *loop, struct ev_signal *w, int revents) {
  ev_unloop(loop, EVUNLOOP_ALL);
}

void ntn_util_sig_register( struct ev_loop *loop, ev_signal *w, int signum,
                           void (*cb)(struct ev_loop *, struct ev_signal *, int))
{
  NT_LOOP_ASSURE(loop);
  if (cb == NULL)
    cb = _sig_cb_unloopall;
  ev_signal_init(w, cb, signum);
  ev_signal_start(loop, w);
}