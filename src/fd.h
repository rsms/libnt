/**
  File descriptor utilities.
  
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
#ifndef _NT_FD_H_
#define _NT_FD_H_

/**
  Check if the file @fd is open.
  
  @param fd socket.
  @returns true if the file descriptor seems to be open, otherwise false.
**/
NT_STATIC_INLINE bool nt_fd_isopen(int fd) {
  return (fd != -1);
}

/**
  Close a file.
  
  Sets the value of @fd to -1 after closing it.
  
  @param fd pointer to socket.
**/
NT_STATIC_INLINE void nt_fd_close(int *fd) {
  if (close(*fd) != 0)
    errno = 0;
  *fd = -1;
}

#endif
