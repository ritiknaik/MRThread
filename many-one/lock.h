#include <sys/syscall.h>
#include <unistd.h>
#include <stdatomic.h>
#include <linux/futex.h>
#include <errno.h>

typedef unsigned int mrthread_spinlock_t;
typedef unsigned int mrthread_mutex_t;

int thread_spin_init(mrthread_spinlock_t *lock);
int thread_lock(mrthread_spinlock_t *lock);
int thread_unlock(mrthread_spinlock_t *lock);
int thread_spin_trylock(mrthread_spinlock_t *lock);

int thread_mutex_init(mrthread_mutex_t *mutex);
int thread_mutex_lock(mrthread_mutex_t *mutex);
int thread_mutex_unlock(mrthread_mutex_t *mutex);
