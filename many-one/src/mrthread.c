#include "../headers/mrthread.h"

// jmp_buf env;
mrthread* running_thread;
thread_queue threads;
int num_thread = 0;

//function to block timer interrupt from SIGVTALRM
void block_timer(){
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &set, NULL);
}

//function to unblock timer interrupt from SIGVTALRM
void unblock_timer(){
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
}

//function to allocate stack to the thread
void* allocate_stack(size_t size){
    void* stack = NULL;
    stack = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED){
        perror("Stack Allocation");
        return NULL;
    }
    return stack;
}

//function to initialize the library and timer
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
    main->waiting_threads = NULL;
    main->user_tid = num_thread;
    main->state = RUNNING;
    main->wait_count = 0;
    main->stack_size = STACK_SIZE;
    
    sigemptyset(&(main->signal_set));
    running_thread = main;
    num_thread++;
    timer_init();
}

//function to set and initialize timer
void timer_init(){
    sigset_t mask;
    sigfillset(&mask);
    
    struct sigaction timer_handler;
    memset(&timer_handler, 0, sizeof(timer_handler));
    timer_handler.sa_handler = alarm_handle;
	timer_handler.sa_flags = SA_RESTART;
	timer_handler.sa_mask = mask;
   
    sigaction(SIGVTALRM, &timer_handler, NULL);  
    struct itimerval clock;
    clock.it_interval.tv_sec = 0 ;
    clock.it_interval.tv_usec = 100;
    clock.it_value.tv_sec = 0;
    clock.it_value.tv_usec = 100;
    //start timer
	setitimer(ITIMER_VIRTUAL, &clock, 0);
}

//raise the signals that the thread recieved
//while it was in the queue and not running
void raise_pending_signals(){
    sigset_t to_raise;
    sigfillset(&to_raise);
    sigdelset(&to_raise, SIGVTALRM);
    sigdelset(&to_raise, SIGALRM);
    sigprocmask(SIG_UNBLOCK, &to_raise, NULL);
    for(int j = 0; j < NSIG; j++){
        if(sigismember(&(running_thread->signal_set), j)){
            raise(j);
            sigdelset(&(running_thread->signal_set), j);
        }
    }
    return;
}

//SIGVTALRM handler
void alarm_handle(){
    scheduler();
}

//function to schedule threads when timer interrupt
//occurs or the thread exits
void scheduler(){
    if(setjmp(running_thread->context) == 0){
        block_timer();
        if(running_thread->state != TERMINATED){
            running_thread->state = READY;
        }
        enqueue_q(&threads, running_thread);
        running_thread = thread_to_sched(&threads);
        if(!running_thread){
            exit(0);
        }
        running_thread->state = RUNNING;
        longjmp(running_thread->context, 1);
    }
    else{
        raise_pending_signals();
        unblock_timer();
        return;
    }
}

//wrapper over the function call
void routine_wrapper(void* farg){
    unblock_timer();
    
    running_thread->return_value = running_thread->f(running_thread->arg);
    thread_exit(running_thread->return_value);
    return;
}

//function to set the context of jmpbuf
//RSP stack pointer, RBP base pointer, PC instruction pointer
void set_context(mrthread* thread){
    setjmp(thread->context);
    thread->context[0].__jmpbuf[JB_RSP] = mangle((long int) thread->stack + STACK_SIZE - sizeof(long int));
    thread->context[0].__jmpbuf[JB_RBP] = thread->context[0].__jmpbuf[JB_RSP];
	thread->context[0].__jmpbuf[JB_PC] = mangle((long int) routine_wrapper);
}

//function to create thread in many-one fashion
int thread_create(int* tid, void *(*f) (void *), void *arg){
    block_timer();
    if(tid == NULL || f == NULL){
        unblock_timer();
        return EINVAL;
    }
    static int initial = 0;
    if(!initial){
        init();
        initial = 1;
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
    thread->wait_count = 0;
    thread->state = READY;
    thread->waiting_threads = NULL;
    thread->user_tid = num_thread;
    thread->return_value = NULL;
    sigemptyset(&(thread->signal_set));

    set_context(thread);
    *tid = thread->user_tid;
    enqueue_q(&threads, thread);
    num_thread++;
    unblock_timer();
    return 0;
}

//function to join a thread in many-one fashion
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
        return ESRCH;
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

//function for thread exit in many-one fashion
void thread_exit(void *retval){
    if(retval == NULL){
        return;
    }
    block_timer();
    running_thread->state = TERMINATED;
    running_thread->return_value = retval;
    for(int i = 0; i < running_thread->wait_count; i++){
        mrthread* t = get_node(&threads, running_thread->waiting_threads[i]);
        t->state = READY;
    }
    unblock_timer();
    scheduler();
}

//function for thread kill in many-one fashion
int thread_kill(mrthread_t tid, int sign){
    block_timer();
    if(sign < 0 || sign > 64){
        unblock_timer();
        return EINVAL;
    }
    if(running_thread->user_tid == tid){
        raise(sign);
        unblock_timer();
        return 0;
    }
    if(sign == SIGSTOP || sign == SIGCONT || sign == SIGINT){
        kill(getpid(), sign);
    }
    else{
        mrthread* thread_to_signal = get_node(&threads, tid);
        if(!thread_to_signal){
            unblock_timer();
            return ESRCH;
        }
        sigaddset(&thread_to_signal->signal_set, sign);
    }
    unblock_timer();
    return 0;
}