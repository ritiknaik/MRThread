#include "mrthread.h"
#include "tests.h"
#include <stdio.h>
#include <string.h>

int *i;
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

void *thread1(){
    printf("In THREAD1\n");
    int r = 1;
    i = &r;
    return i;
}

void *thread2(){
    int r = 1010;
    thread_exit(&r);
}

void *thread3(){
    int r = 1;
    i = &r;
    return i;
}

int main(){
    LINE;
    printf("1] Thread Create Testing\n");
    LINE;
    printf("Test 1 --> Start Routine zero\n");{
        mrthread_t t1;
        to_fail(thread_create( &t1, NULL , NULL));
    }
    printf("Test 2 --> Create thread with invalid arguments\n");{
        mrthread_t t1;
        to_fail(thread_create(NULL, NULL, NULL));
        to_fail(thread_create(NULL, thread1, NULL));
    }
    printf("Test 3 --> Create thread with default attributes\n");{
        mrthread_t tid;
        Test(thread_create(&tid, thread1, NULL));
        Test(thread_join(tid, NULL));
        PASSEDTEST;
    }
    LINE;
    printf("2] Thread Join Testing\n");
    LINE;
    printf("Test 1 --> Invalid mrthread_t passed\n");
    {
        mrthread_t tid1;
        Test(thread_create(&tid1, thread1, NULL));
        mrthread_t t2;
        t2 = 111;
        to_fail(thread_join(t2, NULL));
    }

    printf("Test 2 --> Joining on already joined thread\n");{
        mrthread_t tid2;
        Test(thread_create(&tid2, thread1, NULL));
        Test(thread_join(tid2, NULL));
        to_fail(thread_join(tid2, NULL));
    }
    printf("Test 3 --> Joining on thread and getting the thread value\n");
    {
        mrthread_t tid3;
        Test(thread_create(&tid3, thread2, NULL));
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
            Test(thread_create(&tid[i], thread3, NULL));
            printf("thread %d created\n", i);
        }
        for(int i = 0; i < 5; i++) {
            Test(thread_join(tid[i], NULL));
            printf("thread %d joined\n", i);
        }
        PASSEDTEST;
    }

    LINE;
    printf("2] Thread Exit Testing\n");
    LINE;
    printf("Test 1 --> Created Thread Uses Return To Exit\n");{
        void *ret;
        mrthread_t tid;

        Test(thread_create(&tid, thread1, NULL));
        Test(thread_join(tid, &ret));
        printf("Expected return value: 1\n");
        printf("Actual return value: %d\n", *(int*)ret);
        if(*(int*)ret == 1)
            PASSEDTEST
        else
            FAILEDTEST
    }
    printf("2] Created Thread Uses mthread_exit()\n");
    {
        void *ret;
        mrthread_t tid;

        Test(thread_create(&tid, thread2, NULL));
        Test(thread_join(tid, &ret));
        printf("Expected Exit Status is %d\n", 1010);
        printf("Actual   Exit Status is %d\n", *(int*) ret);
        if(*(int*)ret == 1010)
            PASSEDTEST
        else
            FAILEDTEST
    }

    return 0;
}

