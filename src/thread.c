#include "thread.h"
#include "queue.h"
#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <valgrind/valgrind.h>
#include <errno.h>
#include <sys/signal.h>
#ifndef USE_PTHREAD

#define THREAD_STACK_SIZE 64*1024

struct my_queue  * queue = NULL;
int my_queue_size ;
#ifdef PREEMPTION
struct itimerval timer;
#endif

void timer_handler(int sig) {
	(void) sig;
	thread_yield();
}


//int deadlock;
//thread_mutex_t locker;

// Function initializer to initialize empty queue


// Fonction pour mettre le main dans la queue


void utils(void *(*func)(void*),void * funcarg){
	struct my_thread * current_thread = TAILQ_FIRST(queue);
	current_thread->retval = func(funcarg);
	struct my_thread * waiting = current_thread->wait;
	current_thread->state = IS_FINISHED;
	if(waiting  != NULL){
		if(my_queue_size > 2){
			TAILQ_REMOVE(queue,waiting ,entries);
			TAILQ_INSERT_AFTER(queue,current_thread,waiting ,entries);
		}
		waiting->state = IS_READY;
	}
	if(my_queue_size!=1){
		assert(!thread_yield());
	}
}


int initialize_queue(){
	//assert(!thread_mutex_init(&locker));
	queue = malloc(sizeof(*queue));
	TAILQ_INIT(queue);
	struct my_thread * main_thread = malloc(sizeof(struct my_thread));
	//main_thread->id = MAIN_THRAD_ID;
	getcontext(&main_thread->uc);
	main_thread->state = IS_READY;
	//main_thread->locker =0;
	TAILQ_INSERT_HEAD(queue,main_thread,entries);
	my_queue_size = 1;
	main_id_thread = (thread_t)main_thread;
	main_thread->wait = NULL;
	#ifdef PREEMPTION
	signal(SIGALRM, timer_handler);
    timer.it_value.tv_sec = 0;      
    timer.it_value.tv_usec = 10;     
    timer.it_interval.tv_sec = 0;   
    timer.it_interval.tv_usec = 100;  
    setitimer(ITIMER_REAL, &timer, NULL);
	#endif
	return 0;



}

__attribute__((constructor))
void my_constructor() {
    initialize_queue();
}

__attribute__((destructor))
void my_destructor() {
	struct my_thread * current_thread = TAILQ_FIRST(queue);
	if(current_thread != main_id_thread){
		free(current_thread);
		free(queue);
	}
    else 
		queue_free(&queue);
	//assert(!thread_mutex_destroy(&locker));
}


int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
    // Allocate memory for the new thread
	/*if(queue == NULL){
		if(initialize_queue()){
			perror("queue initializiation failed");
			return EXIT_FAILURE;
		}
	}*/

    struct my_thread *new_item = (struct my_thread *)malloc(sizeof(*new_item));
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
    new_item->uc.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE);
    new_item->uc.uc_stack.ss_size = THREAD_STACK_SIZE;
    new_item->uc.uc_stack.ss_flags = 0;
	new_item->valgrind_stackid = VALGRIND_STACK_REGISTER(new_item->uc.uc_stack.ss_sp\
					,new_item->uc.uc_stack.ss_sp + new_item->uc.uc_stack.ss_size);    
	if (new_item->uc.uc_stack.ss_sp == NULL) {
        perror("Failed to allocate stack for new thread");
        free(new_item);  
        return -1;
    }

    makecontext(&new_item->uc, (void (*)(void))utils, 2, func , funcarg);

    
    TAILQ_INSERT_AFTER(queue,TAILQ_FIRST(queue),new_item, entries);

  
    *newthread = (thread_t)new_item;
	//new_item->id = *newthread;

	my_queue_size++;
	new_item->wait = NULL;

    return 0;  // Success
}

thread_t thread_self(void) {
	/*if(queue == NULL){
		if(initialize_queue()){
			perror("queue initializiation failed");
			return NULL;
		}
	}*/
    thread_t thr = TAILQ_FIRST(queue);
	/*if(my_queue_size==1){
		queue_free(&queue);

	}*/
	return thr;
}

