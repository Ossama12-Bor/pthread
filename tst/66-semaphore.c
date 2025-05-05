#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "thread.h"

#define POOL 11
#define SIZE 10
#define LOOP 1

/* Opaque function used by the reader that should not be modified */
int
lecture( int *value ) {
    usleep( 10 );
    //    fprintf( stderr, "Je lis\n");
    return *value;
}
    
/* Opaque function used by the writer that should not be modified */
void
ecriture( int v, int *value ) {
    usleep( 1 );
    (*value) += v;
    //    fprintf( stderr, "J'ecris\n");
}
    
int A[SIZE];
thread_sem_t empty;
thread_sem_t filled;
int sum;

thread_mutex_t sum_lock ;
thread_mutex_t read_lock ;
int read_index = 0;

/* Function executed by a reader */
void *
lecteur( void *ptr ) { 
    int j, addval;
    
    while( read_index < SIZE )
    {
        //printf("%d\n", read_index);
	/* Let's wait for an available cell */
	if ( thread_sem_trywait( &filled ) == -1 ) {
	    /* Let's come back in two us */
	    //usleep( 2 );
	    continue;
	}
	
	/**
	 * Multiple cells may have been available, we need to handle
	 * them in order, and make sure no reader is reading twice the
	 * same cell
	 */
	thread_mutex_lock( &read_lock );
	/* Backup current value */
	j = read_index;
	/* Increment shared counter */
	read_index++;
	thread_mutex_unlock( &read_lock );
	/* Read the value only when it is ready */
	addval = lecture( A + j );
	thread_sem_post( &empty );
	/* Increment the shared sum */
	thread_mutex_lock( &sum_lock );
	sum += addval;
	thread_mutex_unlock( &sum_lock );
    }
    printf("J'ai fini de lire\n");
    return NULL; // Equivalent a pthread_exit(NULL) 
}

/* Function executed by a writer */
void *
ecrivain(void *ptr) { 
    int i, j;
    
    for(i=0; i<LOOP; i++) {
        for(j=0; j<SIZE; j++){
	    thread_sem_wait( &empty );
            ecriture( 1, A + j );
	    thread_sem_post( &filled );
        }
    }
    printf("J'ai fini d'ecrire\n");
    return NULL; // Equivalent a pthread_exit(NULL) 
}

int main() {
  thread_t pid[POOL];
  int i;
    thread_mutex_init(&sum_lock);
    thread_mutex_init(&read_lock);
  /* Create the semapĥore the represents the number of empty and filled cells */
  if( thread_sem_init( &filled, 0, 0    ) == -1){
    printf( "sem filled\n" );
    exit( EXIT_FAILURE );
  }
   if (thread_sem_init( &empty,  0, SIZE ) == -1){
    printf( "sem empty\n" );
    exit( EXIT_FAILURE );
   }
  /* Make sure the array is set to 0 */
  memset( A, 0, SIZE * sizeof(int) );
  sum = 0;

  /* Create the threads */
  thread_create(&pid[0], ecrivain, NULL);
  for(i=1; i<POOL; i++){
      thread_create(&pid[i], lecteur,  NULL);
  }

  /* Wait for all the threads to be done */
  for(i=0; i<POOL; i++){
      thread_join(pid[i], NULL);
  }

  printf( "La somme est de %d et devrait être de %d\n",
	  sum, (LOOP * (LOOP+1) / 2 ) * SIZE );

  thread_sem_destroy( &filled );
  thread_sem_destroy( &empty );
    thread_mutex_destroy(&sum_lock);
    thread_mutex_destroy(&read_lock);
  return EXIT_SUCCESS;
}
