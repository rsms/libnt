/**
  Generic array modeled on top of buffer.h.
  
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
#ifndef _NT_ARRAY_H_
#define _NT_ARRAY_H_

#include "buffer.h"

typedef nt_buffer_t nt_array_t;

#define nt_array_new(capacity, growextra) \
  nt_buffer_new((capacity)*sizeof(void*), (growextra)*sizeof(void*))

#define nt_array_size(self)       ((self)->end - (self)->start)

#define nt_array_length(self)     ((self)->ptr - (self)->start)

#define nt_array_available(self)  (((self)->end - (self)->ptr) / sizeof(void*))

#define nt_array_push(self, v) \
  nt_buffer_append(self, (byte_t *)((void **)&(v)), sizeof(void*))

#define nt_array_get(self, i) \
  *((void **)((self)->start + (sizeof(void*) * (i))))

#define nt_array_set(self, i, v) \
  memcpy(((void **)((self)->start + (sizeof(void*) * (i)))), (void **)&(v), sizeof(void*))


#endif
