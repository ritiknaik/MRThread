#include "mrthread.h"

jmp_buf env;
mrthread* running_thread;
thread_queue threads;
int num_thread = 0;

void block_timer(){
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_BLOCK, &set, NULL);
}

void unblock_timer(){
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
}

void* allocate_stack(size_t size){
    void* stack = NULL;
    stack = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED){
        perror("Stack Allocation");
        return NULL;
    }
    return stack;
}

void init(){
    init_q(&threads);

    mrthread* main = (mrthread*)malloc(sizeof(mrthread));
    if(!main){
        perror("Malloc");
        return;
    }
    main->stack = NULL;
    main->arg = NULL;
    main->return_value = NULL;
    main->f = NULL;
    main->user_tid = num_thread;
    main->state = RUNNING;
    main->waiting_threads = NULL;
    main->wait_count = 0;
    main->stack_size = STACK_SIZE;
    ///////////////////
    setjmp(main->context);
    sigemptyset(&(main->signal_set));
    running_thread = main;
    num_thread++;
    timer_init();
}

void timer_init(){
    sigset_t mask;
    sigfillset(&mask);
    
    struct sigaction timer_handler;
    memset(&timer_handler, 0, sizeof(timer_handler));
    timer_handler.sa_handler = scheduler;
	timer_handler.sa_flags = SA_RESTART;
	timer_handler.sa_mask = mask;
   
    sigaction(SIGALRM, &timer_handler, NULL);  
    struct itimerval clock;
    clock.it_interval.tv_sec = 0 ;
    clock.it_interval.tv_usec = 100;
    clock.it_value.tv_sec = 0;
    clock.it_value.tv_usec = 100;
    //start timer
	setitimer(ITIMER_REAL, &clock, 0);
}

void scheduler();

int wrapper(void* farg){
    mrthread *temp;
    fflush(stdout);
    temp = (mrthread*)farg;
    temp->return_value = temp->f(temp->arg);

    return 0;
}

void set_context(mrthread* thread){
    setjmp(thread->context);
    thread->context[0].__jmpbuf[JB_RSP] = mangle((long int) thread->stack + STACK_SIZE - sizeof(long int));
	thread->context[0].__jmpbuf[JB_PC] = mangle((long int) wrapper);
}

int thread_create(int* tid, void *(*f) (void *), void *arg){
    block_timer();
    if(tid == NULL || f == NULL){
        return EINVAL;
    }
    mrthread* thread = (mrthread*)malloc(sizeof(mrthread));
    if(!thread){
        perror("Malloc");
        exit(0);
    }
    thread->stack_size = STACK_SIZE;
    thread->stack = allocate_stack(thread->stack_size);
    if(!thread->stack){
        exit(0);
    }
    thread->arg = arg;
    thread->f = f;
    thread->state = READY;
    thread->waiting_threads = NULL;
    thread->user_tid = num_thread;
    thread->wait_count = 0;
    thread->return_value = NULL;
    sigsetempty(&(thread->signal_set));

    set_context(thread);
    *tid = thread->user_tid;
    enqueue_q(&threads, thread);
    num_thread++;
    unblock_timer();
    return 0;
}

int thread_join(int tid, void **retval){
    block_timer();
    if(tid == running_thread->user_tid){
        return EDEADLK;
    }

    mrthread* thread_to_join = get_node(&threads, tid);
    if(!thread_to_join){
        return ESRCH;
    }

    for(int i = 0; i < thread_to_join->wait_count; i++){
        if(thread_to_join->waiting_threads[i] == tid){
            perror("Deadlock");
            return EDEADLK;
        }
    }
    if(thread_to_join->state == TERMINATED){
        unblock_timer();
        return EINVAL;
    }

    thread_to_join->waiting_threads = (int*)realloc(thread_to_join->waiting_threads, (++(thread_to_join->wait_count)) * sizeof(int));
    thread_to_join->waiting_threads[thread_to_join->wait_count - 1] = running_thread->user_tid;
    running_thread->state = WAITING;
    unblock_timer();
    while(thread_to_join->state != TERMINATED);
    block_timer();
    if(retval){
        *retval = thread_to_join->return_value;
    }
    unblock_timer();
    return 0;
}