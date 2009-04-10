/*
 * A variable-sized, thread-safe and lock-free memory pool.
 *
 * Copyright 1996 by Gray Watson.
 * Copyright 2009 Rasmus Andersson.
 *
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies, and that the name of Gray Watson not be used in advertising
 * or publicity pertaining to distribution of the document or software
 * without specific, written prior permission.
 *
 * Gray Watson makes no representations about the suitability of the
 * software described herein for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The author may be reached via http://256.com/gray/
 *
 * $Id: mpool.h,v 1.4 2006/05/31 20:26:11 gray Exp $
 */

#ifndef __NT_MPOOL_H__
#define __NT_MPOOL_H__

#include <sys/types.h>

/*
 * Choose a best fit algorithm not first fit.  This takes more CPU
 * time but will result in a tighter heap.
 */
#define NT_MPOOL_FLAG_BEST_FIT    (1<<0)

/*
 * By default the library adds 2 bytes onto all allocations to insert
 * a magic number that it can look for to determine how large a freed
 * memory chunk is.  This flag indicates that few if any frees are
 * going to be performed on the pool and to not waste memory on these
 * bytes.
 */
#define NT_MPOOL_FLAG_NO_FREE    (1<<1)

/*
 * This enables very heavy packing at the possible expense of CPU.
 * This affects a number of parts of the library.
 *
 * By default the 1st page of memory is reserved for the main mpool
 * structure.  This flag will cause the rest of the 1st block to be
 * available for use as user memory.
 *
 * By default the library looks through the memory when freed looking
 * for a magic value.  There is an internal max size that it will look
 * and then it will give up.  This flag forces it to look until it
 * finds it.
 */
#define NT_MPOOL_FLAG_HEAVY_PACKING  (1<<2)

/*
 * Use sbrk not mmap to allocate pages.  This is not recommended for
 * normal use.
 */
#define NT_MPOOL_FLAG_USE_SBRK    (1<<3)

/*
 * Mpool error codes
 */
#define NT_MPOOL_ERROR_NONE  1  /* no error */
#define NT_MPOOL_ERROR_ARG_NULL  2  /* function argument is null */
#define NT_MPOOL_ERROR_ARG_INVALID  3  /* function argument is invalid */
#define NT_MPOOL_ERROR_PNT    4  /* invalid mpool pointer */
#define NT_MPOOL_ERROR_POOL_OVER  5  /* mpool structure was overwritten */
#define NT_MPOOL_ERROR_PAGE_SIZE  6  /* could not get system page-size */
#define NT_MPOOL_ERROR_OPEN_ZERO  7  /* could not open /dev/zero */
#define NT_MPOOL_ERROR_NO_MEM  8  /* no memory available */
#define NT_MPOOL_ERROR_MMAP  9  /* problems with mmap */
#define NT_MPOOL_ERROR_SIZE  10  /* error processing requested size */
#define NT_MPOOL_ERROR_TOO_BIG  11  /* allocation exceeded max size */
#define NT_MPOOL_ERROR_MEM    12  /* invalid memory address */
#define NT_MPOOL_ERROR_MEM_OVER  13  /* memory lower bounds overwritten */
#define NT_MPOOL_ERROR_NOT_FOUND  14  /* memory block not found in pool */
#define NT_MPOOL_ERROR_IS_FREE  15  /* memory block already free */
#define NT_MPOOL_ERROR_BLOCK_STAT  16  /* invalid internal block status */
#define NT_MPOOL_ERROR_FREE_ADDR  17  /* invalid internal free address */
#define NT_MPOOL_ERROR_SBRK_CONTIG  18  /* sbrk did not return contiguous mem*/
#define NT_MPOOL_ERROR_NO_PAGES  19  /* ran out of pages in pool */
#define NT_MPOOL_ERROR_ALLOC  20  /* calloc,malloc,free,realloc failed */
#define NT_MPOOL_ERROR_PNT_OVER  21  /* pointer structure was overwritten */

/*
 * Mpool function IDs for the nt_mpool_log_func callback function.
 */
