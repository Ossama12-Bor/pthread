#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include "../src/thread.h"

thread_mutex_t lock;
int *tableau ;
int size;
int nb;
static void * thfunc(void *dummy)
{
    int pas = size/nb;
    int tmp = (__intptr_t)dummy;
    int s = 0;
    for(int i=0;i<pas;i++){
        s+= tableau[pas*tmp +i];
    }
    if(tmp < (size -pas*nb)){
        s+=tableau[pas*nb + tmp];
    }

    return (void *)(__intptr_t) s;
}

int main(int argc, char *argv[])
{   
    thread_t *th;
    int err, i;
    struct timeval tv1, tv2;
    unsigned long us;
    void * rs;
    int sum = 0 ;

    if (argc < 3) {
        printf("argument manquant: nombre de threads et taille du tableau\n");
        return -1;
    }

    nb = atoi(argv[1]);
    size =  atoi(argv[2]);

    if (thread_mutex_init(&lock) != 0) {
        fprintf(stderr, "thread_mutex_init failed\n");
        return -1;
    }

    th = malloc(nb*sizeof(*th));
    if (!th) {
        perror("malloc");
        return -1;
    }
    tableau = malloc(sizeof(int)*size);
    if (!tableau) {
        perror("malloc");
        return -1;
    }

    for (i = 0;i< size;i++){
        tableau[i] = i;
    }
    gettimeofday(&tv1, NULL);
    /* on cree tous les threads */
    for(i=0; i<nb; i++) {
        err = thread_create(&th[i], thfunc, (void*)(__intptr_t) i);
        assert(!err);
    }

    for(i=0; i<nb; i++) {
        thread_mutex_lock(&lock);
        err = thread_join(th[i],&rs);
        assert(!err);
        sum +=(__intptr_t)rs;
        thread_mutex_unlock(&lock);
    }
    gettimeofday(&tv2, NULL);

    free(th);
    free(tableau);
    thread_mutex_destroy(&lock);

    us = (tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec);

    if ( sum == (size)*(size-1)/2 ) {
        printf("La somme a été correctement calculée %d et la taille %d\n", sum,size);
        printf("le temps en us%ld\n",us);
        return EXIT_SUCCESS;
    }
    else {
        printf("sum %d\n",sum);
        return EXIT_FAILURE;
    }
    }
