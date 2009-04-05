#ifndef _NT_FREELIST_H_
#define _NT_FREELIST_H_

#include "obj.h"
#include <sys/types.h>

typedef void (nt_freelist_entry_destructor)(void *ptr);


/* Simple freelist of fixed-sized allocations */
typedef struct nt_freelist {
  NT_OBJ_HEAD
	size_t growsize;
	size_t allocated;
	size_t available;
	void **entries;
	nt_freelist_entry_destructor **destructors;
} nt_freelist;

/**
  Create a new nt_freelist.
  growsize is the size of the individual allocations
*/
nt_freelist *nt_freelist_new(size_t growsize);

/**
  Convenience function.
  
  1. if *freelist is NULL, call nt_freelist_new(2)
  2. try nt_freelist_get() and return value if successful
  3. return malloc(allocsize)
*/
void *nt_freelist_get_or_malloc(nt_freelist **freelist, size_t allocsize);

/**
  Get an entry from a freelist.
  Will allocate new entries if necessary
  Returns pointer to allocated memory or NULL on failure.
*/
void *nt_freelist_get(nt_freelist *freelist);

/**
  Returns an entry to the freelist.
  p must be a pointer to an allocation from the freelist.
*/
void nt_freelist_put(nt_freelist *freelist, void *ptr);


#endif