#define NT_MPOOL_FUNC_CLOSE  1  /* nt_mpool_close function called */
#define NT_MPOOL_FUNC_DRAIN  2  /* nt_mpool_drain function called */
#define NT_MPOOL_FUNC_ALLOC  3  /* nt_mpool_alloc function called */
#define NT_MPOOL_FUNC_CALLOC  4  /* nt_mpool_calloc function called */
#define NT_MPOOL_FUNC_FREE    5  /* nt_mpool_free function called */
#define NT_MPOOL_FUNC_RESIZE  6  /* nt_mpool_resize function called */

/*
 * void nt_mpool_log_func_t
 *
 * DESCRIPTION:
 *
 * Mpool transaction log function.
 *
 * RETURNS:
 *
 * None.
 *
 * ARGUMENT:
 *
 * mp_p -> Associated mpool address.
 *
 * func_id -> Integer function ID which identifies which mpool
 * function is being called.
 *
 * byte_size -> Optionally specified byte size.
 *
 * ele_n -> Optionally specified element number.  For nt_mpool_calloc
 * only.
 *
 * new_addr -> Optionally specified new address.  For nt_mpool_alloc,
 * nt_mpool_calloc, and nt_mpool_resize only.
 *
 * old_addr -> Optionally specified old address.  For nt_mpool_resize and
 * nt_mpool_free only.
 *
 * old_byte_size -> Optionally specified old byte size.  For
 * nt_mpool_resize only.
 */
typedef void  (*nt_mpool_log_func_t)(const void *mp_p,
            const int func_id,
            size_t byte_size,
            size_t ele_n,
            const void *old_addr, const void *new_addr,
            size_t old_byte_size);

#ifdef NT_MPOOL_MAIN
#include "mpool_private.h"
#else
/* generic mpool type */
typedef  void  nt_mpool_t;
#endif



/*
 * nt_mpool_t *nt_mpool_open
 *
 * DESCRIPTION:
 *
 * Open/allocate a new memory pool.
 *
 * RETURNS:
 *
 * Success - Pool pointer which must be passed to nt_mpool_close to
 * deallocate.
 *
 * Failure - NULL
 *
 * ARGUMENTS:
 *
 * flags -> Flags to set attributes of the memory pool.  See the top
 * of mpool.h.
 *
 * page_size -> Set the internal memory page-size.  This must be a
 * multiple of the getpagesize() value.  Set to 0 for the default.
 *
 * start_addr -> Starting address to try and allocate memory pools.
 * This is ignored if the NT_MPOOL_FLAG_USE_SBRK is enabled.
 *
 * error_p <- Pointer to integer which, if not NULL, will be set with
 * a mpool error code.
 */
extern
nt_mpool_t  *nt_mpool_open(const unsigned int flags, const unsigned int page_size,
        void *start_addr, int *error_p);

/*
 * int nt_mpool_close
 *
 * DESCRIPTION:
 *
 * Close/free a memory allocation pool previously opened with
 * nt_mpool_open.
 *
 * RETURNS:
 *
 * Success - NT_MPOOL_ERROR_NONE
 *
 * Failure - Mpool error code
 *
 * ARGUMENTS:
 *
 * mp_p <-> Pointer to our memory pool.
 */
extern
int  nt_mpool_close(nt_mpool_t *mp_p);

/*
 * int nt_mpool_drain
 *
 * DESCRIPTION:
 *
 * Wipe an opened memory pool clean so we can start again.
 *
 * RETURNS:
 *
 * Success - NT_MPOOL_ERROR_NONE
 *
 * Failure - Mpool error code
 *
 * ARGUMENTS:
 *
 * mp_p <-> Pointer to our memory pool.
 */
extern
int  nt_mpool_drain(nt_mpool_t *mp_p);

/*
 * void *nt_mpool_alloc
 *
 * DESCRIPTION:
 *
 * Allocate space for bytes inside of an already open memory pool.
 *
 * RETURNS:
 *
 * Success - Pointer to the address to use.
 *
 * Failure - NULL
 *
 * ARGUMENTS:
 *
 * mp_p <-> Pointer to the memory pool.  If NULL then it will do a
 * normal malloc.
 *
 * byte_size -> Number of bytes to allocate in the pool.  Must be >0.
 *
 * error_p <- Pointer to integer which, if not NULL, will be set with
 * a mpool error code.
 */
