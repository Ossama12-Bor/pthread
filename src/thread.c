#include "thread.h"
#include "queue.h"
#include <ucontext.h>

#ifndef USE_PTHREAD

struct my_thread {
    thread_t id;
    ucontext_t uc;
    enum states state;
};

enum states{
    IS_READY, IS_FINISHED, IS_ZOMBIE, IS_RUNNING, IS_BLOCKED, IS_WAITING  
};

struct item {
    struct my_thread Mythread;
    TAILQ_ENTRY(item) entries;
}

// Function initializer to initialize empty queue
TAILQ_HEAD(my_queue, item) queue;
TAILQ_INIT(&queue);

// Fonction pour mettre le main dans la queue

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
    // Allocate memory for the new thread
    struct item *new_item = (struct item *)malloc(sizeof(struct item));
    if (new_item == NULL) {
        perror("Failed to allocate memory for new thread");
        return -1;  // Return failure if memory allocation fails
    }

    
    new_item->Mythread.state = IS_READY;

    
    if (getcontext(&new_item->Mythread.uc) == -1) {
        perror("getcontext failed");
        free(new_item);  
        return -1;
    }

    
    new_item->Mythread.uc.uc_link = NULL;  
    new_item->Mythread.uc.uc_stack.ss_sp = malloc(64*1024);  
    new_item->Mythread.uc.uc_stack.ss_size = 64*1024;
    new_item->Mythread.uc.uc_stack.ss_flags = 0;

    if (new_item->Mythread.uc.uc_stack.ss_sp == NULL) {
        perror("Failed to allocate stack for new thread");
        free(new_item);  
        return -1;
    }

    makecontext(&new_item->Mythread.uc, (void (*)(void))func, 1, funcarg);

    
    TAILQ_INSERT_TAIL(&queue, new_item, entries);

  
    *newthread = (thread_t)new_item;

    return 0;  // Success
}

thread_t thread_self(void) {
    // Assert queue non nulle
    if (TAILQ_EMPTY(&queue)) {
        return NULL;
    }
    return TAILQ_FIRST(&queue)->Mythread->id;
}

int thread_yield(void) {
    
}

int thread_join(thread_t thread, void **retval) {
    // Implémentation ici
}

void thread_exit(void *retval) {
    // Implémentation ici
}

int thread_mutex_init(thread_mutex_t *mutex) {
    // Implémentation ici
}

int thread_mutex_destroy(thread_mutex_t *mutex) {
    // Implémentation ici
}

int thread_mutex_lock(thread_mutex_t *mutex) {
    // Implémentation ici
}

int thread_mutex_unlock(thread_mutex_t *mutex) {
    // Implémentation ici
}

#endif /* USE_PTHREAD */
