#include "../headers/mrthread.h"

//function to initialize the queue
int init_q(thread_queue *q){
    q->start = NULL;
    q->num = 0;
    q->end = NULL;
    return 0;
}

//function to enqueue a thread to the global queue
void enqueue_q(thread_queue *q, mrthread *t){
    node *temp = (node*)malloc(sizeof(node));
    temp->thread = t;
    temp->next = NULL;
    
    if(q->end == NULL){
       q->end = temp;
       q->start = temp;
    }
    else{
        q->end->next = temp;
        q->end = temp;
    }
    q->num += 1;
}

//function to dequeue a thread from global queue
mrthread* dequeue_q(thread_queue *q){
    if(is_empty_q(q)){
        return NULL;
    }
    mrthread *t;
    node *temp;
    temp = q->start;
    t = temp->thread;
    q->start = q->start->next;
    if(q->start == NULL){
        q->end = NULL;
    }
    q->num--;
    free(temp);
    return t;
}

//funcction to get a node from tid
mrthread* get_node(thread_queue *q, int tid){
    if(is_empty_q(q))
        return NULL;
    node *temp = q->start;
    while(temp){
        if(temp->thread->user_tid == tid)
            return temp->thread;
        temp = temp->next;
    }
    return NULL;
}

//function to check if queue is empty
int is_empty_q(thread_queue *q){
    if(q -> start == NULL && q -> end == NULL){
        return 1;
    }
    return 0;
}


//function to find a ready thread from the queue
//to schedule it by the scheduler
mrthread* thread_to_sched(thread_queue* q){
    mrthread* p;
    for(int i = 0; i < (q->num); i++){
        p = dequeue_q(q);
        if(p->state == READY){
            return p;
        }
        enqueue_q(q, p);
    }
    return NULL;
}