#include "../headers/mrthread.h"
#include <sys/mman.h>
#include <errno.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sched.h>
#include <sys/resource.h>

#define CLONE_ALL_FLAGS CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_SYSVSEM|CLONE_PARENT_SETTID|SIGCHLD

//array of threads that are currently on
//each of the MAX_KTHREADS kernel threads
mrthread* running_threads[MAX_KTHREADS];
mrthread_t kernel_tids[MAX_KTHREADS];
thread_queue threads;
int num_thread = 0;

//pseudo code for thread create
int thread_create(int* tid, void *(*f) (void *), void *arg){
    //block timer interrupt for this thread
    if(tid == NULL || f == NULL){
        //block timer interrupt for this thread
        return EINVAL;
    }
    //if first thread then need to initialize the library
    static int initial = 0;
    if(!initial){
        init();
        initial = 1;
    }
    mrthread* thread = (mrthread*)malloc(sizeof(mrthread));
    if(!thread){
        perror("Malloc");
        exit(0);
    }
    thread->stack_size = STACK_SIZE;
    thread->stack = allocate_stack(thread->stack_size);
    if(!thread->stack){
        exit(0);
    }
    thread->arg = arg;
    thread->f = f;
    thread->wait_count = 0;
    thread->state = READY;
    thread->waiting_threads = NULL;
    thread->return_value = NULL;
    thread->user_tid = num_thread;
    sigemptyset(&(thread->signal_set));
    if(num_thread < MAX_KTHREADS){
        thread->state = RUNNING;
        mrthread_t kernel_tid = clone(routine_wrapper, thread->stack + STACK_SIZE, CLONE_ALL_FLAGS, thread, &thread->futex, thread, &thread->futex);
        if(kernel_tid == -1){
            perror("Clone error");
            //free the stack
            free(thread);
            return -1;
        }
        thread->kernel_tid = kernel_tid;
        num_thread++;
        enqueue_q(&threads, thread);
    }
    else{
        set_context(thread);
        *tid = thread->user_tid;
        enqueue_q(&threads, thread);
        num_thread++;
        //call scheduler for (num_thread - 1)% MAX_KTHREADS kernel thread
    }
    //unblock the timer before leaving
    return 0;
}

int thread_join(int tid, void **retval);
void thread_exit(void *retval);
int thread_kill(mrthread_t tid, int sig);