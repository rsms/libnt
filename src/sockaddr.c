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
#include "sockaddr.h"
#include <arpa/inet.h>

// like NT_SOCKADDR_M but fetches a pointer
#define NT_SOCKADDR_MP(ss, member4, member6) \
  (const void *)((ss->ss_family == AF_INET) ? ( \
    (const void *)&((const struct sockaddr_in *)ss)->member4 \
  ) : ( \
    (const void *)&((const struct sockaddr_in6 *)ss)->member6 \
  ))


char *nt_sockaddr_hostcpy(const nt_sockaddr_t *sa, char *buf, size_t bufsize) {
  if (inet_ntop(sa->ss_family,
                NT_SOCKADDR_MP(sa, sin_addr, sin6_addr),
                buf, bufsize-1) == NULL)
  {
    nt_warn("inet_ntop");
    return NULL;
  }
  return buf;
}


const char *nt_sockaddr_host(const nt_sockaddr_t *sa) {
  static char *buf[SOCK_MAXADDRLEN+1];
  buf[0] = '\0';
  if(nt_sockaddr_hostcpy(sa, (char *)&buf, 100) == NULL) {
    return NULL;
  }
  return (const char *)buf;
}
