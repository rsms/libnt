#if defined(__x86_64__)
typedef volatile struct {
	volatile void *opaque1;
	volatile long opaque2;
} nt_atomic_queue_head __attribute__ ((aligned (16)));
#else
typedef volatile struct {
	volatile void *opaque1;
	volatile long opaque2;
} nt_atomic_queue_head;
#endif


#define	NT_ATOMIC_QUEUE_INIT	(nt_atomic_queue_head){ NULL, 0 }


#ifdef __LP64__

void tk_atomic_enqueue64_mp(nt_atomic_queue_head *queue, void *item, size_t offset);
void *tk_atomic_dequeue64_mp(nt_atomic_queue_head *queue, size_t offset);
void *tk_atomic_dequeueonlyif64_mp(nt_atomic_queue_head *queue, size_t offset, void *onlyIf);

void tk_atomic_enqueue64_up(nt_atomic_queue_head *queue, void *item, size_t offset);
void *tk_atomic_dequeue64_up(nt_atomic_queue_head *queue, size_t offset);
void *tk_atomic_dequeueonlyif64_up(nt_atomic_queue_head *queue, size_t offset, void *onlyIf);

#else // __LP64__ not defined

void tk_atomic_enqueue32_mp(nt_atomic_queue_head *queue, void *item, size_t offset);
void *tk_atomic_dequeue32_mp(nt_atomic_queue_head *queue, size_t offset);
void *tk_atomic_dequeueonlyif32_mp(nt_atomic_queue_head *queue, size_t offset, void *onlyIf);

void tk_atomic_enqueue32_up(nt_atomic_queue_head *queue, void *item, size_t offset);
void *tk_atomic_dequeue32_up(nt_atomic_queue_head *queue, size_t offset);
void *tk_atomic_dequeueonlyif32_up(nt_atomic_queue_head *queue, size_t offset, void *onlyIf);


void *tk_atomic_dequeue32_on64_mp(nt_atomic_queue_head *queue, size_t offset);
void *tk_atomic_dequeueonlyif32_on64_mp(nt_atomic_queue_head *queue, size_t offset, void *onlyIf);

void *tk_atomic_dequeue32_on64_up(nt_atomic_queue_head *queue, size_t offset);
void *tk_atomic_dequeueonlyif32_on64_up(nt_atomic_queue_head *queue, size_t offset, void *onlyIf);


// the interface

NT_STATIC_INLINE void nt_atomic_enqueue(nt_atomic_queue_head *queue, void *item, size_t offset) {
  tk_atomic_enqueue32_mp(queue, item, offset);
}

NT_STATIC_INLINE void *Tnt_atomic_dequeue(nt_atomic_queue_head *queue, size_t offset) {
  return tk_atomic_dequeue32_mp(queue, offset);
}

NT_STATIC_INLINE void *nt_atomic_dequeue_only_if(nt_atomic_queue_head *queue, size_t offset, void *onlyIf) {
  return tk_atomic_dequeueonlyif32_mp(queue, offset, onlyIf);
}

#endif
