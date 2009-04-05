/**
  Lock-free, thread-safe implementation of a free-list.
  
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
#include "freelist.h"
#include <string.h>
#include <signal.h>

#define NT_FREELIST_MAX_ALLOC	0x1000000
#define NT_FREELIST_ALLOC_ALIGN	16
#define NT_FREELIST_INITIAL_ALLOC	16

#ifndef roundup
#define roundup(x, y) ((((x) + (y) - 1)/(y))*(y))
#endif

#ifdef NT_FREELIST_DEBUG
# define FLOGIT(a) logit a
#else
# define FLOGIT(a)
#endif


void _dealloc(nt_freelist *self) {
  // todo
  free(self);
}


nt_freelist *nt_freelist_new(size_t growsize) {
  nt_freelist *self;
  
	if ( !(self = (nt_freelist *)malloc(sizeof(nt_freelist))) )
    return NULL;
  
  nt_obj_init((nt_obj *)self, (nt_obj_destructor *)_dealloc);
	
	self->growsize = roundup(growsize, NT_FREELIST_ALLOC_ALIGN);
  self->allocated = 0U;
  self->available = 0U;
	self->entries = NULL;
  self->destructors = NULL;
	
  return self;
}


void *nt_freelist_get_or_malloc(nt_freelist **freelist, size_t allocsize) {
  void *ptr;
  nt_freelist *fl, *otherfl;
  
  if (nt_atomic_readptr(freelist) == NULL) {
    fl = nt_freelist_new(2);
    otherfl = (nt_freelist *)nt_atomic_fetch_and_setptr((void * volatile *)freelist, (void *)fl);
    if (otherfl) {
      // Another thread was faster -- release our freelist and use the other one.
      nt_release(fl);
      fl = otherfl;
    }
  }
  
  if ((ptr = nt_freelist_get(*freelist)) == NULL) {
    if ((ptr = malloc(allocsize)) == NULL)
      return NULL;
  }
  
  return ptr;
}


static int _grow(nt_freelist *self) {
	size_t i, oldallocated, need;
	void *p, *p2;
  
	/* Sanity check */
	if (self->allocated > NT_FREELIST_MAX_ALLOC)
		return -1;

	oldallocated = self->allocated;
	if (self->allocated == 0)
		self->allocated = NT_FREELIST_INITIAL_ALLOC;
	else
		self->allocated <<= 1;
	if (self->allocated > NT_FREELIST_MAX_ALLOC)
		self->allocated = NT_FREELIST_MAX_ALLOC;
	FLOGIT((LOG_DEBUG, "%s: allocated now %zu", __func__, self->allocated));

	/* Check for integer overflow */
	if (SIZE_MAX / self->allocated < self->growsize ||
	    SIZE_MAX / self->allocated < sizeof(*self->entries)) {
		FLOGIT((LOG_DEBUG, "%s: integer overflow", __func__));
 resize_fail:
		self->allocated = oldallocated;
		return -1;
	}

	/* Allocate freelist - max size of allocated */
	need = self->allocated * sizeof(*self->entries);
	if ((p = realloc(self->entries, need)) == NULL) {
		goto resize_fail;
	}
	if ((p2 = realloc(self->destructors, need)) == NULL) {
		goto resize_fail;
	}
	/* Allocate the entries */
	self->entries = p;
  self->destructors = p2;
	need = (self->allocated - oldallocated) * self->growsize;
	if ((p = malloc(need)) == NULL) {
		goto resize_fail;
	}
	if ((p2 = malloc(need)) == NULL) {
		goto resize_fail;
	}

	/*
	 * XXX store these malloc ranges in a tree or list, so we can
	 * validate them in _get/_put. Check that r_low <= addr < r_high, and
	 * (addr - r_low) % self->growsize == 0
	 */

	self->available = self->allocated - oldallocated;
	for (i = 0; i < self->available; i++) {
		self->entries[i] = (u_char *)p + (i * self->growsize);
		self->destructors[i] = (nt_freelist_entry_destructor *)((u_char *)p2 + (i * self->growsize));
	}
	for (i = self->available; i < self->allocated; i++) {
		self->entries[i] = NULL;
		self->destructors[i] = NULL;
	}

	FLOGIT((LOG_DEBUG, "%s: done, available = %zu", __func__, self->available));
	return 0;
}


void *nt_freelist_get(nt_freelist *self) {
	void *r;

	FLOGIT((LOG_DEBUG, "%s: %s(%p)", __func__, __func__, self));
	FLOGIT((LOG_DEBUG, "%s: available = %zu", __func__, self->available));

	if (self->available == 0) {
		if (_grow(self) == -1)
			return NULL;
	}

	/* Sanity check */
	/*if (self->available == 0 || self->available > NT_FREELIST_MAX_ALLOC ||
	    self->entries[self->available - 1] == NULL) {
		warnx("%s: invalid available", __func__);
		raise(SIGSEGV);
	}*/

	self->available--;
	r = self->entries[self->available];
	self->entries[self->available] = NULL;
	self->destructors[self->available] = NULL; // xxx really need to do this?

	FLOGIT((LOG_DEBUG, "%s: done, available = %zu", __func__, self->available));
	return r;
}

void nt_freelist_put(nt_freelist *self, void *p) {
	FLOGIT((LOG_DEBUG, "%s: %s(%p, %zu)", __func__, __func__, self, p));
	FLOGIT((LOG_DEBUG, "%s: available = %zu", __func__, self->available));
	FLOGIT((LOG_DEBUG, "%s: allocated = %zu", __func__, self->available));

	/* Sanity check */
	if (self->available >= self->allocated) {
		warnx("%s: freelist available >= allocated", __func__);
		raise(SIGSEGV);
	}
	if (self->entries[self->available] != NULL) {
		warnx("%s: entries[%zd] != NULL", __func__, self->available);
		raise(SIGSEGV);
	}
  
	self->entries[self->available] = p;
	self->available++;

	FLOGIT((LOG_DEBUG, "%s: done, available = %zu", __func__, self->available));
}
