#ifndef NTN_UTIL_H
#define NTN_UTIL_H

#include <ev.h>
#include <netinet/in.h>
#include <stdint.h>

// Assure loop is set, possibly setting it to ev_default_loop()
#define NT_LOOP_ASSURE(loop) if(loop == NULL) { (loop) = ev_default_loop(0); }

// Write human-readable network address sa to buf
char *ntn_util_sockaddr_hostcpy(struct sockaddr const *sa, char *buf, size_t bufsize);

// Return the port of address sa in host byte order (a normal integer)
uint16_t ntn_util_sockaddr_port(struct sockaddr const *sa);

// Return a string representing backend code returned from ev_backend()
const char *ntn_util_ev_strbackend(unsigned int backend);

// Set non-blocking flag for fd
int ntn_util_fd_setnonblock(int fd);

// Unloop all after secs seconds
void ntn_util_timer_unloopall(struct ev_loop *loop, double secs);

// Register a signal handler
void ntn_util_sig_register(ev_signal *w, int signum, void (*cb)(struct ev_loop *, ev_signal *, int));


#endif
