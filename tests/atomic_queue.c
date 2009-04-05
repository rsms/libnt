/**
 This code is released in the Public Domain (no restrictions, no support
 100% free) by Notion.
*/
#include "../src/atomic_queue.h"
#include <stddef.h>
#include <string.h>
#include <assert.h>

//#include <stdio.h>

int main(int argc, char * const *argv) {
  typedef struct elem {
    long data1;
    volatile struct elem *link;
    int data2;
  } elem_t;
  
  elem_t fred={1L,NULL,1}, mary={2L,NULL,2}, *p;
  nt_atomic_queue q = NT_ATOMIC_QUEUE_INIT;
  
  nt_atomic_enqueue(&q, &fred, offsetof(elem_t,link));
  nt_atomic_enqueue(&q, &mary, offsetof(elem_t,link));
  
  p = nt_atomic_dequeue( &q, offsetof(elem_t,link) );
  assert(p == &mary);
  //printf("p={%ld, %p, %d}\n", p->data1, p->link, p->data2);
  
  p = nt_atomic_dequeue_ifnexteq( &q, offsetof(elem_t,link), &mary );
  assert(p == NULL); /* should fail because the next entry is fred, not mary. */
  
  p = nt_atomic_dequeue( &q, offsetof(elem_t,link) );
  assert(p == &fred);
  //printf("p={%ld, %p, %d}\n", p->data1, p->link, p->data2);
  
  p = nt_atomic_dequeue( &q, offsetof(elem_t,link) );
  assert(p == NULL);
  
  
  /* Darwin OSAtomic benchmark/comparison. Need to
  #include <libkern/OSAtomic.h>
  OSQueueHead q2 = OS_ATOMIC_QUEUE_INIT;
  
  OSAtomicEnqueue( &q2, &fred, offsetof(elem_t,link) );
  OSAtomicEnqueue( &q2, &mary, offsetof(elem_t,link) );
  
  p = OSAtomicDequeue( &q2, offsetof(elem_t,link) );
  assert(p == &mary);
  p = OSAtomicDequeue( &q2, offsetof(elem_t,link) );
  assert(p == &fred);*/
  
  return 0;
}
