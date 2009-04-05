#include "../src/atomic_queue.h"
#include <stddef.h>
#include <string.h>
#include <assert.h>

int main(int argc, char * const *argv) {
  typedef struct elem {
    long data1;
    struct elem *link;
    int data2;
  } elem_t;
  
  elem_t fred, mary, *p;
  
  nt_atomic_queue_head q2 = NT_ATOMIC_QUEUE_INIT;
  
  nt_atomic_enqueue(&q2, &fred, offsetof(elem_t,link));
  nt_atomic_enqueue(&q2, &mary, offsetof(elem_t,link));
  
  p = nt_atomic_dequeue( &q2, offsetof(elem_t,link) );
  assert(p == &mary);
  p = nt_atomic_dequeue( &q2, offsetof(elem_t,link) );
  assert(p == &fred);
  p = nt_atomic_dequeue( &q2, offsetof(elem_t,link) );
  assert(p == NULL);
  
  /* Darwin OSAtomic benchmark/comparison. Need to
  #include <libkern/OSAtomic.h>
  OSQueueHead q = OS_ATOMIC_QUEUE_INIT;
  
  OSAtomicEnqueue( &q, &fred, offsetof(elem_t,link) );
  OSAtomicEnqueue( &q, &mary, offsetof(elem_t,link) );
  
  p = OSAtomicDequeue( &q, offsetof(elem_t,link) );
  assert(p == &mary);
  p = OSAtomicDequeue( &q, offsetof(elem_t,link) );
  assert(p == &fred);*/
  
  return 0;
}
