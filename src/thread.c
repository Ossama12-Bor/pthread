#include "thread.h"
#include "queue.h"
#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifndef USE_PTHREAD

struct my_queue  * queue = NULL;



// Function initializer to initialize empty queue


// Fonction pour mettre le main dans la queue

int initialize_queue(){
	queue = malloc(sizeof(*queue));
	TAILQ_INIT(queue);
	struct my_thread * main_thread = malloc(sizeof(struct my_thread));
	main_thread->id = (thread_t)1;
	getcontext(&main_thread->uc);
	main_thread->state = IS_READY;
	TAILQ_INSERT_HEAD(queue,main_thread,entries);
	return 0;



}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
    // Allocate memory for the new thread
	if(queue == NULL){
		if(initialize_queue(queue)){
			perror("queue initializiation failed");
			return EXIT_FAILURE;
		}
	}

    struct my_thread *new_item = (struct my_thread *)malloc(sizeof(struct my_thread));
    if (new_item == NULL) {
        perror("Failed to allocate memory for new thread");
        return -1;  // Return failure if memory allocation fails
    }

    
    new_item->state = IS_READY;

    
    if (getcontext(&new_item->uc) == -1) {
        perror("getcontext failed");
        free(new_item);  
        return -1;
    }

    
    new_item->uc.uc_link = NULL;  
    new_item->uc.uc_stack.ss_sp = malloc(64*1024);  
    new_item->uc.uc_stack.ss_size = 64*1024;
    new_item->uc.uc_stack.ss_flags = 0;    
	if (new_item->uc.uc_stack.ss_sp == NULL) {
        perror("Failed to allocate stack for new thread");
        free(new_item);  
        return -1;
    }

    makecontext(&new_item->uc, (void (*)(void))func, 1, funcarg);

    
    TAILQ_INSERT_TAIL(queue, new_item, entries);

  
    *newthread = (thread_t)new_item;
	new_item->id = *newthread;

    return 0;  // Success
}

thread_t thread_self(void) {
	if(queue == NULL){
		if(initialize_queue(queue)){
			perror("queue initializiation failed");
			return NULL;
		}
	}
    thread_t thr = TAILQ_FIRST(queue)->id;
	if(queue_size(queue)==1){
		queue_free(&queue);

	}
	return thr;
}

int thread_yield(void) {
	if(queue == NULL){
		if(initialize_queue(queue)){
			perror("queue initializiation failed");
			return EXIT_FAILURE;
		}
	}

	
	
	struct my_thread * current_thread = TAILQ_FIRST(queue);

	head2tail(queue);
		if(!move_availale2head(queue)){
			perror("move failed");
			return EXIT_FAILURE;
		}
	swapcontext(&current_thread->uc,&TAILQ_FIRST(queue)->uc);
	if(queue_size(queue)==1){
		queue_free(&queue);
	}

	return 0;
    
}

int thread_join(thread_t thread, void **retval) {
	int err;
	struct my_thread * thr = find(queue,thread);
	assert(thr != NULL);
	if(retval != NULL)
		*retval = thr->retval;
	while(thr->state != IS_FINISHED){
		err = thread_yield();
	assert(!err);
	}
	if(thr->state ==IS_FINISHED){
		TAILQ_REMOVE(queue,thr,entries);
		free(thr->uc.uc_stack.ss_sp);
		free(thr);
		err = thread_yield();
	}
	return err;

}

void thread_exit(void *retval) {
    struct my_thread * current_thread = TAILQ_FIRST(queue);
	current_thread->state = IS_FINISHED;
	current_thread->retval = retval;
    assert(!thread_yield());

}

int thread_mutex_init(thread_mutex_t *mutex) {
    // Implémentation ici
	return 0;
}

int thread_mutex_destroy(thread_mutex_t *mutex) {
    // Implémentation ici
	return 0;
}

int thread_mutex_lock(thread_mutex_t *mutex) {
    // Implémentation ici
	return 0;
}

int thread_mutex_unlock(thread_mutex_t *mutex) {
    // Implémentation ici
	return 0;
}

#endif /* USE_PTHREAD */
