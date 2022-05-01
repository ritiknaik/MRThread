#include"../headers/mrthread.h"


int thread_spin_init(mrthread_spinlock_t *lock){
    if(!lock)
        return EINVAL;
    // block_timer();
    *lock = 0;
    // unblock_timer();
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
    // block_timer();
    *lock = 0;
    // unblock_timer();
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