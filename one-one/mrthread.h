#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sched.h>
#include<unistd.h>
#define CLONE_ALL_FLAGS CLONE_VM|CLONE_FS|CLONE_FILES

typedef struct mrthread{
    int user_tid;
    int kernel_tid;
    void* stack;
    int stack_size;
    void *(*f) (void *);
    void* arg;
    void* return_value;
} mrthread;

typedef struct node{
    struct mrthread* thread;
    struct node* next;
}node;

typedef struct threadll{
    struct node* head;
    struct node* tail;
} threadll;


int initll(threadll* ll);
node* insertll(threadll* ll, mrthread t);
int deletell(threadll* ll, mrthread t);

int thread_create(int* tid, void *(*f) (void *), void *arg);
int thread_join(int tid, void **retval);