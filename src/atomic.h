/**
 * Atomic memory access and operations
 */
#ifndef _NT_ATOMIC_H_
#define _NT_ATOMIC_H_

#include <stdint.h>

#if (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 1)
  #include "atomic_gcc.h"
#elif (__APPLE__)
  #include "atomic_osx.h"
#else
  #error Unsupported compiler/platform
#endif


// increment by 1 and return the previous value
#define nt_atomic_fetch_and_inc32(ptr) nt_atomic_fetch_and_add32(ptr, 1)

// decrement by 1 and return the previous value
#define nt_atomic_fetch_and_dec32(ptr) nt_atomic_fetch_and_sub32(ptr, 1)

// increment by 1 and return the resulting value
#define nt_atomic_inc_and_fetch32(ptr) nt_atomic_add_and_fetch32(ptr, 1)

// decrement by 1 and return the resulting value
#define nt_atomic_dec_and_fetch32(ptr) nt_atomic_sub_and_fetch32(ptr, 1)

// increment by 1
#define nt_atomic_inc32(ptr) nt_atomic_add_and_fetch32(ptr, 1)

// decrement by 1
#define nt_atomic_dec32(ptr) nt_atomic_sub_and_fetch32(ptr, 1)

// fetch and set pointer
inline static void *nt_atomic_fetch_and_setptr(void * volatile *ptr, void *newval) {
  void *oldval;
  do {
    oldval = nt_atomic_readptr(ptr);
  } while(!nt_atomic_bool_compare_and_swapptr(ptr, oldval, newval));
  return oldval;
}

// fetch and set 32-bit integer
inline static int32_t nt_atomic_fetch_and_set32(volatile int32_t *ptr, int32_t newval) {
  int32_t oldval;
  do {
    oldval = nt_atomic_read32(ptr);
  } while(!nt_atomic_bool_compare_and_swap32(ptr, oldval, newval));
  return oldval;
}

#endif
