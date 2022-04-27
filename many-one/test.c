#include "mrthread.h"
#include "tests.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>

int *i, infinite, mask;
void to_fail(int retval){
    if(retval != 0){
        printf("Error value : %s\n", strerror(retval));
        PASSEDTEST
    }
    else
        FAILEDTEST
}

void to_pass(int retval){
    if(retval == 0)
        PASSEDTEST
    else
        FAILEDTEST
}

int Test(int retval){
    if(retval != 0)
        printf("Error value : %s\n\n", strerror(retval));
    return retval;
}

void *func1(){
    printf("In THREAD1\n");
    int r = 1;
    i = &r;
    return i;
}

void *func2(){
    int r = 1010;
    thread_exit(&r);
}

void *func3(){
    int r = 1;
    i = &r;
    return i;
}

void *func4(){
    printf("In thread having infinite loop\n");
    int r = 30;
    mrthread_t t3;
    while(infinite);
    thread_create(&t3, func3, NULL);
    thread_join(t3, NULL);
    int *p = &r;
    return p;
}

void *func5(){
    mask = 1;
    while(mask);
}

void sigusr1_handler(){
    printf("Inside handler\n");
	infinite = 0;
    printf("returned from handler\n");
}

int main(){
    PATTERN;
    printf("1] Thread Create Testing\n");
    PATTERN;
    printf("Test 1 --> Start Routine zero\n");{
        mrthread_t t1;
        to_fail(thread_create( &t1, NULL , NULL));
    }
    printf("Test 2 --> Create thread with invalid arguments\n");{
        mrthread_t t1;
        to_fail(thread_create(NULL, NULL, NULL));
        to_fail(thread_create(NULL, func1, NULL));
    }
    printf("Test 3 --> Create thread with default attributes\n");{
        mrthread_t tid;
        Test(thread_create(&tid, func1, NULL));
        printf("create done\n");
        Test(thread_join(tid, NULL));
        PASSEDTEST;
    }
    PATTERN;
    printf("2] Thread Join Testing\n");
    PATTERN;
    printf("Test 1 --> Invalid mrthread_t passed\n");
    {
        mrthread_t tid1;
        Test(thread_create(&tid1, func1, NULL));
        mrthread_t t2;
        t2 = 111;
        to_fail(thread_join(t2, NULL));
    }

    printf("Test 2 --> Joining on already joined thread\n");{
        mrthread_t tid2;
        Test(thread_create(&tid2, func1, NULL));
        Test(thread_join(tid2, NULL));
        to_fail(thread_join(tid2, NULL));
    }
    printf("Test 3 --> Joining on thread and getting the thread value\n");
    {
        mrthread_t tid3;
        Test(thread_create(&tid3, func2, NULL));
        void *ret;
        Test(thread_join(tid3, &ret));
        printf("Expected return value: 1010\n");
        printf("Actual return value: %d\n", *(int*)ret);
        if(*(int*)ret == 1010)
            PASSEDTEST
        else
            FAILEDTEST
    }
    printf("Test 4 --> Joining on more than one thread\n");
    {
        mrthread_t tid[5];
        for(int i = 0; i < 5; i++) {
            Test(thread_create(&tid[i], func3, NULL));
            printf("thread %d created\n", i);
        }
        for(int i = 0; i < 5; i++) {
            Test(thread_join(tid[i], NULL));
            printf("thread %d joined\n", i);
        }
        PASSEDTEST;
    }

    PATTERN;
    printf("2] Thread Exit Testing\n");
    PATTERN;
    printf("Test 1 --> Created Thread Uses Return To Exit\n");{
        void *ret;
        mrthread_t tid;

        Test(thread_create(&tid, func1, NULL));
        Test(thread_join(tid, &ret));
        printf("Expected return value: 1\n");
        printf("Actual return value: %d\n", *(int*)ret);
        if(*(int*)ret == 1)
            PASSEDTEST
        else
            FAILEDTEST
    }
    printf("Test 2 --> Created Thread Uses mthread_exit()\n");
    {
        void *ret;
        mrthread_t tid;

        Test(thread_create(&tid, func2, NULL));
        Test(thread_join(tid, &ret));
        printf("Expected Exit Status is %d\n", 1010);
        printf("Actual   Exit Status is %d\n", *(int*) ret);
        if(*(int*)ret == 1010)
            PASSEDTEST
        else
            FAILEDTEST
    }

    PATTERN;
    printf("3] Thread Kill Testing\n");
    PATTERN;
    printf("Test 1 --> Send invalid signal\n");
    {
        mrthread_t tid;
        struct sigaction action;
        action.sa_handler = sigusr1_handler;
        sigaction(SIGUSR1, &action, NULL);
        infinite = 1;
        Test(thread_create(&tid, func4, NULL));
        Test(thread_kill(tid, -1));
        infinite = 0;
        Test(thread_join(tid, NULL));
        PASSEDTEST;
    }

    printf("Test 2 --> Send signal to a thread\n");
    {
        void *ret;
        mrthread_t tid;
        struct sigaction action;
        action.sa_handler = sigusr1_handler;
        sigaction(SIGUSR1, &action, NULL);
        infinite = 1;
        Test(thread_create(&tid, func4, NULL));
        Test(thread_kill(tid, SIGUSR1));
        Test(thread_join(tid, &ret));
        if(*(int*)ret == 30)
            PASSEDTEST
        else 
            FAILEDTEST
    }

    printf("Test 3 --> Checking signal handling for SIGTSTP SIGCONT SIGKILL\n");
    {
        mrthread_t tid;
        Test(thread_create(&tid, func5, NULL));
        printf("Sending SIGTSTP signal\n");
        Test(thread_kill(tid, SIGTSTP));
        printf("Sending SIGCONT signal\n");
        Test(thread_kill(tid, SIGCONT));
        printf("Sending SIGKILL signal\n");
        int ret = Test(thread_kill(tid, SIGKILL));
        if(ret == 0)
            PASSEDTEST
        else 
            FAILEDTEST
        Test(thread_join(tid, NULL));  
    }
    return 0;
}