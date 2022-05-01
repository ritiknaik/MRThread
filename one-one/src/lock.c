#include"../headers/lock.h"

//function to initialize spin lock of thread
int thread_spin_init(mrthread_spinlock_t *lock){
    if(!lock)
        return EINVAL;
    *lock = 0;
}

//function to acquire the lock of spinlock atomically
int thread_lock(mrthread_spinlock_t *lock){
    if(!lock)
        return EINVAL;
    while(atomic_flag_test_and_set(lock));
    return 0;
}

//function to release the lock of spinlock atomically
int thread_unlock(mrthread_spinlock_t *lock){
    if(!lock)
        return EINVAL;
    *lock = 0;
    return 0;
}

//function to check if spinlock is available or not
int thread_spin_trylock(mrthread_spinlock_t *lock){
    if(!lock)
        return EINVAL;
    if(atomic_flag_test_and_set(lock)){
        return EBUSY;
    }
    return 0;
}

//function to initialize mutex lock of thread
int thread_mutex_init(mrthread_mutex_t *mutex){
    if(!mutex)
        return EINVAL;
    *mutex = 0;
    return 0;
}

//function to acquire lock of mutex and sleeping 
//when the mutex is locked
int thread_mutex_lock(mrthread_mutex_t *mutex){
    if(!mutex)
        return EINVAL;

    int ret;
    while(1){
        const int is_locked = 0;
        if(atomic_compare_exchange_strong(mutex, &is_locked, 1)){
            break;
        }
        ret = syscall(SYS_futex, mutex, FUTEX_WAIT, 0, NULL, NULL, 0);
        if(ret == -1 && errno != EAGAIN){
            perror("futex wait error");
        }
    }
    return 0;
}

//function to release the lock of mutex and waking
//up one process which is waiting to acquire the lock
int thread_mutex_unlock(mrthread_mutex_t *mutex){
    if(!mutex)
        return EINVAL;
    
    int ret;
    const int is_unlocked = 1;
    if(atomic_compare_exchange_strong(mutex, &is_unlocked, 0)){
        ret = syscall(SYS_futex, mutex, FUTEX_WAKE, 1, NULL, NULL, 0);
        if(ret == -1){
            perror("futex wake error");
        }
    }
    return 0;
}