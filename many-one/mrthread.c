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
    sigemptyset(&(main->signal_set));
    running_thread = main;
    num_thread++;
    timer_init();
    //printf("done init\n");
}

void timer_init(){
    sigset_t mask;
    sigfillset(&mask);
    
    struct sigaction timer_handler;
    memset(&timer_handler, 0, sizeof(timer_handler));
    timer_handler.sa_handler = alarm_handle;
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
    //printf("setted timer\n");
}

void raise_pending_signals(){
    sigset_t to_raise;
    sigfillset(&to_raise);
    sigdelset(&to_raise, SIGALRM);
    sigprocmask(SIG_UNBLOCK, &to_raise, NULL);
    for(int j = 0; j < NSIG; j++){
        if(sigismember(&(running_thread->signal_set), j)){
            //printf("raised signal %d\n", j);
            raise(j);
            //printf("came back\n");
            sigdelset(&(running_thread->signal_set), j);
        }
    }
    return;
}

void alarm_handle(){
    //printf("alarm\n");
    scheduler();
}

void scheduler(){
    //printf("scheduler called\n");
    // block_timer();
    // int a;
    if(setjmp(running_thread->context) == 0){
        block_timer();
        if(running_thread->state != TERMINATED){
            running_thread->state = READY;
        }
        //printf("before enqueue\n");
        enqueue_q(&threads, running_thread);
        //printf("before thread to sched\n");
        running_thread = thread_to_sched(&threads);
        if(!running_thread){
            exit(0);
        }
        running_thread->state = RUNNING;
        //printf("before longjmp\n");
        //printf("tid of new: %d\n", running_thread->user_tid);
        //printf("jmpbuf: %s\n", running_thread->context);
        longjmp(running_thread->context, 1);
    }
    else{
        // unblock_timer();
        raise_pending_signals();
        unblock_timer();
        return;
    }
}

void wrapper(void* farg){
    //printf("wrapper called\n");
    unblock_timer();
    //printf("before return\n");
    running_thread->return_value = running_thread->f(running_thread->arg);
    //printf("*(int*)retval in wrapper %d\n", *(int*)running_thread->return_value);
    //printf("retval in wrapper%d\n", running_thread->return_value);
    //printf("before exit\n");
    // running_thread->state = TERMINATED;
    // // running_thread->return_value = retval;
    // // //printf("retval in exit %d\n", *(int*)running_thread->return_value);
    // for(int i = 0; i < running_thread->wait_count; i++){
    //     mrthread* t = get_node(&threads, running_thread->waiting_threads[i]);
    //     t->state = READY;
    //     //printf("wait to ready done\n");
    // }
    // scheduler();
    thread_exit(running_thread->return_value);

    //printf("*(int*)retval in wrapper after thread exit %d\n", *(int*)running_thread->return_value);
    return;
}

void set_context(mrthread* thread){
    setjmp(thread->context);
    thread->context[0].__jmpbuf[JB_RSP] = mangle((long int) thread->stack + STACK_SIZE - sizeof(long int));
    thread->context[0].__jmpbuf[JB_RBP] = thread->context[0].__jmpbuf[JB_RSP];
	thread->context[0].__jmpbuf[JB_PC] = mangle((long int) wrapper);
}

int thread_create(int* tid, void *(*f) (void *), void *arg){
    //printf("create thread called\n");
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
    thread->state = READY;
    thread->waiting_threads = NULL;
    thread->user_tid = num_thread;
    thread->wait_count = 0;
    thread->return_value = NULL;
    sigemptyset(&(thread->signal_set));

    set_context(thread);
    //printf("create jmpbuf: %s, tid: %d\n", thread->context, thread->user_tid);
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
    //printf("entered join\n");
    mrthread* thread_to_join = get_node(&threads, tid);
    if(!thread_to_join){
        //printf("node not ofund\n");
        return ESRCH;
    }
    //printf("before deadlock\n");
    for(int i = 0; i < thread_to_join->wait_count; i++){
        if(thread_to_join->waiting_threads[i] == tid){
            perror("Deadlock");
            return EDEADLK;
        }
    }
    if(thread_to_join->state == TERMINATED){
        unblock_timer();
        //printf("terminated before joining\n");
        return ESRCH;
    }
    //printf("before assigning\n");
    thread_to_join->waiting_threads = (int*)realloc(thread_to_join->waiting_threads, (++(thread_to_join->wait_count)) * sizeof(int));
    //printf("after realloc\n");
    thread_to_join->waiting_threads[thread_to_join->wait_count - 1] = running_thread->user_tid;
    //printf("after wait\n");
    running_thread->state = WAITING;
    //printf("after waiting\n");
    unblock_timer();
    //printf("before loop\n");
    while(thread_to_join->state != TERMINATED);
    //printf("before blocktimer\n");
    block_timer();
    if(retval){

        *retval = thread_to_join->return_value;
        //printf("*(int*)retval in join %d\n", *(int*)thread_to_join->return_value);
    }
    unblock_timer();
    return 0;
}

void thread_exit(void *retval){
    if(retval == NULL){
        return;
    }
    block_timer();
    //printf("return value in exit%d\n", *(int*)retval);
    running_thread->state = TERMINATED;
    running_thread->return_value = retval;
    // //printf("retval in exit %d\n", *(int*)running_thread->return_value);
    for(int i = 0; i < running_thread->wait_count; i++){
        mrthread* t = get_node(&threads, running_thread->waiting_threads[i]);
        t->state = READY;
        //printf("wait to ready done\n");
    }
    unblock_timer();
    //printf("after exit\n");
    scheduler();
}

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
            //printf("node not found\n");
            unblock_timer();
            return ESRCH;
        }
        sigaddset(&thread_to_signal->signal_set, sign);
    }
    //printf("added signal\n");
    unblock_timer();
    return 0;
}