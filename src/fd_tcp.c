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
#include "fd_tcp.h"
#include "util.h"
#include "mpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>


int nt_fd_tcp_socket(bool ipv6) {
  return socket(ipv6 ? AF_INET6 : AF_INET, SOCK_STREAM, PF_INET);
}


int nt_fd_tcp_bind(int fd, const nt_sockaddr_t *sa) {
  static const char on = 1;
  struct addrinfo *ptr;
  
  nt_fd_tcp_setiopt(fd, SOL_SOCKET, SO_REUSEADDR, 1);
  #ifdef SO_REUSEPORT
  nt_fd_tcp_setiopt(fd, SOL_SOCKET, SO_REUSEPORT, 1);
  #endif
  nt_fd_tcp_setblocking(fd, false);
  
  /* Don't delay send to coalesce packets (disable the Nagle algorithm) */
  /*if ( !blocking && !nt_tcp_socket_setiopt(self, IPPROTO_TCP, TCP_NODELAY, 1)) {
    perror("setsockopt IPPROTO_TCP TCP_NODELAY 1 in nt_tcp_socket_bind()");
    nt_tcp_socket_close(self);
    return false;
  }*/
  
  return bind(fd, (const struct sockaddr *)sa, sizeof(nt_sockaddr_t));
}


int nt_fd_tcp_accept(int fd, nt_sockaddr_t *sa) {
  socklen_t sockaddr_len = sizeof(nt_sockaddr_t);
  self->fd = accept(server_fd, (struct sockaddr *)sa, &sockaddr_len);
  
  if (self->fd == -1) {
    perror("accept() in nt_tcp_socket_accept()");
    return false;
  }
  
  return true;
}
