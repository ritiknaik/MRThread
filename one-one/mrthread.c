#include"mrthread.h"
#include<sys/mman.h>
#include<errno.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<signal.h>
#include<sched.h>
#include<sys/resource.h>

#define CLONE_ALL_FLAGS CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_SYSVSEM|CLONE_PARENT_SETTID|SIGCHLD


threadll thread_list;
int num_thread = 1;

void cleanup(){
    // printf("cleanedup\n");
    node *tmp;
    tmp = thread_list.start;
    while(tmp->next != NULL){
        node *p = tmp;
        tmp = tmp->next;
        deletell(&thread_list, p->thread->kernel_tid);
        num_thread--;
    }
    // printf("numthread %d\n", num_thread);
}

static void init(){
    initll(&thread_list);
    mrthread* thread = (mrthread*)calloc(1, sizeof(mrthread));
    node *p = insertll(&thread_list, thread);
    p->thread->kernel_tid = getpid();
    atexit(cleanup);
    return;
}

//allocatea stack of the thread
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
int wrapper(void* farg){
    mrthread *temp;
    // FILE* fp;
    // fp=fopen("output.txt", "w+");

    //printf("inside wrapper\n");
    fflush(stdout);
    temp = (mrthread*)farg;
    // fclose(fp);
    temp->return_value = temp->f(temp->arg);
    // fprintf(fp, "stack: %d\n",temp->kernel_tid);
    return 0;
}

//create a thread
int thread_create(mrthread_t* tid, void *(*f) (void *), void *arg){
    //printf("inside thread_create\n");
    if(tid == NULL || f == NULL){
        return EINVAL;
    }
    void* stack = allocate_stack(STACK_SIZE);
    int t;
    static int initial = 0;
    if(initial == 0){
        initial = 1;
        init(&thread_list);
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
    //printf("before insertion\n");
    node* insertedNode = insertll(&thread_list, thread);
    //printf("after insertion\n");
    mrthread_t kernel_tid = clone(wrapper, thread->stack + STACK_SIZE, CLONE_ALL_FLAGS, thread, &thread->futex, thread, &thread->futex);
    //printf("tid: %d\n", kernel_tid);
    if(kernel_tid == -1){
        perror("Clone error");
        free(stack);
        free(thread);
        return -1;
    }
    insertedNode->thread->kernel_tid = kernel_tid;
    num_thread++;
    *tid = kernel_tid;
    //printf("finished thread create\n");
    return 0;
}

int thread_join(mrthread_t tid, void **retval){
    //printf("inside join\n");
    node *p = get_node(&thread_list, tid);
    // int found = 0;
    int wstatus;
    //printf("after search\n");
    if(p){

        // ret = syscall(SYS_futex, &p->thread->futex, FUTEX_WAIT, tid, NULL, NULL, 0);
        //printf("waiting for %d\n", tid);
        int w = waitpid(tid, &wstatus, 0);
        if(w == -1)
            perror("waitpid");
        // if(WIFEXITED(wstatus))
        //     printf("child exited\n");
        // else
        //     printf("child running\n");
    }
    else
        return ESRCH;
    // if( == -1 && errno != EAGAIN)
    //     return ret;

    if(retval)
        *retval = p->thread->return_value;
    if(deletell(&thread_list, tid)){
        printf("delete error");
    }
    num_thread--;
    return 0;
}

void thread_exit(void *retval){
    if(retval == NULL)
        return;
    pid_t cur_pid = getpid();
    node *p = get_node(&thread_list, cur_pid);
    //int found = 0;
    // while(p){
    //     if(p->thread->kernel_tid == cur_pid){
    //         found = 1;
    //         break;
    //     }
    //     p = p->next;
    // }
    if(p){
        p->thread->return_value = retval;
        kill(cur_pid, SIGKILL);  
        munmap(p->thread->stack, STACK_SIZE);
        p->thread->stack = NULL;
        free(p->thread);
    }
    return;
}

int thread_kill(mrthread_t tid, int sig){
    if(sig < 0 || sig > 64)
        return EINVAL;

    if(sig > 0){
        // pid_t th_group_id = getpid();
        // int ret = tgkill(th_group_id, tid, sig);
        kill(tid, sig);
        // if (ret == -1)
        // {
        //     perror("tgkill");
        //     return ret;
        // }
    }
    return 0;
}