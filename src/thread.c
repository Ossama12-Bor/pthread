#include "thread.h"

#ifndef USE_PTHREAD

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
    // Implémentation ici
}

thread_t thread_self(void) {
    // Implémentation ici
}

int thread_yield(void) {
    // Implémentation ici
}

int thread_join(thread_t thread, void **retval) {
    // Implémentation ici
}

void thread_exit(void *retval) {
    // Implémentation ici
}

int thread_mutex_init(thread_mutex_t *mutex) {
    // Implémentation ici
}

int thread_mutex_destroy(thread_mutex_t *mutex) {
    // Implémentation ici
}

int thread_mutex_lock(thread_mutex_t *mutex) {
    // Implémentation ici
}

int thread_mutex_unlock(thread_mutex_t *mutex) {
    // Implémentation ici
}

#endif /* USE_PTHREAD */
