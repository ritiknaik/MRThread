#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdatomic.h>
#include<setjmp.h>
#include<signal.h>
#include<errno.h>
#include<string.h>
#include<sys/mman.h>
#include<sys/time.h>
#include"lock.h"

#define STACK_SIZE 65536

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

typedef struct mrthread{
    int user_tid;
    // mrthread_t kernel_tid;
    // int32_t futex;
    void *stack;
    int stack_size;
    void *(*f) (void *);
    void *arg;
    void *return_value;
    sigset_t signal_set;
    jmp_buf context;
    int state;
    int *waiting_threads; //list of all waiters on this process
    int wait_count;
}mrthread;

// void cleanup(thread_queue *q);
void alarm_handle();
long int mangle(long int p);
void block_timer();
void unblock_timer();
void* allocate_stack(size_t size);
void init();
void timer_init();
void set_context(mrthread* thread);
void wrapper(void* farg);
void raise_pending_signals();
void scheduler();

int thread_create(int* tid, void *(*f) (void *), void *arg);
int thread_join(int tid, void **retval);
void thread_exit(void *retval);
int thread_kill(mrthread_t tid, int sig);


typedef struct node{
    struct mrthread* thread;
    struct node* next;
}node;

typedef struct thread_queue{
    int num;
    node *start;
    node *end;
}thread_queue;

int init_q(thread_queue* q);
void enqueue_q(thread_queue* q, mrthread* t);
mrthread* dequeue_q(thread_queue* q);
mrthread* get_node(thread_queue* q, mrthread_t tid);
int is_empty_q(thread_queue *q);
mrthread* thread_to_sched(thread_queue* q);