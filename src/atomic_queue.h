#ifndef _NT_ATOMIC_QUEUE_H_
#define _NT_ATOMIC_QUEUE_H_
/**
  Lock-free, thread-safe FIFO-queue.
  
  Modelled after the Darwin/XNU OSAtomic queue.
  
  Example:
  
    typedef struct elem {
      long data1;
      struct elem *link;
      int data2;
    } elem_t;
    
    elem_t fred, mary, *p;
    nt_atomic_queue q = NT_ATOMIC_QUEUE_INIT;
    
    nt_atomic_enqueue( &q, &fred, offsetof(elem_t,link) );
    nt_atomic_enqueue( &q, &mary, offsetof(elem_t,link) );
    
    p = nt_atomic_dequeue( &q, offsetof(elem_t,link) );

  In this example, the call of nt_atomic_dequeue() will return a ptr to mary.

*/
#include <sys/types.h>

/**
  Queue type
*/
typedef volatile struct {
	volatile void *opaque1;
	volatile long opaque2;
#if defined(__x86_64__)
} nt_atomic_queue NT_ATTR(aligned (16));
#else
} nt_atomic_queue;
#endif

/**
  Initialize a queue.
*/
#define	NT_ATOMIC_QUEUE_INIT (nt_atomic_queue){ NULL, 0L }

/**
  Enqueue (push) an element in @queue.
  
  @param queue  pointer to a nt_atomic_queue.
  @param elem   the element to enqueue.
  @param offset offset in bytes to the link pointer of @elem. Usually
                offsetof(my_struct, link_member).
*/
extern void nt_atomic_enqueue(nt_atomic_queue *queue, void *elem, size_t offset) NT_ATTR(nonnull(1,2));

/**
  Dequeue (pop) the first (top/next) element from a @queue.
  
  @param queue  pointer to a nt_atomic_queue
  @param offset offset in bytes to the link pointer. Usually
                offsetof(my_struct, link_member).
  @returns pointer to a dequeued element or NULL if the queue is empty.
*/
extern void *nt_atomic_dequeue(nt_atomic_queue *queue, size_t offset) NT_ATTR(nonnull(1));

/**
  CAS-version of nt_atomic_dequeue.
  
  Returns the first (next) element if it has the same address as @cmpptr,
  otherwise the second element, if any, is returned.
  
  @param queue  pointer to a nt_atomic_queue
  @param offset offset in bytes to the link pointer. Usually
                offsetof(my_struct, link_member).
  @param cmpptr compare next element to this address; if they are the same, the
                next element is returned. Otherwise the second element, if any,
                is returned.
  @returns pointer to a dequeued element or NULL if the queue is empty.
*/
extern void *nt_atomic_dequeue_ifnexteq(nt_atomic_queue *queue, size_t offset, void *cmpptr) NT_ATTR(nonnull(1,3));


#if !defined (__GNUC__) || (__GNUC__ < 4)
/**
  Initialize library.
  
  This need to be called once in order to initialize the library on targets
  not supporting "constructors".
*/
void nt_atomic_queue_init(void);
#endif


#endif // _NT_ATOMIC_QUEUE_H_
