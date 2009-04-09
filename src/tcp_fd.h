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
#ifndef _NT_TCP_FD_H_
#define _NT_TCP_FD_H_

#include "sockaddr.h"
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

/**
  Bind socket @fd to address @sa.
  
  @param fd socket.
  @param sa address.
  @param ipv6_only only bind to IPv6 sockets.
  @returns boolean success
**/
bool nt_tcp_fd_bind(int fd, const nt_sockaddr_t *sa, bool ipv6_only);

/**
  Enable listening.
  
  @param fd socket.
  @param backlog listen queue size.
  @returns boolean success
**/
NT_STATIC_INLINE void nt_tcp_fd_listen(int fd, int backlog) {
  AZ(listen(fd, backlog));
}

/**
  Accept a connection from @fd and store the client address in @sa.
  
  @param fd server socket on which the accept event have or will occur.
  @param sa a value-return parameter which is the client address.
  @returns open client socket on success or -1 on failure.
**/
int nt_tcp_fd_accept(int fd, nt_sockaddr_t *sa);

/**
  Check if the socket @fd is open.
  
  @param fd socket.
  @returns true if the file descriptor seems to be open, otherwise false.
**/
NT_STATIC_INLINE bool nt_tcp_fd_isopen(int fd) {
  return (fd != -1);
}

NT_STATIC_INLINE void nt_tcp_fd_setblocking(int fd, bool blocking) {
  blocking = !blocking;
	AZ(ioctl(fd, FIONBIO, (int *)&blocking));
}

/**
  Get an integer socket option.
  
  @param fd socket.
  @param level on which level to read the option from (i.e. IPPROTO_TCP, etc).
  @param option the option to retrieve.
  @param value value-return parameter
**/
NT_STATIC_INLINE void nt_tcp_fd_getiopt(int fd, int level, int option, int *value) {
  socklen_t vsz = 0;
  AZ(getsockopt(fd, level, option, (void *restrict)&value, &vsz));
}

/**
  Set an integer socket option.
  
  @param fd socket.
  @param level on which level to set the option for (i.e. IPPROTO_TCP, etc).
  @param option the option to set.
  @param value the value
**/
NT_STATIC_INLINE void nt_tcp_fd_setiopt(int fd, int level, int option, int value) {
  AZ(setsockopt(fd, level, option, (const void *)&value, (socklen_t)sizeof(int)));
}

/**
  Close a socket.
  
  Sets the value of @fd to -1 after closing it.
  
  @param fd pointer to socket.
**/
NT_STATIC_INLINE void nt_tcp_fd_close(int *fd) {
  if (close(*fd) != 0)
    errno = 0;
  *fd = -1;
}

#endif
