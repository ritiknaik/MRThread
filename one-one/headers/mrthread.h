#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include"lock.h"
#include<errno.h>
#include<sys/mman.h>
#define STACK_SIZE 65536

typedef pid_t mrthread_t;

typedef struct mrthread{
    int user_tid;
    mrthread_t kernel_tid;
    int32_t futex;
    void* stack;
    int stack_size;
    void *(*f) (void *);
    void* arg;
    void* return_value;
} mrthread;

//utility function
void cleanup();
static void init();
int routine_wrapper(void* farg);
void* allocate_stack(size_t size);

//thread functions
int thread_create(int* tid, void *(*f) (void *), void *arg);
int thread_join(int tid, void **retval);
void thread_exit(void *retval);
int thread_kill(mrthread_t tid, int sig);

//linked list structure
typedef struct node{
    struct mrthread* thread;
    struct node* next;
}node;

typedef struct threadll{
    struct node* start;
    struct node* end;
} threadll;

//linked list manipulation code
int initll(threadll* ll);
node* insertll(threadll* ll, mrthread* t);
int deletell(threadll* ll, int tid);
node* get_node(threadll* ll, mrthread_t tid);