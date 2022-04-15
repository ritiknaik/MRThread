#include"mrthread.h"
#include<errno.h>
#include<sys/mman.h>

//initialize global linked list
int initll(threadll* ll){
    if (!ll)
        return -1;
    ll->head = ll->tail = NULL;
    return 0;
}

//insert to global linked list
node* insertll(threadll* ll, mrthread* t){
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

int deletell(threadll* ll, mrthread_t tid){
    //printf("inside deletell\n");
    node* tmp = ll->head;
    if(tmp == NULL){
        return -1;
    }
    if(tmp->thread->kernel_tid == tid){
        ll->head = ll->head->next;
        if(tmp->thread->stack){
            //printf("before unmap\n");
            if(munmap(tmp->thread->stack, STACK_SIZE)){
                return errno;
            }
        }
        //printf("after unmap\n");
        free(tmp->thread);
        free(tmp);
        if(ll->head == NULL){
            ll->tail = ll->head;
        }
        return 0;
    }
    while(tmp->next){
        if(tmp->next->thread->kernel_tid == tid){
            node* tn = tmp->next->next;
            if(tmp->next == ll->tail){
                ll->tail = tmp;
            }
            // if (tmp->next->thread->stack)
            // {
            //     if (munmap(tmp->next->thread->stack, STACK_SIZE))
            //     {
            //         return errno;
            //     }
            // }
            free(tmp->next->thread);
            free(tmp->next);
            tmp->next = tn;
            break;
        }
        tmp = tmp->next;
    }
    return 0;
}

node* get_node(threadll* ll, mrthread_t tid){
    node* p = ll->head;
    while(p){
        if(p->thread->kernel_tid == tid){
            return p;
        }
        p = p->next;
    }
    return NULL;
}