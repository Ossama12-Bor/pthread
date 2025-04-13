#ifndef LIST_HANDLER__H
#define LIST_HANDLER__H
#include "queue.h"
#include <ucontext.h>
#include <stdlib.h>
typedef void * thread_t;
extern thread_t  main_id_thread;
enum states{
    IS_READY, IS_FINISHED, IS_ZOMBIE, IS_RUNNING, IS_BLOCKED, IS_WAITING  
};




struct my_thread {
    ucontext_t uc;
    enum states state;
    int valgrind_stackid;
    thread_t wait ;
    TAILQ_ENTRY(my_thread) entries;
    void * retval;
};

TAILQ_HEAD(my_queue, my_thread);

void  head2tail(struct my_queue * queue);




int move_availale2head(struct my_queue * queue); // return 0 if no elem available 

void queue_free(struct my_queue **queue);

#endif