extern
void  *nt_mpool_alloc(nt_mpool_t *mp_p, size_t size, int *error_p);

/*
 * void *nt_mpool_calloc
 *
 * DESCRIPTION:
 *
 * Allocate space for elements of bytes in the memory pool and zero
 * the space afterwards.
 *
 * RETURNS:
 *
 * Success - Pointer to the address to use.
 *
 * Failure - NULL
 *
 * ARGUMENTS:
 *
 * mp_p <-> Pointer to the memory pool.  If NULL then it will do a
 * normal calloc.
 *
 * count -> Number of elements to allocate.
 *
 * size -> Number of bytes per element being allocated.
 *
 * error_p <- Pointer to integer which, if not NULL, will be set with
 * a mpool error code.
 */
extern
void  *nt_mpool_calloc(nt_mpool_t *mp_p, size_t count, size_t size, int *error_p);

/*
 * int nt_mpool_free
 *
 * DESCRIPTION:
 *
 * Free an address from a memory pool.
 *
 * RETURNS:
 *
 * Success - NT_MPOOL_ERROR_NONE
 *
 * Failure - Mpool error code
 *
 * ARGUMENTS:
 *
 * mp_p <-> Pointer to the memory pool.  If NULL then it will do a
 * normal free.
 *
 * addr <-> Address to free.
 *
 * size -> Size of the address being freed.
 */
extern
int  nt_mpool_free(nt_mpool_t *mp_p, void *addr, size_t size);

/*
 * void *nt_mpool_resize
 *
 * DESCRIPTION:
 *
 * Reallocate an address in a mmeory pool to a new size.  This is
 * different from realloc in that it needs the old address' size.  If
 * you don't have it then you need to allocate new space, copy the
 * data, and free the old pointer yourself.
 *
 * RETURNS:
 *
 * Success - Pointer to the address to use.
 *
 * Failure - NULL
 *
 * ARGUMENTS:
 *
 * mp_p <-> Pointer to the memory pool.  If NULL then it will do a
 * normal realloc.
 *
 * old_addr -> Previously allocated address.
 *
 * old_byte_size -> Size of the old address.  Must be known, cannot be
 * 0.
 *
 * new_byte_size -> New size of the allocation.
 *
 * error_p <- Pointer to integer which, if not NULL, will be set with
 * a mpool error code.
 */
extern
void  *nt_mpool_resize(nt_mpool_t *mp_p, void *old_addr,
          size_t old_byte_size,
          size_t new_byte_size,
          int *error_p);

/*
 * int nt_mpool_stats
 *
 * DESCRIPTION:
 *
 * Return stats from the memory pool.
 *
 * RETURNS:
 *
 * Success - NT_MPOOL_ERROR_NONE
 *
 * Failure - Mpool error code
 *
 * ARGUMENTS:
 *
 * mp_p -> Pointer to the memory pool.
 *
 * page_size_p <- Pointer to an unsigned integer which, if not NULL,
 * will be set to the page-size of the pool.
 *
 * num_alloced_p <- Pointer to an size_t which, if not NULL,
 * will be set to the number of pointers currently allocated in pool.
 *
 * user_alloced_p <- Pointer to an size_t which, if not NULL,
 * will be set to the number of user bytes allocated in this pool.
 *
 * max_alloced_p <- Pointer to an size_t which, if not NULL,
 * will be set to the maximum number of user bytes that have been
 * allocated in this pool.
 *
 * tot_alloced_p <- Pointer to an size_t which, if not NULL,
 * will be set to the total amount of space (including administrative
 * overhead) used by the pool.
 */
extern
int  nt_mpool_stats(const nt_mpool_t *mp_p, unsigned int *page_size_p,
        size_t *num_alloced_p,
        size_t *user_alloced_p,
        size_t *max_alloced_p,
        size_t *tot_alloced_p);


