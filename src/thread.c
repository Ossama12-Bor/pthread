#include "thread.h"
#include "queue.h"
#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>


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
	main_thread->state = IS_RUNNING;
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

    return 0;  // Success
}

thread_t thread_self(void) {
	if(queue == NULL){
		if(initialize_queue(queue)){
			perror("queue initializiation failed");
			return NULL;
		}
	}
    return TAILQ_FIRST(queue)->id;
}

int thread_yield(void) {
	if(queue == NULL){
		if(initialize_queue(queue)){
			perror("queue initializiation failed");
			return EXIT_FAILURE;
		}
	}
	
	struct my_thread * current_thread = TAILQ_FIRST(queue);
	if(current_thread->state == IS_FINISHED){
		TAILQ_REMOVE(queue,current_thread,entries);
		free(current_thread);
		if(move_availale2head(queue)){
			swapcontext(NULL,&(TAILQ_FIRST(queue)->uc));
			return 0;
		}
			
		return 1;
	}
	else {
		current_thread->state = IS_READY;
		ucontext_t uc = current_thread->uc;
		head2tail(queue);
		if(move_availale2head(queue)){
			swapcontext(&uc,&TAILQ_FIRST(queue)->uc);
			return 0;
		}
			
		return 1;
	}
	
    
}

int thread_join(thread_t thread, void **retval) {
	return 0;
}

void thread_exit(void *retval) {
    // Implémentation ici
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
