#include "list_handler.h"
#include "queue.h"

struct my_thread * first_available(struct my_queue * queue ){
	struct my_thread *var ;
	TAILQ_FOREACH(var, queue, entries){
		if (var->state == IS_READY){
			break;
		}
	}
	return var->state == IS_READY ? var : NULL;
}

struct my_thread * find(struct my_queue * queue , thread_t id) 
{	
	struct my_thread *var ;
	TAILQ_FOREACH(var, queue, entries){
		if (var->id == id){
			break;
		}
	}
	return var->id == id ? var : NULL;
	
};
void head2tail(struct my_queue * queue){
	struct my_thread *var = TAILQ_FIRST(queue) ;
	TAILQ_REMOVE(queue, var, entries);
	TAILQ_INSERT_TAIL(queue,var,entries);
}

int move_availale2head(struct my_queue * queue){
	struct my_thread *var = first_available(queue);
	if (var == NULL){
		return 0;
	}
	TAILQ_REMOVE(queue, var, entries);
	TAILQ_INSERT_HEAD(queue,var,entries);
	return 1;

}