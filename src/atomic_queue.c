#include "atomic_queue.h"
#include "machine.h"
#include <stdlib.h>
#include <dlfcn.h>
#include <err.h>
#include <stdbool.h>

#ifdef __LP64__

void nt_atomic_enqueue64_mp(nt_atomic_queue *queue, void *elem, size_t offset);
void *nt_atomic_dequeue64_mp(nt_atomic_queue *queue, size_t offset);
void *nt_atomic_dequeue_ifnexteq64_mp(nt_atomic_queue *queue, size_t offset, void *cmpptr);

void nt_atomic_enqueue64_up(nt_atomic_queue *queue, void *elem, size_t offset);
void *nt_atomic_dequeue64_up(nt_atomic_queue *queue, size_t offset);
void *nt_atomic_dequeue_ifnexteq64_up(nt_atomic_queue *queue, size_t offset, void *cmpptr);

#else // __LP64__ not defined

void nt_atomic_enqueue32_mp(nt_atomic_queue *queue, void *elem, size_t offset);
void *nt_atomic_dequeue32_mp(nt_atomic_queue *queue, size_t offset);
void *nt_atomic_dequeue_ifnexteq32_mp(nt_atomic_queue *queue, size_t offset, void *cmpptr);

void nt_atomic_enqueue32_up(nt_atomic_queue *queue, void *elem, size_t offset);
void *nt_atomic_dequeue32_up(nt_atomic_queue *queue, size_t offset);
void *nt_atomic_dequeue_ifnexteq32_up(nt_atomic_queue *queue, size_t offset, void *cmpptr);


void *nt_atomic_dequeue32_on64_mp(nt_atomic_queue *queue, size_t offset);
void *nt_atomic_dequeue_ifnexteq32_on64_mp(nt_atomic_queue *queue, size_t offset, void *cmpptr);

void *nt_atomic_dequeue32_on64_up(nt_atomic_queue *queue, size_t offset);
void *nt_atomic_dequeue_ifnexteq32_on64_up(nt_atomic_queue *queue, size_t offset, void *cmpptr);

#endif // __LP64__

typedef void  (*_enqueue_func)(nt_atomic_queue *queue, void *elem, size_t offset);
typedef void *(*_dequeue_func)(nt_atomic_queue *queue, size_t offset);
typedef void *(*_enqueue_ifnexteq_func)(nt_atomic_queue *queue, size_t offset, void *cmpptr);

/* ------------------------------------------------------------------------- */

static _enqueue_func _enqueue = NULL;
static _dequeue_func _dequeue = NULL;
static _enqueue_ifnexteq_func _dequeue_ifnexteq = NULL;

/* ------------------------------------------------------------------------- */

#if defined (__GNUC__) && (__GNUC__ >= 4)
NT_ATTR(constructor) static void _init(void)
#else
void nt_atomic_queue_init(void)
#endif
{
  int ncpu = 0, fake64 = 0;
  _enqueue_func enqueue = NULL, os_enqueue = NULL;
  _dequeue_func dequeue = NULL, os_dequeue = NULL;
  _enqueue_ifnexteq_func dequeue_ifnexteq = NULL;
  
  if ((ncpu = nt_machine_ncpu(&fake64)) == -1) {
    warnx("call to nt_machine_cpu in atomic_queue init failed -- defaulting to mp routines");
    ncpu = 2;
  }
  
  /* Assign functions */
  #ifdef __LP64__
    if(ncpu == 1) {
      enqueue = nt_atomic_enqueue64_up;
      dequeue = nt_atomic_dequeue64_up;
      dequeue_ifnexteq = nt_atomic_dequeue_ifnexteq64_up;
    }
    else {
      enqueue = nt_atomic_enqueue64_mp;
      dequeue = nt_atomic_dequeue64_mp;
      dequeue_ifnexteq = nt_atomic_dequeue_ifnexteq64_mp;
    }
  #else
    if(ncpu == 1) {
      enqueue = nt_atomic_enqueue32_up;
      if(fake64) {
        dequeue = (_dequeue_func)dlsym(RTLD_DEFAULT, "nt_atomic_dequeue32_on64_up");
        dequeue_ifnexteq = (_enqueue_ifnexteq_func)dlsym(RTLD_DEFAULT, "nt_atomic_dequeue_ifnexteq32_on64_up");
      }
      else {
        dequeue = nt_atomic_dequeue32_up;
        dequeue_ifnexteq = nt_atomic_dequeue_ifnexteq32_up;
      }
    }
    else {
      enqueue = nt_atomic_enqueue32_mp;
      if(fake64) {
        dequeue = (_dequeue_func)dlsym(RTLD_DEFAULT, "nt_atomic_dequeue32_on64_mp");
        dequeue_ifnexteq = (_enqueue_ifnexteq_func)dlsym(RTLD_DEFAULT, "nt_atomic_dequeue_ifnexteq32_on64_mp");
      } else {
        dequeue       = nt_atomic_dequeue32_mp;
        dequeue_ifnexteq = nt_atomic_dequeue_ifnexteq32_mp;
      }
    }
  #endif
  
  /* Use Darwin routines if available (ppc only on <=10.4, x86 on >10.4) */
  #ifdef __APPLE__
  if((os_enqueue = (_enqueue_func)dlsym(RTLD_DEFAULT, "OSAtomicEnqueue")) != NULL) { enqueue = os_enqueue; }
  if((os_dequeue = (_dequeue_func)dlsym(RTLD_DEFAULT, "OSAtomicDequeue")) != NULL) { dequeue = os_dequeue; }
  #endif  
  
  _enqueue = enqueue;
  _dequeue = dequeue;
  _dequeue_ifnexteq = dequeue_ifnexteq;
}

/* ------------------------------------------------------------------------- */

extern void nt_atomic_enqueue(nt_atomic_queue *queue, void *elem, size_t offset) {
  _enqueue(queue, elem, offset);
}

extern void *nt_atomic_dequeue(nt_atomic_queue *queue, size_t offset) {
  return _dequeue(queue, offset);
}

extern void *nt_atomic_dequeue_ifnexteq(nt_atomic_queue *queue, size_t offset, void *cmpptr) {
  return _dequeue_ifnexteq(queue, offset, cmpptr);
}

