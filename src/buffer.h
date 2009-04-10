/**
  Generic in-memory buffer.
  
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
#ifndef _NT_BUFFER_H_
#define _NT_BUFFER_H_

#include "obj.h"

#define NT_ALIGN(value, size) (((value)+(size)-1)&~((size)-1))

#define NT_BUFFER_GROWSIZE 0x8000
#define NT_BUFFER_GROW_MAX 0x1000000

typedef struct nt_buffer_t {
  byte_t *start;    /* start address */
  byte_t *ptr;      /* next free byte */
  byte_t *end;      /* end address */
  size_t growextra; /* grow: realloc(sizeneeded + growextra) */
} nt_buffer_t;

nt_buffer_t *nt_buffer_new(size_t size, size_t growextra);

#define nt_buffer_size(self)      ((self)->end - (self)->start)
#define nt_buffer_occupied(self)  ((self)->ptr - (self)->start)
#define nt_buffer_available(self) ((self)->end - (self)->ptr)
#define nt_buffer_length(self)    nt_buffer_occupied(self)

bool nt_buffer_grow(nt_buffer_t *self, size_t length);

bool nt_buffer_append(nt_buffer_t *self, const byte_t *what, size_t length);
bool nt_buffer_appendb(nt_buffer_t *self, byte_t b);
#define nt_buffer_appendc(self, c) nt_buffer_appendb(self, (byte_t)c)
bool nt_buffer_appendf(nt_buffer_t *self, const char *fmt, ...) NT_ATTR((format(printf, 2, 3)));
#define nt_buffer_appends(buf, what) nt_buffer_append((buf), (const byte_t *)(what), strlen(what))


/* array interface modeled on top of buffer */
typedef nt_buffer_t nt_array_t;

#define TZ sizeof(void*)
#define nt_array_new(capacity, growextra) nt_buffer_new((capacity)*TZ, (growextra)*TZ)
#define nt_array_size(self)       ((self)->end - (self)->start)
#define nt_array_length(self)     ((self)->ptr - (self)->start)
#define nt_array_available(self)  (((self)->end - (self)->ptr) / TZ)

#define nt_array_push(self, v)    nt_buffer_append(self, (byte_t *)((void **)&(v)), TZ)
#define nt_array_get(self, i)     *((void **)((self)->start + (TZ * (i))))
#define nt_array_set(self, i, v) \
  memcpy(((void **)((self)->start + (TZ * (i)))), (void **)&(v), TZ)


#endif
