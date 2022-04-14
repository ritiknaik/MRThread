#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
// #include <pthread.h>
#include "mrthread.h"
  
// A normal C function that is executed as a thread 
// when its name is specified in pthread_create()
void *myThreadFun(void *vargo)
{
    sleep(3);
    FILE* fp;
    fp = fopen("output.txt", "w+");
    fprintf(fp, "Printing from Thread \n");
    fclose(fp);
    return NULL;
}
   
int main()
{
    mrthread_t thread_id;
    printf("Before Thread\n");
    thread_create(&thread_id,myThreadFun,NULL);
    printf("main runnning\n");
    thread_join(thread_id, NULL);
    //pthread_create(&thread_id, NULL, myThreadFun, NULL);
    //pthread_join(thread_id, NULL);
    printf("After Thread\n");
    exit(0);
}