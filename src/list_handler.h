#ifndef LIST_HANDLER__H
#define LIST_HANDLER__H
#include "queue.h"
#include <ucontext.h>
typedef void * thread_t;

enum states{
    IS_READY, IS_FINISHED, IS_ZOMBIE, IS_RUNNING, IS_BLOCKED, IS_WAITING  
};




struct my_thread {
	thread_t id;
    ucontext_t uc;
    enum states state;
    TAILQ_ENTRY(my_thread) entries;
};

TAILQ_HEAD(my_queue, my_thread);

void  head2tail(struct my_queue * queue);


struct my_thread *  first_available(struct my_queue * queue );

int move_availale2head(struct my_queue * queue); // return 0 if no elem available 
#endif