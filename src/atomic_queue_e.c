#include "atomic_queue.h"

extern void nt_atomic_enqueue(nt_atomic_queue_head *queue, void *item, size_t offset) {
  tk_atomic_enqueue32_mp(queue, item, offset);
}

extern void *nt_atomic_dequeue(nt_atomic_queue_head *queue, size_t offset) {
  return tk_atomic_dequeue32_mp(queue, offset);
}

extern void *nt_atomic_dequeue_only_if(nt_atomic_queue_head *queue, size_t offset, void *onlyIf) {
  return tk_atomic_dequeueonlyif32_mp(queue, offset, onlyIf);
}

