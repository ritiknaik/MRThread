
#include <stdio.h>
#include "../headers/mrthread.h"
#define GREEN "\033[0;32;32m"
#define RED "\033[0;31;31m"
#define RESET "\033[m"

//Abhijit Sir's code for Race problems
long c = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0, run = 1;
mrthread_spinlock_t lock;
void *func1(void *arg) {
	while(run == 1) {
		thread_mutex_lock(&lock);
        c++;
		thread_mutex_unlock(&lock);
		c1++;
    }
    return NULL;
}

void *func2(void *arg) {
	while(run == 1) {
        thread_mutex_lock(&lock);
        c++;
		thread_mutex_unlock(&lock);
		c2++;
    }
    return NULL;
}

void *func3(void *arg) {
	while(run == 1) {
        thread_mutex_lock(&lock);
        c++;
		thread_mutex_unlock(&lock);
		c3++;
    }
    return NULL;
}

void *func4(void *arg) {
	while(run == 1) {
        thread_mutex_lock(&lock);
        c++;
		thread_mutex_unlock(&lock);
		c4++;
    }
    return NULL;
}

void *func5(void *arg) {
	while(run == 1) {
        thread_mutex_lock(&lock);
        c++;
		thread_mutex_unlock(&lock);
		c5++;
    }
    return NULL;
}

int main() {
	mrthread_t f1, f2, f3, f4, f5; 
	int p1 = thread_create(&f1, func1, NULL);
	int p2 = thread_create(&f2, func2, NULL);
	int p3 = thread_create(&f3, func3, NULL);
	int p4 = thread_create(&f4, func4, NULL);
    int p5 = thread_create(&f4, func5, NULL);

	if(p1 != 0 || p2 != 0 || p3 !=0 || p4 != 0 || p5 != 0) {
		printf("thread_create error \n");
		return 0;
	}
    printf("Sleeping for 2 seconds\n");
	sleep(2);
	run = 0;
	thread_join(f1, NULL);
	thread_join(f2, NULL);
	thread_join(f3, NULL);
	thread_join(f4, NULL);
    thread_join(f5, NULL);

	if(c == c1 + c2 + c3 + c4 + c5) {
		printf(GREEN "Race test with 5 threads passed\n" RESET);
	}
	else {
		printf(RED "Race test with 5 threads failed\n" RESET);
	}
    printf("c = %ld\n", c);
    printf("c1+c2+c3+c4+c5 = %ld\n", c1 + c2 + c3 + c4 + c5);
    printf("c1 = %ld\n", c1);
    printf("c2 = %ld\n", c2);
    printf("c3 = %ld\n", c3);
    printf("c4 = %ld\n", c4);
    printf("c5 = %ld\n", c5);
    return 0;
}