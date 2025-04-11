#include "list_handler.h"
#include "queue.h"
#include <valgrind/valgrind.h>

thread_t main_id_thread;

struct my_thread * first_available(struct my_queue * queue ){
	if(queue != NULL){
		struct my_thread *var,*tmp ;
		TAILQ_FOREACH_SAFE(var, queue, entries,tmp){
			if (var->state == IS_READY){
				return var;
			}
		}
	}
	return  NULL;
}

/*struct my_thread * find(struct my_queue * queue , thread_t id) 
{   
    struct my_thread *var = NULL;
    TAILQ_FOREACH(var, queue, entries){  
        if (var->id == id){
            return var;
        }
    }
    return NULL;
}*/

int queue_size(struct my_queue * queue) 
{	int c=0;
	struct my_thread *var,*tmp ;
	TAILQ_FOREACH_SAFE(var, queue, entries,tmp){
		c++;
		}
	return c;
	
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
	*queue = NULL;
}

int dead_lock(struct my_queue *queue)
{	
	int c = 0;
	struct my_thread *var,*tmp ;
	TAILQ_FOREACH_SAFE(var, queue, entries,tmp){
		if(var->locker == 1)
			c++;
		}
	return c;
	
};