#ifndef _NT_OBJ_H_
#define _NT_OBJ_H_

#if !defined(NT_OBJ_REFCOUNT_CHECKS) || NT_OBJ_REFCOUNT_CHECKS
  #include <err.h> /* warnx() */
#endif

#include "atomic.h"
#include <stdint.h>

typedef struct nt_obj {
  /* refcount - the actual reference counter */
  volatile int32_t refcount;
  
  /* destructor - pointer to the function that will clean up the object when
   *              the last reference to the object is released. Required.
   */
  void (* volatile destructor)(struct nt_obj *obj);
} nt_obj;

/* Convenience macros with type casting */
#define nt_getref(obj) nt_obj_get((nt_obj *)(obj))
#define nt_putref(obj) nt_obj_put((nt_obj *)(obj))

/* Head definition
 * usage:
 *
 * struct my_obj {
 *   NTREF_HEAD
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
 * ntref_init - initialize object.
 * @obj: object in question.
 */
inline static void nt_obj_init(nt_obj *obj, void (*destructor)(nt_obj *obj)) {
  nt_obj_set_refcount(obj, 1);
  nt_atomic_setptr((void * volatile *)&((obj)->destructor), destructor);
}

/**
 * ntref_get - increment refcount for object.
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

#endif /* _NT_OBJ_H_ */
