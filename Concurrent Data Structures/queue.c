#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include "queue.h"

typedef struct queue {
    void **elements;
    int size;
    int in;
    int out;
    sem_t mutex, filled, empty;
} queue_t;

// creates the queue
queue_t *queue_new(int size) {
    queue_t *q = malloc(sizeof(queue_t));
    if (q == NULL) {
        return NULL;
    }

    // alocates space for elements
    typedef void *element_ptr;
    q->elements = malloc(sizeof(element_ptr) * size);
    if (q->elements == NULL) {
        free(q);
        return NULL;
    }

    q->size = size;
    q->in = 0; // index to insert the next element
    q->out = 0; // index from which to remove the next element

    sem_init(&q->mutex, 0, 1);
    sem_init(&q->filled, 0, 0); // how many slots are filled in the beginning
    sem_init(&q->empty, 0, size); // how many slots are empty in the beginning

    return q;
}

// deleting the queue
void queue_delete(queue_t **q) {
    if (q && *q) {
        free((*q)->elements);
        sem_destroy(&(*q)->mutex);
        sem_destroy(&(*q)->filled);
        sem_destroy(&(*q)->empty);
        free(*q);
        *q = NULL;
    }
}

// should block if queue is full
bool queue_push(queue_t *q, void *elem) {
    if (q == NULL) {
        return false;
    }

    sem_wait(&q->empty); // wait if queue is filled and signal pop
    sem_wait(&q->mutex); // lock to account for other threads

    q->elements[q->in] = elem; // places the new element at the position indexed by q+in
    q->in = (q->in + 1) % q->size;

    sem_post(&q->mutex);
    sem_post(&q->filled);

    return true;
}

// should block if queue is empty
bool queue_pop(queue_t *q, void **elem) {
    if (q == NULL || elem == NULL) {
        return false;
    }

    sem_wait(&q->filled); // wait if queue is empty
    sem_wait(&q->mutex); // lock

    *elem = q->elements[q->out];
    q->out = (q->out + 1) % q->size;

    sem_post(&q->mutex); // unlock
    sem_post(&q->empty);

    return true;
}
