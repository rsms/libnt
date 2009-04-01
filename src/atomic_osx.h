/**
 * Atomic memory access using OSAtomic functions in OS X >=10.4
 */
#ifndef _NT_ATOMIC_OSX_H_
#define _NT_ATOMIC_OSX_H_

#include <libkern/OSAtomic.h>

/* ---- ptr ---- */

// void *nt_atomic_compare_and_swapptr(void * volatile *ptr, void *oldval, void *newval)
#define nt_atomic_compare_and_swapptr(ptr, oldval, newval) \
  (OSAtomicCompareAndSwapPtrBarrier(oldval, newval, ptr) ? (oldval) : NULL)

#define nt_atomic_bool_compare_and_swapptr(ptr, oldval, newval) \
  OSAtomicCompareAndSwapPtrBarrier(oldval, newval, ptr)


#if __LP64__
  #define nt_atomic_readptr(ptr) \
    (void *)OSAtomicAdd64Barrier((int64_t)0LL, (volatile int64_t *)(ptr))
#else
  #define nt_atomic_readptr(ptr) \
    (void *)OSAtomicAdd32Barrier(0, (volatile int32_t *)(ptr))
#endif


#define nt_atomic_setptr(ptr, newval) \
  while(!nt_atomic_bool_compare_and_swapptr(ptr, nt_atomic_readptr(ptr), newval))


/* ---- 32 ---- */

#define nt_atomic_compare_and_swap32(ptr, oldval, newval) \
  (OSAtomicCompareAndSwap32Barrier(oldval, newval, ptr) ? (oldval) : NULL)

#define nt_atomic_bool_compare_and_swap32(ptr, oldval, newval) \
  OSAtomicCompareAndSwap32Barrier(oldval, newval, ptr)

#define nt_atomic_read32(ptr) OSAtomicAdd32Barrier(0, ptr)

#define nt_atomic_set32(ptr, newval) \
  while(!nt_atomic_bool_compare_and_swap32(ptr, nt_atomic_read32(ptr), newval))

#define nt_atomic_add32(ptr, n) OSAtomicAdd32Barrier(n, ptr)

#define nt_atomic_sub32(ptr, n) OSAtomicAdd32Barrier(-(n), ptr)

#define nt_atomic_add_and_fetch32(ptr, n) nt_atomic_add32(ptr, n)

#define nt_atomic_sub_and_fetch32(ptr, n) nt_atomic_sub32(ptr, n)

inline static int32_t nt_atomic_fetch_and_add32(volatile int32_t *ptr, int32_t n) {
  int32_t oldval;
  do {
    oldval = nt_atomic_read32(ptr);
  } while(!nt_atomic_bool_compare_and_swap32(ptr, oldval, oldval+n));
  return oldval;
}

#define nt_atomic_fetch_and_sub32(ptr, n) nt_atomic_fetch_and_add32(ptr, -(n))

#endif
