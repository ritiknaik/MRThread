#include "../headers/mrthread.h"
#include <stdio.h>
#define GREEN "\033[0;32;32m"
#define RED "\033[0;31;31m"
#define RESET "\033[m"
#define MAX 16

//function to test library on binary search
int keys[] = { 1, 12, 45, 55, 65, 71, 77, 89, 91, 123, 135, 140, 146, 155, 169, 221 };

int key = 169;
int is_present = 0;
struct arg_struct{
    int begin;
    int end;
};
int result_expected = 1;

void* binary_search(void* arg){
    struct arg_struct *argument = (struct arg_struct *)arg;
    int begin = argument->begin;
    int end = argument->end;
    int mid;
    while (begin < end && !is_present){
        mid = (end - begin) / 2 + begin;
        if (keys[mid] == key){
            is_present = 1;
          break;
        }
        else if (keys[mid] > key)
            end = mid - 1;
        else
            begin = mid + 1;
    }
}

int main(){
    mrthread_t threads[4];
    for (int i = 0; i < 4; i++){
        struct arg_struct argument;
        argument.begin = i * (MAX / 4);
        argument.end = (i+1) * (MAX / 4);
        thread_create(&threads[i],binary_search, (void *)&argument);
    }
    for (int i = 0; i < 4; i++){
        thread_join(threads[i], NULL);
    }
    if (is_present == result_expected)
        printf(GREEN "Binary search using Multi-threading passed\n" RESET);
    else
    	printf(RED "Binary search using Multi-threading failed\n" RESET);
}