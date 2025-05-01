#include "thread.h"
#include "queue.h"
#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <valgrind/valgrind.h>
#include <errno.h>
#include <string.h>
#include <sys/signal.h>
#ifndef USE_PTHREAD

#define THREAD_STACK_SIZE 64*1024

struct my_queue  * queue = NULL;
//struct my_queue * waiting_queue = NULL;

int my_queue_size ;


#ifdef PREEMPTION
 struct itimerval timer;
 volatile sig_atomic_t preempt_flag = 0;

// void timer_handler(int sig) {
//     sigset_t set, old;
//     sigemptyset(&set);
//     sigaddset(&set, SIGPROF);
//     sigprocmask(SIG_BLOCK, &set, &old);
//     thread_yield();
//     sigprocmask(SIG_SETMASK, &old, NULL);
// }
void timer_handler(int sig) {
    preempt_flag = 1;
    sigset_t set, old;
 //   memset(&set, 0, sizeof(sigset_t));
 //   memset(&old, 0, sizeof(sigset_t));
    
    sigemptyset(&set);
    sigaddset(&set, SIGPROF);
    
    if (sigprocmask(SIG_BLOCK, &set, &old) == -1) return;
    
    if (my_queue_size > 1 && !TAILQ_EMPTY(queue)) {
        thread_yield();
    }
    
    sigprocmask(SIG_SETMASK, &old, NULL);
}
#endif



//int deadlock;
//thread_mutex_t locker;

// Function initializer to initialize empty queue


// Fonction pour mettre le main dans la queue


void utils(void *(*func)(void*),void * funcarg){
	struct my_thread * current_thread = TAILQ_FIRST(queue);
	current_thread->retval = func(funcarg);
	#ifdef PREEMPTION
	if (preempt_flag) {
		preempt_flag = 0;
	    thread_yield();
	}
	#endif	
	thread_exit(current_thread->retval);
}


int initialize_queue(){
	//assert(!thread_mutex_init(&locker));
	queue = malloc(sizeof(*queue));
	//waiting_queue  = malloc(sizeof(*waiting_queue));
	TAILQ_INIT(queue);
	//TAILQ_INIT(waiting_queue);
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
    struct sigaction sa = {0};
    sa.sa_handler = timer_handler;
    sa.sa_flags = SA_RESTART | SA_SIGINFO; 
    sigemptyset(&sa.sa_mask);
    sigaction(SIGPROF, &sa, NULL);  
    timer.it_value.tv_usec = 100000;
    timer.it_interval.tv_usec = 1000;
    setitimer(ITIMER_PROF, &timer, NULL);  
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
	free(current_thread);
	free(queue);

}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
  
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
	
    thread_t thr = TAILQ_FIRST(queue);
	
	return thr;
}

int thread_yield(void) {
	#ifdef PREEMPTION
	if (preempt_flag) {
		preempt_flag = 0;
	}
	#endif	
	struct my_thread * current_thread = TAILQ_FIRST(queue);

	if (my_queue_size > 1){
		if (current_thread->state == IS_WAITING){
			TAILQ_REMOVE(queue,current_thread,entries);
			my_queue_size--;
		}
		else {
			head2tail(queue);
		}
		
		struct my_thread  * thread = TAILQ_FIRST(queue);
		if(thread != current_thread)
			swapcontext(&(current_thread->uc),&(thread->uc));
	}
	else if (my_queue_size == 1 && current_thread->state == IS_WAITING){
		return 1;
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
		if(my_queue_size > 2 && thr->state != IS_WAITING){
			TAILQ_REMOVE(queue,thr,entries);
			TAILQ_INSERT_AFTER(queue,current,thr,entries);

		}
		err = thread_yield();
		if(err == 1){
			printf("mal zaml bok\n");
			// thr->wait = NULL;
			current->state =IS_READY;
			return EDEADLK;
		}
			
	}
	current->state = IS_READY;
	thr->wait = NULL;
	
	if(retval != NULL)
		*retval = thr->retval;
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
		TAILQ_INSERT_AFTER(queue,current_thread,waiting ,entries);
		my_queue_size++;
		waiting->state = IS_READY;
	}
	TAILQ_REMOVE(queue,current_thread,entries);
	my_queue_size--;
    if(my_queue_size){
		struct my_thread * thread = TAILQ_FIRST(queue);
		swapcontext(&(current_thread->uc),&(thread->uc));
	}
	TAILQ_INSERT_HEAD(queue,current_thread,entries);
	my_queue_size++;
    exit(0);
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
	struct mutex_thread * first_thread = TAILQ_FIRST(mutex->queue);
	struct my_thread * current_thread = TAILQ_FIRST(queue);
	if(first_thread == NULL){
		struct mutex_thread * thread = malloc(sizeof(struct mutex_thread));
		thread->thread = TAILQ_FIRST(queue);
		TAILQ_INSERT_HEAD(mutex->queue,thread,entries);
		return 0;
	}
	if(current_thread != first_thread->thread){
		struct mutex_thread * thread = malloc(sizeof(struct mutex_thread));
		thread->thread = current_thread;
		TAILQ_INSERT_TAIL(mutex->queue,thread,entries);
		thread->thread->state = IS_WAITING;
		TAILQ_REMOVE(queue,first_thread->thread,entries);
		TAILQ_INSERT_AFTER(queue,current_thread,first_thread->thread,entries);
		thread_yield();
	}
	return 0;
}

int thread_mutex_unlock(thread_mutex_t *mutex) {
	struct mutex_thread * first = TAILQ_FIRST(mutex->queue);
    TAILQ_REMOVE(mutex->queue,first,entries);
	free(first);
	first = TAILQ_FIRST(mutex->queue);
	if(first != NULL){
		first->thread->state = IS_READY;
		TAILQ_INSERT_AFTER(queue,TAILQ_FIRST(queue),first->thread,entries);
		my_queue_size++;
	}
		
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

