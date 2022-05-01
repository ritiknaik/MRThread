#include "../headers/mrthread.h"

//function to initialize global linked list
int initll(threadll* ll){
    if (!ll)
        return -1;
    ll->start = ll->end = NULL;
    return 0;
}

//function to insert to global linked list
node* insertll(threadll* ll, mrthread* t){
    node* temp;
    temp = (node*)malloc(sizeof(node));
    if(temp == NULL){
        perror("malloc failed");
        return NULL;
    }
    temp->next = NULL;
    temp->thread = t;
    if(ll->start == NULL){
        ll->start = ll->end = temp;
    }
    else{
        ll->end->next = temp;
        ll->end = temp;
    }
    return temp;
}

//function to delete a thread from linked list
int deletell(threadll* ll, mrthread_t tid){
    node* tmp = ll->start;
    if(tmp == NULL){
        return -1;
    }
    if(tmp->thread->kernel_tid == tid){
        ll->start = ll->start->next;
        if(tmp->thread->stack){
            if(munmap(tmp->thread->stack, STACK_SIZE)){
                return errno;
            }
        }
        free(tmp->thread);
        free(tmp);
        if(ll->start == NULL){
            ll->end = ll->start;
        }
        return 0;
    }
    while(tmp->next){
        if(tmp->next->thread->kernel_tid == tid){
            node* tn = tmp->next->next;
            if(tmp->next == ll->end){
                ll->end = tmp;
            }
            free(tmp->next->thread);
            free(tmp->next);
            tmp->next = tn;
            break;
        }
        tmp = tmp->next;
    }
    return 0;
}

// function to find the node from given tid
node* get_node(threadll* ll, mrthread_t tid){
    node* p = ll->start;
    while(p){
        if(p->thread->kernel_tid == tid){
            return p;
        }
        p = p->next;
    }
    return NULL;
}