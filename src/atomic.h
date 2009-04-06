/**
  Atomic memory access and operations.
  
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
NT_STATIC_INLINE void *nt_atomic_fetch_and_setptr(void * volatile *ptr, void *newval) {
  void *oldval;
  do {
    oldval = nt_atomic_readptr(ptr);
  } while(!nt_atomic_bool_compare_and_swapptr(ptr, oldval, newval));
  return oldval;
}

// fetch and set 32-bit integer
NT_STATIC_INLINE int32_t nt_atomic_fetch_and_set32(volatile int32_t *ptr, int32_t newval) {
  int32_t oldval;
  do {
    oldval = nt_atomic_read32(ptr);
  } while(!nt_atomic_bool_compare_and_swap32(ptr, oldval, newval));
  return oldval;
}

/**
  atomic_synchronize_io - ensures orderly load and store operations to
  noncached memory mapped I/O devices.
  
  Executes the eieio instruction on PowerPC processors.
*/
NT_STATIC_INLINE void atomic_synchronize_io(void) {
#if defined(__ppc__)
  __asm__("eieio");
#endif
}

/*#if defined(__APPLE__)
  // Caution: in OS X v <=10.4, only PPC was supported
  //          thus maybe we should check versions here?
  #include "atomic_queue_osx.h"
#else*/
  #include "atomic_queue.h"
//#endif

#endif
