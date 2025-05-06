#ifndef LIST_HANDLER__H
#define LIST_HANDLER__H
#include "queue.h"
#include <ucontext.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_SIGNALS 2
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
    uint32_t pending_signals;            // Bitmask of pending signals
    uint32_t blocked_signals;            // Bitmask of blocked signals
    void (*handlers[MAX_SIGNALS])(int);  // Array of signal handlers
};

struct mutex_thread {
	struct my_thread *thread;
	TAILQ_ENTRY(mutex_thread) entries;

};
TAILQ_HEAD(my_queue, my_thread);
TAILQ_HEAD(mutex_queue,mutex_thread);

void  head2tail(struct my_queue * queue);




int move_availale2head(struct my_queue * queue); // return 0 if no elem available 

void queue_free(struct my_queue **queue);

#endif