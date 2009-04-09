/**
  Socket address functions.
  
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
#ifndef _NT_SOCKADDR_H_
#define _NT_SOCKADDR_H_

#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct sockaddr_storage nt_sockaddr_t;

/* Read member without knowing the family */
#define NT_SOCKADDR_M(ss, member4, member6) \
  ((ss->ss_family == AF_INET) ? ( \
    ((const struct sockaddr_in *)ss)->member4; \
  ) : ( \
    ((const struct sockaddr_in6 *)ss)->member6; \
  ))

// Return a human readable string representing the hostname.
// This is not thread safe (See nt_sockaddr_hostcpy() for a thread safe version).
const char *nt_sockaddr_host(const nt_sockaddr_t *addr);

// Copy a human readable string representing the hostname.
char *nt_sockaddr_hostcpy(const nt_sockaddr_t *addr, char *buf, size_t bufsize);

// Port of socket address, in host byte order.
NT_STATIC_INLINE uint16_t nt_sockaddr_port(const nt_sockaddr_t *sa) {
  return ntohs(NT_SOCKADDR_M(sa, sin_port, sin6_port));
}

// Return the socket address family (e.g. AF_INET, AF_INET6)
NT_STATIC_INLINE sa_family_t nt_sockaddr_family(const nt_sockaddr_t *sockaddr) {
  /* the family is the first member of struct sockaddr */
  return (sa_family_t)( *((sa_family_t *)(&addr)) );
}

#endif
