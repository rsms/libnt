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

typedef void (nt_obj_destructor)(struct nt_obj *obj);

typedef struct nt_obj {
  /* refcount - the actual reference counter */
  volatile int32_t refcount;
  
  /* destructor - pointer to the function that will clean up the object when
   *              the last reference to the object is released. Required.
   */
  volatile nt_obj_destructor *destructor;
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
 * nt_obj_set_destructor - set destructor.
 * @obj: object in question.
 * @destructor: pointer to a destructor.
 */
#define nt_obj_set_destructor(obj, destructor) \
  nt_atomic_setptr((void * volatile *)&( ((nt_obj *)(obj))->destructor ), \
                   (nt_obj_destructor *)(destructor))

/**
 * nt_obj_init - initialize object.
 * @obj: object in question.
 */
inline static void nt_obj_init(nt_obj *obj, nt_obj_destructor *destructor) {
  nt_obj_set_refcount(obj, 1);
  if (!destructor) {
    destructor = (nt_obj_destructor *)&free;
  }
  nt_obj_set_destructor(obj, destructor);
}

/**
 * nt_obj_get - increment refcount for object.
 * @obj: object.
 */
inline static void nt_obj_get(nt_obj *obj) {
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
inline static int nt_obj_put(nt_obj *obj) {
  if (nt_atomic_dec_and_fetch32(&((obj)->refcount)) == 0) {
#if !defined(NT_OBJ_REFCOUNT_CHECKS) || NT_OBJ_REFCOUNT_CHECKS
    if (obj->destructor == NULL) {
      warnx("nt_obj_put: NULL destructor when trying to deallocate");
      return 0;
    }
#endif
    obj->destructor(obj);
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
inline static nt_obj *nt_obj_swap(nt_obj * volatile *obj, nt_obj *newobj) {
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
