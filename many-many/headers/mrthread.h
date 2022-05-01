#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<setjmp.h>
#include<signal.h>
#include<errno.h>
#include<string.h>
#include<sys/mman.h>
#include<sys/time.h>
#include"lock.h"

#define STACK_SIZE 65536
#define MAX_KTHREADS 10

#define RUNNING 0
#define READY 1
#define TERMINATED 2
#define WAITING 3
#define JOINED 4
#define STOPPED 5

#define JB_RBP 5
#define JB_RSP 6
#define JB_PC 7

typedef pid_t mrthread_t;

//structure of thread
typedef struct mrthread{
    int user_tid;
    int kernel_tid;
    int32_t futex;
    void *stack;
    int stack_size;
    void *(*f) (void *);
    void *arg;
    void *return_value;
    jmp_buf context;
    sigset_t signal_set;
    int state;
    int *waiting_threads; //list of all threads waiting on this process
    int wait_count;
}mrthread;

void routine_wrapper(void* farg);

//library functions
int thread_create(int* tid, void *(*f) (void *), void *arg);
int thread_join(int tid, void **retval);
void thread_exit(void *retval);
int thread_kill(mrthread_t tid, int sig);

//structure of node for queue
typedef struct node{
    struct mrthread* thread;
    struct node* next;
}node;

//structure for global queue of threads
typedef struct thread_queue{
    int num;
    node *start;
    node *end;
}thread_queue;

//queue functionalities
int init_q(thread_queue* q);
void enqueue_q(thread_queue* q, mrthread* t);
mrthread* dequeue_q(thread_queue* q);
mrthread* get_node(thread_queue* q, mrthread_t tid);
int is_empty_q(thread_queue *q);
mrthread* thread_to_sched(thread_queue* q);