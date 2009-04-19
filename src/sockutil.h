/**
  TCP socket operations (on file descriptors).
  
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
#ifndef _NT_FD_TCP_H_
#define _NT_FD_TCP_H_

#include "fd.h" /* nt_fd_close() etc */
#include "sockaddr.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>

/**
  Creates an endpoint for communication and returns a descriptor.
  
  Create a TCP 4 socket:  nt_sockutil_socket(AF_INET,  SOCK_STREAM)
  Create a TCP 6 socket:  nt_sockutil_socket(AF_INET6, SOCK_STREAM)
  Create a UDP 4 socket:  nt_sockutil_socket(AF_INET,  SOCK_DGRAM)
  Create a UDP 6 socket:  nt_sockutil_socket(AF_INET6, SOCK_DGRAM)
  Create a UNIX socket:   nt_sockutil_socket(AF_UNIX,  SOCK_STREAM)
  
  @param family protocol family (AF_INET, AF_INET6 or AF_UNIX)
  @returns the socket FD or -1 on error.
**/
int nt_sockutil_socket(int type, int family);

/**
  Get an integer socket option.
  
  @param fd socket.
  @param level on which level to read the option (i.e. SOL_SOCKET, etc).
  @param option the option to retrieve.
  @returns the value
**/
NT_STATIC_INLINE int nt_sockutil_getiopt(int fd, int level, int option) {
  int v = 0;
  socklen_t vsz = sizeof(v);
  AZ(getsockopt(fd, level, option, (void *restrict)&v, &vsz));
  return v;
}

/**
  Set an integer socket option.
  
  @param fd socket.
  @param level on which level to set the option for (i.e. IPPROTO_TCP, etc).
  @param option the option to set.
  @param value the value
**/
NT_STATIC_INLINE void nt_sockutil_setiopt(int fd, int level, int option, int value) {
  AZ(setsockopt(fd, level, option, (const void *)&value, (socklen_t)sizeof(int)));
}

/**
  Control if the socket should block on I/O or not.
  
  @param fd socket.
  @param blocking if true, operations on @fd will block, otherwise calls like
                  read() will not block.
**/
NT_STATIC_INLINE void nt_sockutil_setblocking(int fd, bool blocking) {
  blocking = !blocking;
	AZ(ioctl(fd, FIONBIO, (int *)&blocking));
}

/**
  Bind socket @fd to address @sa.
  
  @param fd socket.
  @param sa address.
  @returns see documentation of bind()
**/
int nt_sockutil_bind(int fd, const nt_sockaddr_t *sa);

/**
  Check if socket is listening or not.
  
  @param fd socket.
  @returns true if @fd has been successfully passed through listen(),
           otherwise false.
**/
NT_STATIC_INLINE bool nt_sockutil_islistening(int fd) {
  return nt_sockutil_getiopt(fd, SOL_SOCKET, SO_ACCEPTCONN) ? true : false;
}

/**
  Accept a connection from @fd and store the client address in @sa.
  
  @param fd server socket on which the accept event have or will occur.
  @param sa a value-return parameter which is the client address.
  @returns open client socket on success or -1 on failure.
**/
NT_STATIC_INLINE int nt_sockutil_accept(int fd, nt_sockaddr_t *sa) {
  socklen_t saz = sizeof(nt_sockaddr_t);
  return accept(fd, (struct sockaddr *)sa, &saz);
}

/**
  Shutdown a socket.
  
  @param fd socket
**/
NT_STATIC_INLINE int nt_sockutil_shutdown(int fd) {
  return shutdown(fd, SHUT_RDWR);
}


#endif
