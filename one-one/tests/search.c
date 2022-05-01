#include "../headers/mrthread.h"
#include <stdio.h>
#define GREEN "\033[0;32;32m"
#define RED "\033[0;31;31m"
#define RESET "\033[m"
#define MAX 16

//testing binary search with multithreading library
int keys[] = { 1, 12, 45, 55, 65, 71, 77, 89, 91, 123, 135, 140, 146, 155, 169, 221 };
int key = 140;
int is_present = 0;

mrthread_spinlock_t lock;
struct arg_struct {
    int begin;
    int end;
};
int result_expected = 1;  

void* binary_search(void* arg){
    struct arg_struct *argument = (struct arg_struct *)arg;
    int begin = argument->begin;
    int end = argument->end; 
    int mid;
    while (begin < end && !is_present) { 
        mid = (end - begin) / 2 + begin;
        if (keys[mid] == key) {
            thread_lock (&lock);
            is_present = 1;
            thread_unlock(&lock);
          break;
        }
        else if (keys[mid] > key)
            end = mid - 1;
        else
            begin = mid + 1;
    }
}

int main(){
    //testing using 4 threads
    mrthread_t threads[4];
    struct arg_struct argument;
    for(int i = 0; i < 4; i++){
        thread_lock(&lock);
        argument.begin = i * (MAX / 4);
        argument.end = (i+1) * (MAX / 4);
        thread_unlock(&lock);
        thread_create(&threads[i],binary_search, (void *)&argument);
    } 
    for(int i = 0; i < 4; i++){
        thread_join(threads[i], NULL);
    }

    if(is_present == result_expected)
        printf(GREEN "Binary search using 4 threads passed\n" RESET);
    else
    	printf(RED "Binary search using 4 threads failed\n" RESET);
    return 0;
}