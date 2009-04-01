#ifndef _NT_ATOMIC_H_
#define _NT_ATOMIC_H_

#if (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 1)
  #include "atomic_gcc.h"
#elif (__APPLE__)
  #include "atomic_osx.h"
#else
  #error Unsupported compiler/platform
#endif


// increment/decrement convenience macros

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

#endif
