/**
  Lock-free, thread-safe and reference-counted object.
  
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
#ifndef _NT_OBJ_H_
#define _NT_OBJ_H_

#include "atomic.h"
#include <stdint.h>
#include <stdlib.h>
#include <err.h> /* warnx() */

struct nt_obj *obj;

typedef void (nt_obj_deallocator)(struct nt_obj *obj);

typedef struct nt_obj {
  /* refcount - the actual reference counter */
  volatile int32_t refcount;
  
  /* deallocator - pointer to the function that will clean up the object when
   *              the last reference to the object is released. Required.
   */
  volatile nt_obj_deallocator *deallocator;
} nt_obj;

/* Convenience macros with type casting */
/* get/put naming */
#define nt_getref(obj)  nt_obj_get((nt_obj *)(obj))
#define nt_putref(obj)  nt_obj_put((nt_obj *)(obj))
/* inc/dec naming */
#define nt_incref(obj)  nt_obj_get((nt_obj *)(obj))
#define nt_decref(obj)  nt_obj_put((nt_obj *)(obj))
/* retain/release naming */
#define nt_retain(obj)  nt_obj_get((nt_obj *)(obj))
#define nt_release(obj) nt_obj_put((nt_obj *)(obj))

/* get/put naming */
#define nt_getref(obj)  nt_obj_get((nt_obj *)(obj))
#define nt_putref(obj)  nt_obj_put((nt_obj *)(obj))
/* inc/dec naming */
#define nt_incref(obj)  nt_obj_get((nt_obj *)(obj))
#define nt_decref(obj)  nt_obj_put((nt_obj *)(obj))
/* retain/release naming */
#define nt_xretain(obj)  ((obj) ? nt_obj_get((nt_obj *)(obj)) : (void)(0))
#define nt_xrelease(obj) ((obj) ? nt_obj_put((nt_obj *)(obj)) : (void)(0))

/* Head definition
 * usage:
 *
 * struct my_obj {
 *   NT_OBJ_HEAD
 *   int my_foo;
 *   char *my_bar;
 * }
 *
 */
#define NT_OBJ_HEAD nt_obj ntobj;


/**
 * nt_obj_set_refcount - set refcount to requested number.
 * @obj: object in question.
 * @count: initial reference counter
 */
#define nt_obj_set_refcount(obj, count) nt_atomic_set32(&((obj)->refcount), count)

/**
 * nt_obj_get_refcount - get refcount number.
 * @obj: object in question.
 */
#define nt_obj_get_refcount(obj) nt_atomic_read32(&((obj)->refcount))

/**
 * nt_obj_set_deallocator - set deallocator.
 * @obj: object in question.
 * @deallocator: pointer to a deallocator.
 */
#define nt_obj_set_deallocator(obj, deallocator) \
  nt_atomic_setptr((void * volatile *)&( ((nt_obj *)(obj))->deallocator ), \
    (nt_obj_deallocator *)(deallocator))

/**
  Initialize an object, setting refcount to 1 and assign a deallocator.
  
  @obj: object in question.
 */
NT_STATIC_INLINE void nt_obj_init(nt_obj *obj, nt_obj_deallocator *deallocator) {
  nt_obj_set_refcount(obj, 1);
  nt_obj_set_deallocator(obj, deallocator);
}

/**
  Faster (but non-thread safe) version of nt_obj_init
  
  @obj: object in question.
 */
#define NT_OBJ_INIT(obj, _deallocator) \
  do { \
    ((nt_obj *)(obj))->refcount = 1; \
    ((nt_obj *)(obj))->deallocator = (volatile nt_obj_deallocator *)_deallocator; \
  } while(0)

/**
  Cenvenience macro for defining, allocating and initializing an object.
  Should be used in your objects constructor.
  
  Example:
  
    typedef struct myobj_t {
      NT_OBJ_HEAD
      int mymember;
    } myobj_t;
    
    static void _dealloc(myobj_t *o) {
      nt_free(o, sizeof(myobj_t));
    }
    
    myobj_t *myobj_new() {
      NT_OBJ_ALLOC_INIT_self(myobj_t, _dealloc);
      self->mymember = 123;
      return self;
    }
*/
#define NT_OBJ_ALLOC_INIT_self(T, deallocator) \
  T *self; \
  do { \
    if ((self = (T *)nt_malloc(sizeof(T))) == NULL) { \
      return NULL; \
    } \
    NT_OBJ_INIT((nt_obj *)self, (nt_obj_deallocator *)(deallocator)); \
  } while(0)


/**
  Helper for defining constructor and destructor.
  
  Example:
  
    typedef struct myobj_t {
      NT_OBJ_HEAD
      char *mymember;
    } myobj_t;
  
    NT_OBJ( myobj_t, myobj_new(const char myarg), {
      // constructor
      strcpy(self->mymember, myarg);
    },{
      // destructor
      free(self->mymember);
    })
**/
#define NT_OBJ(T, constructorproto, initblock, deallocblock) \
  static void _dealloc_ ##T(T *self) { \
    deallocblock \
    nt_free(self, sizeof(T)); \
  } \
  T * constructorproto { \
    NT_OBJ_ALLOC_INIT_self(T, &_dealloc_ ##T); \
    initblock \
    return self; \
  }


/**
 * nt_obj_get - increment refcount for object.
 * @obj: object.
 */
NT_STATIC_INLINE void nt_obj_get(nt_obj *obj) {
#if !defined(NT_OBJ_REFCOUNT_CHECKS) || NT_OBJ_REFCOUNT_CHECKS
  if (nt_atomic_inc_and_fetch32(&obj->refcount) == 1)
    warnx("nt_obj_get: trying to get reference to dead object");
#else
  nt_atomic_inc32(&obj->refcount);
#endif
}

/**
 * nt_obj_put - decrement refcount for object.
 * @ntref: object.
 *
 * Decrement the refcount, and if 0, call release().
 * Return 1 if the object was removed, otherwise return 0.  Beware, if this
 * function returns 0, you still can not count on the ntref from remaining in
 * memory.  Only use the return value if you want to see if the ntref is now
 * gone, not present.
 */
NT_STATIC_INLINE int nt_obj_put(nt_obj *obj) {
  if (nt_atomic_dec_and_fetch32(&((obj)->refcount)) == 0) {
#if !defined(NT_OBJ_REFCOUNT_CHECKS) || NT_OBJ_REFCOUNT_CHECKS
    if (obj->deallocator == NULL) {
      warnx("nt_obj_put: NULL deallocator when trying to deallocate");
      return 0;
    }
#endif
    obj->deallocator(obj);
    return 1;
  }
  return 0;
}

/**
 * nt_obj_swap - replace @obj with @newobj.
 *
 * This is an atomic operation and takes care of decreasing and increasing
 * reference counts.
 *
 * Returns the previous value of @obj
 */
NT_STATIC_INLINE nt_obj *nt_obj_swap(nt_obj * volatile *obj, nt_obj *newobj) {
  nt_obj *oldobj;
  if (*obj != newobj) {
    oldobj = (nt_obj *)nt_atomic_fetch_and_setptr((void * volatile *)obj, (void *)newobj);
    if (*obj != oldobj) {
      if (oldobj) {
        nt_obj_put(oldobj);
      }
      if (newobj) {
        nt_obj_get(newobj);
      }
    }
    return oldobj;
  }
  return *obj;
}


#endif /* _NT_OBJ_H_ */
