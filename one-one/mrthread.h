#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#define CLONE_ALL_FLAGS CLONE_VM|CLONE_FS|CLONE_FILES

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


int thread_create(int* tid, void *(*f) (void *), void *arg);
int thread_join(int tid, void **retval);


#define STACK_SIZE 4096

typedef struct node{
    struct mrthread* thread;
    struct node* next;
}node;

typedef struct threadll{
    struct node* head;
    struct node* tail;
} threadll;


int initll(threadll* ll);
node* insertll(threadll* ll, mrthread* t);
int deletell(threadll* ll, int tid);