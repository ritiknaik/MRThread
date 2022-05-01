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

threadll thread_list;
int num_thread = 1;
mrthread_spinlock_t thread_list_lock;

//cleans up the stacks and memroy
void cleanup(){
    node *tmp;
    thread_lock(&thread_list_lock);
    tmp = thread_list.start;
    while(tmp->next != NULL){
        node *p = tmp;
        tmp = tmp->next;
        deletell(&thread_list, p->thread->kernel_tid);
        
        num_thread--;
    }
    thread_unlock(&thread_list_lock);
}

//init function
static void init(){
    initll(&thread_list);
    mrthread* thread = (mrthread*)calloc(1, sizeof(mrthread));
    node *p = insertll(&thread_list, thread);
    p->thread->kernel_tid = getpid();
    atexit(cleanup);
    return;
}

//allocate stack for the thread
void* allocate_stack(size_t size){
    void* stack = NULL;
    stack = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED){
        perror("Stack Allocation");
        return NULL;
    }
    return stack;
}

//wrapper to pass the clone system call
int routine_wrapper(void* farg){
    mrthread *temp;
    temp = (mrthread*)farg;
    temp->return_value = temp->f(temp->arg);
    return 0;
}

//function to create a one-one thread using clone()
int thread_create(mrthread_t* tid, void *(*f) (void *), void *arg){
    if(tid == NULL || f == NULL){
        return EINVAL;
    }
    thread_lock(&thread_list_lock);
    void* stack = allocate_stack(STACK_SIZE);
    int t;
    static int initial = 0;
    //checking if this is the first thread
    if(initial == 0){
        initial = 1;
        init(&thread_list);
    }
    mrthread* thread = (mrthread*)calloc(1, sizeof(mrthread));
    if(thread == NULL){
        thread_unlock(&thread_list_lock);
        return -1;
    }
    thread->user_tid = num_thread;
    thread->arg = arg;
    thread->f = f;
    thread->stack = stack;
    thread->stack_size = STACK_SIZE;
    node* inserted_node = insertll(&thread_list, thread);
    mrthread_t kernel_tid = clone(routine_wrapper, thread->stack + STACK_SIZE, CLONE_ALL_FLAGS, thread, &thread->futex, thread, &thread->futex);
    if(kernel_tid == -1){
        thread_unlock(&thread_list_lock);
        perror("Clone error");
        free(stack);
        free(thread);
        return -1;
    }
    inserted_node->thread->kernel_tid = kernel_tid;
    num_thread++;
    *tid = kernel_tid;
    thread_unlock(&thread_list_lock);
    return 0;
}

//function to join a thread
int thread_join(mrthread_t tid, void **retval){
    thread_lock(&thread_list_lock);
    node *p = get_node(&thread_list, tid);
    int wstatus;
    if(p){
        thread_unlock(&thread_list_lock);
        int w = waitpid(tid, &wstatus, 0);
        thread_lock(&thread_list_lock);
        if(w == -1)
            perror("waitpid");
    }
    else{
        thread_unlock(&thread_list_lock);
        return ESRCH;
    }

    if(retval)
        *retval = p->thread->return_value;
    if(deletell(&thread_list, tid)){
        //printf("delete error");
    }
    num_thread--;
    thread_unlock(&thread_list_lock);
    return 0;
}

//function for thread exit
void thread_exit(void *retval){
    if(retval == NULL)
        return;
    thread_lock(&thread_list_lock);
    pid_t cur_pid = getpid();
    node *p = get_node(&thread_list, cur_pid);
    thread_unlock(&thread_list_lock);
    if(p){
        p->thread->return_value = retval;
        kill(cur_pid, SIGKILL);  
        munmap(p->thread->stack, STACK_SIZE);
        p->thread->stack = NULL;
        free(p->thread);
    }
    return;
}

//function to send signals to other threads
int thread_kill(mrthread_t tid, int sig){
    if(sig < 0 || sig > 64)
        return EINVAL;

    if(sig > 0){
        kill(tid, sig);
    }
    return 0;
}