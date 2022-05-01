#include"../headers/mrthread.h"

//function to initialize the spinlock
int thread_spin_init(mrthread_spinlock_t *lock){
    if(!lock)
        return EINVAL;
    *lock = 0;
}

//function to acquire the lock of spinlock
int thread_lock(mrthread_spinlock_t *lock){
    if(!lock)
        return EINVAL;
    while(atomic_flag_test_and_set(lock));
    return 0;
}

//function to release the lock of spinlock
int thread_unlock(mrthread_spinlock_t *lock){
    if(!lock)
        return EINVAL;
    *lock = 0;
    return 0;
}

//function to check if the lock is available or not
int thread_spin_trylock(mrthread_spinlock_t *lock){
    if(!lock)
        return EINVAL;
    if(atomic_flag_test_and_set(lock)){
        return EBUSY;
    }
    return 0;
}