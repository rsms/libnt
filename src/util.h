#ifndef _NT_UTIL_H_
#define _NT_UTIL_H_

#include <netinet/in.h>
#include <stdint.h>

// Write human-readable network address sa to buf
char *nt_util_sockaddr_hostcpy(struct sockaddr const *sa, char *buf, size_t bufsize);

// Return the port of address sa in host byte order (a normal integer)
uint16_t nt_util_sockaddr_port(struct sockaddr const *sa);

// Set non-blocking flag for fd
int nt_util_fd_setnonblock(int fd);

#endif