/*
 * int nt_mpool_set_log_func
 *
 * DESCRIPTION:
 *
 * Set a logging callback function to be called whenever there was a
 * memory transaction.  See nt_mpool_log_func_t.
 *
 * RETURNS:
 *
 * Success - NT_MPOOL_ERROR_NONE
 *
 * Failure - Mpool error code
 *
 * ARGUMENTS:
 *
 * mp_p <-> Pointer to the memory pool.
 *
 * log_func -> Log function (defined in mpool.h) which will be called
 * with each mpool transaction.
 */
extern
int nt_mpool_set_log_func(nt_mpool_t *mp_p, nt_mpool_log_func_t log_func);

/*
 * int nt_mpool_set_max_pages
 *
 * DESCRIPTION:
 *
 * Set the maximum number of pages that the library will use.  Once it
 * hits the limit it will return NT_MPOOL_ERROR_NO_PAGES.
 *
 * NOTE: if the NT_MPOOL_FLAG_HEAVY_PACKING is set then this max-pages
 * value will include the page with the mpool header structure in it.
 * If the flag is _not_ set then the max-pages will not include this
 * first page.
 *
 * RETURNS:
 *
 * Success - NT_MPOOL_ERROR_NONE
 *
 * Failure - Mpool error code
 *
 * ARGUMENTS:
 *
 * mp_p <-> Pointer to the memory pool.
 *
 * max_pages -> Maximum number of pages used by the library.
 */
extern
int  nt_mpool_set_max_pages(nt_mpool_t *mp_p, const unsigned int max_pages);

/*
 * const char *nt_mpool_strerror
 *
 * DESCRIPTION:
 *
 * Return the corresponding string for the error number.
 *
 * RETURNS:
 *
 * Success - String equivalient of the error.
 *
 * Failure - String "invalid error code"
 *
 * ARGUMENTS:
 *
 * error -> Error number that we are converting.
 */
extern
const char  *nt_mpool_strerror(const int error);

/* ----------------------------------------------------------- */

/**
  Global shared memory pool used by nt_malloc and friends.
  
  You are responsible for nt_mpool_open a new pool and setting this pointer to
  that pool.
  
  - When a pool has been set, calls to nt_malloc and friends will act on that
    pool.
  
  - If no pool has been set, calls to nt_malloc and friends will have the same
    effect as regular malloc() (and friends) calls.
*/
extern nt_mpool_t *nt_mpool_shared;

/**
  Error code set by failed calls to nt_malloc and friends.
  
  Note: This variable stores NT_MPOOL_ERROR_* codes, not regular errno codes.
*/
extern int nt_mpool_shared_errno;

/**
  Allocate memory from the globally shared memory pool.
  
  If the pool have not been initialized, this implies a normal malloc().
  
  @param size number of bytes to allocate.
*/
NT_STATIC_INLINE void *nt_malloc(size_t size) {
  return nt_mpool_alloc(nt_mpool_shared, size, &nt_mpool_shared_errno);
}

/**
  Allocate and zero out memory for @n number of @sz sized blocks.
  
  If the pool have not been initialized, this implies a normal calloc().
  
  @param n number of elements to allocate.
  @param sz number of bytes per element being allocated.
*/
NT_STATIC_INLINE void *nt_calloc(size_t count, size_t size) {
  return nt_mpool_calloc(nt_mpool_shared, count, size, &nt_mpool_shared_errno);
}

/**
  Resize memory for @oldptr.
  
  @param n number of elements to allocate.
  @param sz number of bytes per element being allocated.
*/
NT_STATIC_INLINE void *nt_realloc(void *oldptr, size_t oldsz, size_t newsz) {
  return nt_mpool_resize(nt_mpool_shared, oldptr, oldsz, newsz,
                         &nt_mpool_shared_errno);
}

/**
  Free memory.
  
  If the pool have not been initialized, this implies a normal free().
  
  @param ptr    pointer to block of memory which is to be freed.
  @param size   size of the address being freed.
*/
NT_STATIC_INLINE void nt_free(void *ptr, size_t size) {
  nt_mpool_shared_errno = nt_mpool_free(nt_mpool_shared, ptr, size);
}

#endif /* ! __NT_MPOOL_H__ */
