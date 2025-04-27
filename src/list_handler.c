#include "list_handler.h"
#include "queue.h"
#include <valgrind/valgrind.h>

thread_t main_id_thread;



void head2tail(struct my_queue * queue){
	struct my_thread *var = TAILQ_FIRST(queue) ;
	TAILQ_REMOVE(queue, var, entries);
	TAILQ_INSERT_TAIL(queue,var,entries);
}



void queue_free(struct my_queue **queue){
	if(queue!=NULL){
	struct my_thread *var,*tmp ;

	TAILQ_FOREACH_SAFE(var, *queue, entries,tmp){
		if(var != main_id_thread){
			VALGRIND_STACK_DEREGISTER(var->valgrind_stackid);
			free(var->uc.uc_stack.ss_sp);
	
		}
		free(var);
		}
	
	free(*queue);
	}
}
