/**
  Atomic memory access using GCC >=4.1 built-ins.
  http://gcc.gnu.org/onlinedocs/gcc-4.2.4/gcc/Atomic-Builtins.html
  
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
#ifndef _NT_ATOMIC_GCC_H_
#define _NT_ATOMIC_GCC_H_

/* ---- ptr ---- */

// compare and swap.
// if the current value of *ptr is oldval, then write newval into *ptr.
// returns the contents of *ptr before the operation.
#define nt_atomic_compare_and_swapptr(ptr, oldval, newval) \
  __sync_val_compare_and_swap(ptr, oldval, newval)

// compare and swap.
// if the current value of *ptr is oldval, then write newval into *ptr.
// returns true if the comparison is successful and newval was written.
#define nt_atomic_bool_compare_and_swapptr(ptr, oldval, newval) \
  __sync_bool_compare_and_swap((intptr_t)ptr, (intptr_t)oldval, (intptr_t *)newval)

// read pointer
#define nt_atomic_readptr(ptr) __sync_fetch_and_add(ptr, 0)

// set pointer
#define nt_atomic_setptr(ptr, newval) \
  while(!nt_atomic_bool_compare_and_swapptr(ptr, nt_atomic_readptr(ptr), newval))

/* ---- 32 ---- */

// compare and swap.
// if the current value of *ptr is oldval, then write newval into *ptr.
// returns the contents of *ptr before the operation.
#define nt_atomic_compare_and_swap32(ptr, oldval, newval) \
  __sync_val_compare_and_swap(ptr, oldval, newval)

// compare and swap.
// if the current value of *ptr is oldval, then write newval into *ptr.
// returns true if the comparison is successful and newval was written.
#define nt_atomic_bool_compare_and_swap32(ptr, oldval, newval) \
  __sync_bool_compare_and_swap(ptr, oldval, newval)

// read value
#define nt_atomic_read32(ptr) __sync_fetch_and_add(ptr, 0)

// set value
#define nt_atomic_set32(ptr, newval) \
  while(!__sync_bool_compare_and_swap(ptr, nt_atomic_read32(ptr), newval))

// add value
#define nt_atomic_add32(ptr, n) __sync_add_and_fetch(ptr, n)

// subtract value
#define nt_atomic_sub32(ptr, n) __sync_sub_and_fetch(ptr, n)

// add value and return the resulting value
#define nt_atomic_add_and_fetch32(ptr, n) __sync_add_and_fetch(ptr, n)

// subtract value and return the resulting value
#define nt_atomic_sub_and_fetch32(ptr, n) __sync_sub_and_fetch(ptr, n)

// add value and return the previous value
#define nt_atomic_fetch_and_add32(ptr, n) __sync_fetch_and_add(ptr, n)

// subtract value and return the previous value
#define nt_atomic_fetch_and_sub32(ptr, n) __sync_fetch_and_sub(ptr, n)


#endif
