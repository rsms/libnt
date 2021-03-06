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
#include "sockutil.h"
#include "util.h"
#include "mpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/un.h>


int nt_sockutil_socket(int type, int family) {
  int fd;
  if (family == AF_UNIX) {
    if ((fd = socket(family, type, 0)) == -1)
      nt_warn("nt_sockutil_socket UNIX");
  }
  else if (type == SOCK_DGRAM) {
    if ((fd = socket(family, type, IPPROTO_UDP)) == -1)
      nt_warn("nt_sockutil_socket UDP");
  }
  else {
    if ((fd = socket(family, type, IPPROTO_TCP)) == -1)
      nt_warn("nt_sockutil_socket TCP");
  }
  return fd;
}


int nt_sockutil_bind(int fd, const nt_sockaddr_t *sa) {
  int rc;
  socklen_t salen;
  
  // Reuse address
  nt_sockutil_setiopt(fd, SOL_SOCKET, SO_REUSEADDR, 1);
  
  // Reuse port
  #ifdef SO_REUSEPORT
  nt_sockutil_setiopt(fd, SOL_SOCKET, SO_REUSEPORT, 1);
  #endif
  
  // APPLE: Allow reuse of port/socket by different userids
  #ifdef SO_REUSESHAREUID
  nt_sockutil_setiopt(fd, SOL_SOCKET, SO_REUSESHAREUID, 1);
  #endif
  
  // Set non-blocking
  nt_sockutil_setblocking(fd, false);
  
  /* Don't delay send to coalesce packets (disable the Nagle algorithm) */
  /*if ( !blocking && !nt_tcp_socket_setiopt(self, IPPROTO_TCP, TCP_NODELAY, 1)) {
    perror("setsockopt IPPROTO_TCP TCP_NODELAY 1 in nt_tcp_socket_bind()");
    nt_tcp_socket_close(self);
    return false;
  }*/
  
  if (sa->ss_family == AF_UNIX)
    salen = sizeof(struct sockaddr_un);
  if (sa->ss_family == AF_INET6)
    salen = sizeof(struct sockaddr_in6);
  else
    salen = sizeof(struct sockaddr_in);
  
  if ((rc = bind(fd, (const struct sockaddr *)sa, salen)) == -1)
    nt_warn("bind");
  return rc;
}