int thread_yield(void) {
	/*
	if(queue == NULL){
		if(initialize_queue()){
			perror("queue initializiation failed");
			return EXIT_FAILURE;
		}
	}
	if(my_queue_size==1 && TAILQ_FIRST(queue)==main_id_thread){
		queue_free(&queue);
		return 0;
	}
	*/
	
	struct my_thread * current_thread = TAILQ_FIRST(queue);

	if (my_queue_size > 1){
		head2tail(queue);
			if(!move_availale2head(queue)){
				struct my_thread * last = TAILQ_LAST(queue,my_queue);
				TAILQ_REMOVE(queue,last,entries);
				TAILQ_INSERT_HEAD(queue,last,entries);
				return 1;
			}
		struct my_thread  * thread = TAILQ_FIRST(queue);
		if(thread != current_thread)
			swapcontext(&(current_thread->uc),&(thread->uc));
	}



	return 0;
    
}

int thread_join(thread_t thread, void **retval) {
	int err;
	struct my_thread * current  = TAILQ_FIRST(queue);
	struct my_thread * thr = thread;
	assert(thr != NULL);
	/*if(thr->wait == current){
		return EDEADLK;
	}*/
	thr->wait = current;
	current->state =IS_WAITING;
	if(thr->state != IS_FINISHED){
		if(my_queue_size > 2){
			TAILQ_REMOVE(queue,thr,entries);
			TAILQ_INSERT_AFTER(queue,current,thr,entries);
		}
		err = thread_yield();
		if(err == 1){
			thr->wait = NULL;
			current->state =IS_READY;
			return EDEADLK;
		}
			
	}
	thr->wait = NULL;
	
	if(retval != NULL)
		*retval = thr->retval;
	TAILQ_REMOVE(queue,thr,entries);
	my_queue_size--;
	if(thread!= main_id_thread){
		VALGRIND_STACK_DEREGISTER(thr->valgrind_stackid);
		free(thr->uc.uc_stack.ss_sp);
	}
	free(thr);
	return 0;

}

void thread_exit(void *retval ) {
    struct my_thread * current_thread = TAILQ_FIRST(queue);
	struct my_thread * waiting = current_thread->wait;
	current_thread->state = IS_FINISHED;
	current_thread->retval = retval;
	if(waiting  != NULL){
		if(my_queue_size > 2){
			TAILQ_REMOVE(queue,waiting ,entries);
			TAILQ_INSERT_AFTER(queue,current_thread,waiting ,entries);
		}
		waiting->state = IS_READY;
	}
    assert(!thread_yield());
    abort();
}

int thread_mutex_init(thread_mutex_t *mutex) {
	mutex->queue = malloc(sizeof(struct mutex_queue));
	TAILQ_INIT(mutex->queue);
	if(mutex->queue == NULL){
		perror("hadi ghir zidta\n");
		return 1;
	}
	return 0;
}

int thread_mutex_destroy(thread_mutex_t *mutex) {
    free(mutex->queue);
	return 0;
}

int thread_mutex_lock(thread_mutex_t *mutex) {
	struct mutex_thread * thread = malloc(sizeof(struct mutex_thread));
	thread->thread = thread_self();
	TAILQ_INSERT_TAIL(mutex->queue,thread,entries);
	thread->thread->state = IS_BLOCKED;
	if(thread != TAILQ_FIRST(mutex->queue)){
		thread_yield();
	}
		
	thread->thread->state = IS_READY;
	return 0;
}

int thread_mutex_unlock(thread_mutex_t *mutex) {
	struct mutex_thread * first = TAILQ_FIRST(mutex->queue);
    TAILQ_REMOVE(mutex->queue,first,entries);
	free(first);
	first = TAILQ_FIRST(mutex->queue);
	if(first != NULL)
		first->thread->state = IS_READY;
	return 0;
}

int thread_sem_init(thread_sem_t *sem,int pshared __attribute__((unused)),int value){
	if (value < 0 || sem == NULL) {
		return -1;
	}

	sem->dummy = value;
	return 0;
}

int thread_sem_destroy(thread_sem_t *sem){
	sem->dummy = -1;
	return 0;
}

int thread_sem_wait(thread_sem_t *sem){
	while(sem->dummy <= 0){
		thread_yield();
	}
	sem->dummy = sem->dummy -1;
	return 0;
}

int thread_sem_post(thread_sem_t *sem){
	sem->dummy = sem->dummy + 1;
	return 0;
}

int thread_sem_trywait(thread_sem_t *sem){
	if(sem->dummy <= 0){
		printf("sem->dummy <= 0\n");
		thread_yield();
		return -1;
	}

	sem->dummy -= 1;
	return 0;
}

#endif /* USE_PTHREAD */

