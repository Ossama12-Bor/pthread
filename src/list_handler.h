#ifndef LIST_HANDLER__H
#define LIST_HANDLER__H
#include "thread.h"

void  head2tail(struct my_queue * queue);


struct my_thread *  first_available(struct my_queue * queue );

int move_availale2head(struct my_queue * queue); // return 0 if no elem available 
#endif