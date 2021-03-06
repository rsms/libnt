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
**/
#include "buffer.h"
#include "mpool.h"
#include <stdarg.h>


#define ADDROF(a, i, size) ((void **)((a)->start + ((size) * (i))))


NT_OBJ(nt_buffer_t, nt_buffer_new(size_t size, size_t growextra),
{/* constructor: */
  if (size == 0)
	  size = growextra;
	if (size == 0)
    size = 1;
  size = NT_ALIGN_M(size); // min size is machine alignment
	if ( (self->start = (byte_t *)nt_malloc(size)) == NULL ) {
    return NULL; // ENOMEM
  }
	self->end = self->start + size;
	self->ptr = self->start;
  self->growextra = growextra;
},
{/* destructor: */
  if (self->start && nt_buffer_size(self)) {
    nt_free(self->start, nt_buffer_size(self));
  }
})


bool nt_buffer_grow(nt_buffer_t *self, size_t length) {
	byte_t *new_start;
	size_t new_size;

	if (length <= nt_buffer_available(self))
	  return true;
	
  if (
    #ifdef NT_BUFFER_GROW_MAX
  	  length > NT_BUFFER_GROW_MAX || 
  	#endif
	  self->start == NULL) {
	  return false;
  }
  
	new_size = NT_ALIGN_M(nt_buffer_occupied(self) + length + self->growextra);
	
	#ifdef NT_BUFFER_GROW_MAX
	if (new_size > NT_BUFFER_GROW_MAX)
	  return false;
	#endif
	
	if ((new_start = nt_realloc(self->start, nt_buffer_size(self), new_size)) == NULL)
    err(1, NULL);
  
	self->ptr = new_start + nt_buffer_occupied(self);
	self->start = new_start;
	self->end = new_start + new_size;
	
	return true;
}


bool nt_buffer_append(nt_buffer_t *self, const byte_t *what, size_t length) {
	if (length > nt_buffer_available(self) && !nt_buffer_grow(self, length))
    return false;
	memcpy(self->ptr, what, length);
	self->ptr += length;
  return true;
}


bool nt_buffer_appendb(nt_buffer_t *self, byte_t what) {
	if (self->ptr >= self->end && !nt_buffer_grow(self, 1))
	  return false;
	*(self->ptr++) = what;
	return true;
}


bool nt_buffer_appendf(nt_buffer_t *self, const char *fmt, ...) {
	va_list args;
	size_t length, size;
	int n;
  
	length = 1;
	size = nt_buffer_available(self);
	
	do {
		if (length > size) {
			if (nt_buffer_grow(self, length))
			  return false;
			size = nt_buffer_available(self);
		}
    
		va_start(args, fmt);
		n = vsnprintf((char *)self->ptr, size, fmt, args);
		va_end(args);
    
		if (n >= 0) {
			if (n < size) {
				self->ptr += n;
				return true;
			}
			length = n + 1;
		}
		else {
			length = size << 1;
		}
	}
	while (length > size);
  
	return false;
}


ssize_t nt_buffer_indexof(nt_buffer_t *self, const byte_t *what, size_t n) {
  ssize_t i;
  size_t len = nt_buffer_length(self);
  // what happens if len > SSIZETMAX?
  for (i = 0; i < len; i++)
    if (memcmp((const char *)(self->start + i), (const char *)what, n) == 0)
      return i;
  return -1;
}


void nt_buffer_del(nt_buffer_t *self, size_t index, size_t count, size_t size) {
  size_t length, i, cpycount;
  void *srcp, *dstp;
  
  length = nt_buffer_length(self) / size;
  cpycount = count;
  assert(index < length);
  
  for (i = index; i < length; i += cpycount) {
    if (i + cpycount > length)
      cpycount = length - i;
    srcp = ADDROF(self, i + cpycount, size);
    dstp = ADDROF(self, i, size);
    AN(memcpy((void *)dstp, (const void *)srcp, size * cpycount));
  }
  
  self->ptr = (byte_t *)ADDROF(self, length - count, size);
  memset((void *)dstp, 0, size * count);
}
