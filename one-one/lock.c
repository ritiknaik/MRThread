#include"lock.h"

int thread_spin_init(mrthread_spinlock_t *lock){
    if(!lock)
        return EINVAL;
    *lock = 0;
}

int thread_lock(mrthread_spinlock_t *lock){
    if(!lock)
        return EINVAL;
    while(atomic_flag_test_and_set(lock));
    return 0;
}

int thread_unlock(mrthread_spinlock_t *lock){
    if(!lock)
        return EINVAL;
    *lock = 0;
    return 0;
}

int thread_spin_trylock(mrthread_spinlock_t *lock){
    if(!lock)
        return EINVAL;
    if(atomic_flag_test_and_set(lock)){
        return EBUSY;
    }
    return 0;
}

int thread_mutex_init(mrthread_mutex_t *mutex);
int thread_mutex_lock(mrthread_mutex_t *mutex);
int thread_mutex_unlock(mrthread_mutex_t *mutex);