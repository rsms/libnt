#ifndef _NT_ATOMIC_QUEUE_H_
#define _NT_ATOMIC_QUEUE_H_

#include <sys/types.h>

#if defined(__x86_64__)
typedef volatile struct {
	volatile void *opaque1;
	volatile long opaque2;
} nt_atomic_queue_head NT_ATTR(aligned (16));
#else
typedef volatile struct {
	volatile void *opaque1;
	volatile long opaque2;
} nt_atomic_queue_head;
#endif


#define	NT_ATOMIC_QUEUE_INIT (nt_atomic_queue_head){ NULL, 0 }

extern void nt_atomic_enqueue(nt_atomic_queue_head *queue, void *item, size_t offset) NT_ATTR(nonnull(1,2));
extern void *nt_atomic_dequeue(nt_atomic_queue_head *queue, size_t offset) NT_ATTR(nonnull(1));
extern void *nt_atomic_dequeue_only_if(nt_atomic_queue_head *queue, size_t offset, void *onlyIf) NT_ATTR(nonnull(1,3));


#if !defined (__GNUC__) || (__GNUC__ < 4)
/**
  Initialize library.
  
  This need to be called once in order to initialize the library on targets
  not supporting "constructors".
*/
void nt_atomic_queue_init(void);
#endif


#endif // _NT_ATOMIC_QUEUE_H_
