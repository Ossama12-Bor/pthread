#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include "../src/thread.h"

thread_mutex_t lock;
int *array;
int size;
int nb;

static int cmp_int(const void *a, const void *b)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);
}

static void *thfunc(void *arg)
{
    int id   = (int)(__intptr_t)arg;
    int pas  = size / nb;
    int deb  = id * pas;
    int fin  = (id == nb - 1) ? size : deb + pas;
    qsort(array + deb, fin - deb, sizeof(int), cmp_int);
    return NULL;
}

static void merge_segments(int start, int mid, int end, int *tmp)
{
    int i = start, j = mid, k = 0;
    while (i < mid && j < end)
        tmp[k++] = (array[i] <= array[j]) ? array[i++] : array[j++];
    while (i < mid) tmp[k++] = array[i++];
    while (j < end) tmp[k++] = array[j++];
    memcpy(array + start, tmp, (end - start) * sizeof(int));
}

static int is_sorted(void)
{
    for (int i = 1; i < size; ++i)
        if (array[i - 1] > array[i]) return 0;
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "argument manquant : <nb_threads> <taille_tableau>\n");
        return EXIT_FAILURE;
    }

    nb   = atoi(argv[1]);
    size = atoi(argv[2]);

    if (thread_mutex_init(&lock) != 0) {
        perror("thread_mutex_init");
        return EXIT_FAILURE;
    }

    array = malloc(size * sizeof(int));
    assert(array);

    srand(42);
    for (int i = 0; i < size; ++i)
        array[i] = rand();

    thread_t *th = malloc(nb * sizeof(*th));
    assert(th);

    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);

    for (int i = 0; i < nb; ++i) {
        int err = thread_create(&th[i], thfunc, (void *)(__intptr_t)i);
        assert(!err);
    }

    for (int i = 0; i < nb; ++i) {
        void *rs;
        thread_mutex_lock(&lock);
        int err = thread_join(th[i], &rs);
        assert(!err);
        (void)rs;
        thread_mutex_unlock(&lock);
    }

    int pas  = size / nb;
    int *tmp = malloc(size * sizeof(int));
    assert(tmp);

    int seg_size = pas;
    while (seg_size < size) {
        for (int start = 0; start + seg_size < size; start += 2 * seg_size) {
            int mid = start + seg_size;
            int end = (mid + seg_size < size) ? mid + seg_size : size;
            merge_segments(start, mid, end, tmp);
        }
        seg_size *= 2;
    }

    free(tmp);

    gettimeofday(&tv2, NULL);
    unsigned long us = (tv2.tv_sec - tv1.tv_sec) * 1000000UL
                     + (tv2.tv_usec - tv1.tv_usec);

    int ok = is_sorted();
    free(array);
    free(th);
    thread_mutex_destroy(&lock);

    if (ok) {
        printf("Le tri a réussi pour %d éléments (%lu µs)\n", size, us);
        printf("GRAPH;%d;%lu;%e;SUCCESS\n", size, 0UL, us / 1e6);
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "ERREUR : le tableau n’est pas trié !\n");
        printf("GRAPH;%d;%lu;%e;FAILED\n", size, 0UL, us / 1e6);
        return EXIT_FAILURE;
    }
}
