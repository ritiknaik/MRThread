#include"mrthread.h"
#include<sys/mman.h>
#include<errno.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<sched.h>

threadll thread_list;
int num_thread = 1;
//allocatea stack of the thread
void* allocate_stack(size_t size){
    void* stack = NULL;
    stack = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED)
    {
        perror("Stack Allocation");
        return NULL;
    }
    return stack;
}

//wrapper to pass the clone system call
int wrapper(void* farg){
    mrthread *temp;
    printf("inside wrapper\n");
    temp = (mrthread*)farg;
    temp->return_value = temp->f(temp->arg);
    printf("stack: %d\n",temp->kernel_tid);
    return 0;
}

//create a thread
int thread_create(mrthread_t* tid, void *(*f) (void *), void *arg){
    printf("inside thread_create\n");
    void* stack = allocate_stack(STACK_SIZE);
    int t;
    static int initial = 0;
    if(initial == 0){
        initial = 1;
        initll(&thread_list);
    }
    mrthread* thread = (mrthread*)calloc(1, sizeof(mrthread));
    if(thread == NULL){
        return -1;
    }
    thread->user_tid = num_thread;
    thread->arg = arg;
    thread->f = f;
    thread->stack = stack;
    thread->stack_size = STACK_SIZE;
    printf("before insertion\n");
    node* insertedNode = insertll(&thread_list, thread);
    printf("after insertion\n");
    mrthread_t kernel_tid = clone(wrapper, thread->stack + STACK_SIZE, CLONE_ALL_FLAGS, thread, &thread->futex, thread, &thread->futex);
    printf("tid: %d\n", kernel_tid);
    if(kernel_tid == -1){
        perror("Clone error");
        free(stack);
        free(thread);
        return -1;
    }
    insertedNode->thread->kernel_tid = kernel_tid;
    num_thread++;
    *tid = kernel_tid;
    printf("finished thread create\n");
    return 0;
}

int thread_join(mrthread_t tid, void **retval){
    printf("inside join\n");
    node *p = thread_list.head;
    int found = 0;
    int ret;
    while(p){
        if(p->thread->kernel_tid == tid){
            found = 1;
            break;
        }
        p = p->next;
    }
    printf("after search\n");
    if(found){

        // ret = syscall(SYS_futex, &p->thread->futex, FUTEX_WAIT, tid, NULL, NULL, 0);
        printf("waiting\n");
        waitpid(tid, NULL, 0);
    }
    if(ret == -1 && errno != EAGAIN)
        return ret;

    if(retval)
        *retval = p->thread->return_value;
    if(deletell(&thread_list, tid)){
        printf("delete error");
    }
    num_thread--;
    return 0;
}