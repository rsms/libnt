#ifndef _NT_SPINLOCK_H_
#define _NT_SPINLOCK_H_

#include "atomic.h"
#include <stdbool.h>

/* Prototypes:
extern void nt_spinlock_init(nt_spinlock_t *lock);
extern bool nt_spinlock_try(nt_spinlock_t *lock);
extern void nt_spinlock_lock(nt_spinlock_t *lock);
extern void nt_spinlock_unlock(nt_spinlock_t *lock);
*/

#ifdef __APPLE__XXX

#include <libkern/OSAtomic.h>
typedef OSSpinLock nt_spinlock_t;
#define nt_spinlock_init(lock) nt_atomic_set32(&lock, 0)
#define nt_spinlock_try(lock) OSSpinLockTry(lock)
#define nt_spinlock_lock(lock) OSSpinLockLock(lock)
#define nt_spinlock_unlock(lock) OSSpinLockUnlock(lock)

#else

#include <stdint.h>
typedef volatile int32_t nt_spinlock_t;
#ifndef __SMP__
#define nt_spinlock_init(lock)
#define nt_spinlock_try(lock) (true)
#define nt_spinlock_lock(lock)
#define nt_spinlock_unlock(lock)
#else	/* __SMP__ */
#define nt_spinlock_init(lock) nt_atomic_set32(&lock, 0)
#define nt_spinlock_try(lock) nt_atomic_bool_compare_and_swap32(&lock, 0, 1)
#define nt_spinlock_lock(lock) while(!nt_atomic_bool_compare_and_swap32(&lock, 0, 1))
#define nt_spinlock_unlock(lock) nt_atomic_set32(&lock, 0)
#endif /* __SMP__ */

#endif /* __APPLE__ */


#endif
