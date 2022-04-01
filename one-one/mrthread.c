#include<mrthread.h>
#include<mman.h>
#include<errno.h>
#define STACK_SIZE 4096

threadll thread_list;

//initialize global linked list
int initll(threadll* ll){
    if (!ll)
        return -1;
    ll->head = ll->tail = NULL;
    return 0;
}

//insert to global linked list
node* insertll(threadll* ll, mrthread t){
    node* temp;
    temp = (node*)malloc(sizeof(node));
    if(temp == NULL){
        perror("malloc failed");
        return NULL;
    }
    temp->next = NULL;
    temp->thread = t;
    if(ll->head == NULL){
        ll->head = ll->tail = temp;
    }
    else{
        ll->tail->next = temp;
        ll->tail = temp;
    }
    return temp;
}

//allocatea stack of the thread
void* allocate_stack(size_t size){
    void* stack = NULL;
    stack = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED)
    {
        perror("Stack Allocation");
        return NULL;
    }
    return stack;
}

//wrapper to pass the clone system call
int wrapper(void* farg){
    mrthread *temp;
    temp = (mrthread*)farg;
    temp->return_value = temp->f(temp->arg);
    return 0;
}

//create a thread
int thread_create(int* tid, void *(*f) (void *), void *arg){
    void* stack = allocate_stack(STACK_SIZE);
    int t;
    static int initial = 0;
    if(initial == 0){
        initial = 1;
        initll(&thread_list);
    }
    mrthread* thread = (mrthread*)malloc(sizeof(mrthread));
    if(thread == NULL)
        return -1;
    thread->user_tid = tid;
    thread->arg = arg;
    thread->f = f;
    thread->stack = stack;
    thread->stack_size = STACK_SIZE;
    node* insertedNode = insertll(&thread_list, thread);
    int kernel_tid = clone(wrapper, thread->stack + STACK_SIZE, CLONE_ALL_FLAGS, thread);
    if(kernel_tid == -1){
        perror("Clone error");
        free(stack);
        free(thread);
        return -1;
    }
    thread->kernel_tid = kernel_tid;
    return 0;
}