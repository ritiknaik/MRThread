#include "../headers/mrthread.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#define RED "\033[1;31m"
#define RESET "\033[0m"
#define GREEN "\e[0;32m"

//testing file to test all functionality of the library
int *i, infinite, mask, succ_count, fail_count;

void pattern(void){
    printf("__________________________________\n");
}

//function for test cleared
void test_cleared(void){
    printf(GREEN "Test Passed\n" RESET);
    succ_count++;
}

//function for test failed
void test_failed(void){
    printf(RED "Test Failed\n" RESET);
    fail_count++;
}

//function to check if test was failed successfully
void to_fail(int retval){
    if(retval != 0){
        test_cleared();
    }
    else{
        test_failed();
    }       
}

//function to check if passed successfully
void to_pass(int retval){
    if(retval == 0){
        test_cleared();
    }    
    else{
        test_failed();
    }
        
}

//function to test if return value was 0 or not
int Test(int retval){
    if(retval != 0)
        printf("Error value : %s\n", strerror(retval));
    return retval;
}

void *func1(){
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

//function having infinite loop to test signal
//internally creatng and joining one thread
void *func4(){
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

//signal handler for sigusr1
void sigusr1_handler(){
	infinite = 0;
}

int main(){
    pattern();
    printf("1] Thread Create Testing\n");
    pattern();
    
    /* Testing with start routine NULL */
    {
        mrthread_t t1;
        to_fail(thread_create( &t1, NULL , NULL));
    }

    /* Testing with invalid arguments */
    {
        mrthread_t t1;
        to_fail(thread_create(NULL, NULL, NULL));
        to_fail(thread_create(NULL, func1, NULL));
    }

    /* Testing with default attributes */
    {
        mrthread_t tid;
        Test(thread_create(&tid, func1, NULL));
        Test(thread_join(tid, NULL));
        test_cleared();
    }
    pattern();
    printf("2] Thread Join Testing\n");
    pattern();

    /* Testing with invalid TID */
    {
        mrthread_t tid1;
        Test(thread_create(&tid1, func1, NULL));
        mrthread_t t2;
        t2 = 111;
        to_fail(thread_join(t2, NULL));
    }

    /* Testing with already joined thread */
    {
        mrthread_t tid2;
        Test(thread_create(&tid2, func1, NULL));
        Test(thread_join(tid2, NULL));
        to_fail(thread_join(tid2, NULL));
    }

    /* Testing and retrieving return value */
    {
        mrthread_t tid3;
        Test(thread_create(&tid3, func2, NULL));
        void *ret;
        Test(thread_join(tid3, &ret));
        if(*(int*)ret == 1010){
            test_cleared();
        }    
        else{
            test_failed();
        }  
    }

    /* Testing multiple creates and joins */
    {
        mrthread_t tid[5];
        for(int i = 0; i < 5; i++) {
            if(Test(thread_create(&tid[i], func3, NULL)) == 0)
                succ_count;
        }
        for(int i = 0; i < 5; i++) {
            Test(thread_join(tid[i], NULL));
        }
        test_cleared();
    }

    pattern();
    printf("2] Thread Exit Testing\n");
    pattern();

    /* Testing Created Thread Uses Return To Exit */
    {
        void *ret;
        mrthread_t tid;

        Test(thread_create(&tid, func1, NULL));
        Test(thread_join(tid, &ret));
        if(*(int*)ret == 1){
            test_cleared();
        }   
        else{
            test_failed();
        }
    }

    /* Test 2 --> Created Thread Uses mrthread_exit() */
    {
        void *ret;
        mrthread_t tid;

        Test(thread_create(&tid, func2, NULL));
        Test(thread_join(tid, &ret));
        if(*(int*)ret == 1010){
            test_cleared();
        }
        else{
            test_failed();
        }
    }

    pattern();
    printf("3] Thread Kill Testing\n");
    pattern();

    /* Testing with invalid signal */
    {
        mrthread_t tid;
        struct sigaction action;
        action.sa_handler = sigusr1_handler;
        sigaction(SIGUSR1, &action, NULL);
        infinite = 1;
        Test(thread_create(&tid, func4, NULL));
        to_fail(thread_kill(tid, -1));
        infinite = 0;
        Test(thread_join(tid, NULL));
        test_cleared();
    }

    /* Testing of sending signal to a thread */
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
        if(*(int*)ret == 30){
            test_cleared();
        }
        else{ 
            test_failed();
        }
    }

    /* Testing signals SIGTSTP SIGCONT SIGKILL */
    {
        mrthread_t tid;
        Test(thread_create(&tid, func5, NULL));
        Test(thread_kill(tid, SIGTSTP));
        Test(thread_kill(tid, SIGCONT));
        int ret = Test(thread_kill(tid, SIGKILL));
        if(ret == 0){
            test_cleared();
        }
        else{
            test_failed();
        }
        Test(thread_join(tid, NULL));
    }

    printf(GREEN "\nSUCCESS COUNT = %d\n" RESET,succ_count);
    if(fail_count>0)
        printf(RED "FAILURE COUNT = %d\n" RESET, fail_count);
    return 0;
}