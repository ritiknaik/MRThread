#include "../headers/mrthread.h"

int init_q(thread_queue *q){
    q->start = NULL;
    q->num = 0;
    q->end = NULL;
    return 0;
}

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

int is_empty_q(thread_queue *q){
    if(q -> start == NULL && q -> end == NULL){
        return 1;
    }
    return 0;
}

// void cleanup(thread_queue *q){
//     node *temp = q->start;
//     node *temp2;
//     for(int i = 0; i < q->num; i++){
//         temp2 = temp->next;
//         free(temp->thread);
//         free(temp);
//         temp = temp2;
//     }    
//     q->num = 0;
//     return;
// }

mrthread* thread_to_sched(thread_queue* q){
    // mrthread* p;
    // p = dequeue_q(q);
    // if(!p){
    //     return NULL;
    // }
    // if(p->state == READY){
    //     return p;
    // }
    // else{
    //     for(int i = 0; i < q->num; i++){
    //         enqueue_q(q, p);
    //         p = dequeue_q(q);
    //         if(p->state == READY){
    //             return p;
    //         }
    //     }
    // }
    // return NULL;
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