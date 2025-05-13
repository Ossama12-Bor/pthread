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
#include <stdint.h>
#include <signal.h>

#define MAX_SIGNALS 2
#define THREAD_SIG_TERM 0   // Terminate thread signal
#define THREAD_SIG_USR1 1   // User-defined signal
#ifndef USE_PTHREAD

#define THREAD_STACK_SIZE 64*1024

struct my_queue  * queue = NULL;
//struct my_queue * waiting_queue = NULL;

int my_queue_size ;

stack_t alt_stack;

#ifndef PREEMPTION
void signal_handler(int sig) {
    if (sig == SIGSEGV) {
        pthread_exit(NULL);
    }
}
#endif

#ifdef PREEMPTION
static struct sigaction sa;
static struct itimerval timer;
static int preemption_initialized = 0;

void desactivate_preemption(void) {
    if (preemption_initialized)
        sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL);
}

void activate_preemption(void) {
    if (preemption_initialized)
        sigprocmask(SIG_UNBLOCK, &sa.sa_mask, NULL); 
}

void handler(int sig) {
    // struct timeval tv1, tv2;
    // unsigned long us;

    // gettimeofday(&tv1, NULL);
  desactivate_preemption();
    if (sig == SIGVTALRM && my_queue_size > 1) {
        thread_yield();
    }
    activate_preemption();
    // gettimeofday(&tv2, NULL);
	// us = (tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec);

    // printf("Programme exécuté en %ld us\n", us);


}

void begin_preemption(void) {
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGVTALRM);
  sa.sa_flags = 0;
  sigaction(SIGVTALRM, &sa, NULL);
  desactivate_preemption();

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 1000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 10000;

    setitimer(ITIMER_VIRTUAL, &timer, NULL);

    preemption_initialized = 1;  
}


#endif



void setup_signal_stack() {
    alt_stack.ss_sp = malloc(SIGSTKSZ);  // Allouer un espace mémoire pour la pile alternative
    if (alt_stack.ss_sp == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    alt_stack.ss_size = SIGSTKSZ;
	alt_stack.ss_flags = 0;
    if (sigaltstack(&alt_stack, NULL) == -1) {
        perror("sigaltstack");
        exit(EXIT_FAILURE);
    }
}


void utils(void *(*func)(void*), void *funcarg) {
#ifdef	PREEMPTION
  activate_preemption();
#endif  
  
    struct my_thread *current_thread = TAILQ_FIRST(queue);
    current_thread->retval = func(funcarg);
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
    begin_preemption();
	#endif

	return 0;



}

__attribute__((constructor))
void my_constructor() {
    initialize_queue();
	setup_signal_stack();
}



__attribute__((destructor))
void my_destructor() {
	struct my_thread * current_thread = TAILQ_FIRST(queue);
	free(current_thread);
	free(queue);

}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
	#ifdef PREEMPTION
	desactivate_preemption();  
	#endif
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
#ifdef	PREEMPTION
  activate_preemption();
#endif  

    return 0;  // Success
}


thread_t thread_self(void) {
	
    thread_t thr = TAILQ_FIRST(queue);
	
	return thr;
}

int thread_yield(void) {

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
		if(thread != current_thread) {
	/*		#ifdef PREEMPTION
    desactivate_preemption();
    #endif*/
			swapcontext(&(current_thread->uc),&(thread->uc));
		
	#ifdef PREEMPTION
		activate_preemption();
	#endif
		}
	}
	else if (my_queue_size == 1 && current_thread->state == IS_WAITING){
		return 1;
	}
	return 0;
    
}

int thread_join(thread_t thread, void **retval) {
	#ifdef PREEMPTION
	desactivate_preemption();
	#endif 

	int err;
	struct my_thread * current  = TAILQ_FIRST(queue);
	struct my_thread * thr = thread;
	if(thr == NULL){
		perror("error: thread joined by two threads");
		return ESRCH;
	}
	if(thr->wait != NULL)
		return EINVAL;
		
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
		#ifdef PREEMPTION
		activate_preemption();
		#endif
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
	#ifdef PREEMPTION
	activate_preemption();
	#endif 

	return 0;

}


void thread_exit(void *retval ) {
	#ifdef PREEMPTION
    desactivate_preemption();
	#endif
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
	#ifdef PREEMPTION
    activate_preemption();
	#endif
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

int thread_sem_init(thread_sem_t *sem, int pshared, int value)
{
    (void)pshared;                              
    sem->value = value;
    sem->queue = malloc(sizeof(struct mutex_queue));
    if (!sem->queue) {
        perror("thread_sem_init");
        return 1;
    }
    TAILQ_INIT(sem->queue);
    return 0;
}

int thread_sem_destroy(thread_sem_t *sem)
{
    free(sem->queue);
    return 0;
}


int thread_sem_wait(thread_sem_t *sem)
{
    struct my_thread *current = TAILQ_FIRST(queue);
    sem->value--;
    if (sem->value < 0) {
        struct mutex_thread *node = malloc(sizeof(struct mutex_thread));
        node->thread = current;
        TAILQ_INSERT_TAIL(sem->queue, node, entries);

        current->state = IS_WAITING;         
        thread_yield();                      
    }
    return 0;
}

int thread_sem_trywait(thread_sem_t *sem)
{
    if (sem->value <= 0)
        return EAGAIN;                        

    sem->value--;
    return 0;
}

int thread_sem_post(thread_sem_t *sem)
{
    sem->value++;

    if (sem->value <= 0) {
        struct mutex_thread *first = TAILQ_FIRST(sem->queue);
        if (first) {
            TAILQ_REMOVE(sem->queue, first, entries);
            first->thread->state = IS_READY;
            TAILQ_INSERT_AFTER(queue,TAILQ_FIRST(queue),first->thread,entries);
            my_queue_size++;
            free(first);
        }
    }
    return 0;
}

// ==========================
// SIGNAL HANDLING SECTION
// ==========================
// ... (functions above)
// Sends a signal to the specified thread
int thread_kill(thread_t tid, int signal) {
    if (signal < 0 || signal >= MAX_SIGNALS || tid == NULL)
        return -1;

    struct my_thread *t = (struct my_thread *)tid;
    t->pending_signals |= (1U << signal);
    return 0;
}

int thread_signal(struct my_thread *current_thread, int signal, void (*handler)(int)) {
    if (signal < 0 || signal >= MAX_SIGNALS || current_thread == NULL)
        return -1;

    current_thread->handlers[signal] = handler;
    return 0;
}


// Deliver all pending signals to the thread (typically called in scheduler)
void deliver_pending_signals(struct my_thread *tcb) {
    for (int sig = 0; sig < MAX_SIGNALS; ++sig) {
        uint32_t mask = (1U << sig);

        if ((tcb->pending_signals & mask) && !(tcb->blocked_signals & mask)) {
            tcb->pending_signals &= ~mask;  // Clear pending signal

            if (tcb->handlers[sig]) {
                tcb->handlers[sig](sig);    // Call handler
            } else {
                if (sig == THREAD_SIG_TERM) {
                    printf("[thread %p] got THREAD_SIG_TERM\n", (void *)tcb);
                    thread_exit(NULL);
                } else {
                    printf("[thread %p] unhandled signal %d\n", (void *)tcb, sig);
                }
            }
        }
    }
}


#endif /* USE_PTHREAD */