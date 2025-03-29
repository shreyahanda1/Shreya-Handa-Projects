#include "rwlock.h"
#include <stdlib.h>
#include <pthread.h>

// defines reader-writer lock strucure
typedef struct rwlock {
    pthread_mutex_t lock;

    pthread_cond_t check;

    int active_readers;
    int waiting_readers;

    int active_writers;
    int waiting_writers;

    PRIORITY priority;
    uint32_t n_way;

    uint32_t readers;
    uint32_t writers;
} rwlock_t;

// initializes reader-writer lock and its components. sets counters to 0 and initializes mutex and cv.
rwlock_t *rwlock_new(PRIORITY p, uint32_t n) {
    rwlock_t *rw = malloc(sizeof(rwlock_t));
    if (rw == NULL)
        return NULL;

    pthread_mutex_init(&rw->lock, NULL);
    pthread_cond_init(&rw->check, NULL);

    rw->active_readers = 0;
    rw->waiting_readers = 0;

    rw->active_writers = 0;
    rw->waiting_writers = 0;

    rw->priority = p;
    rw->n_way = n;

    rw->readers = 0;
    rw->writers = 0;
    return rw;
}

// deletes reader-writer lock by destroying mutex and cv
void rwlock_delete(rwlock_t **rw) {
    if (rw && *rw) {
        pthread_mutex_destroy(&(*rw)->lock);
        pthread_cond_destroy(&(*rw)->check);
        free(*rw);
        *rw = NULL;
    }
}

void reader_lock(rwlock_t *rw) {
    // locks the mutex so no other threads can access simultaneously
    pthread_mutex_lock(&rw->lock);

    // increments waiting_readers to show one reader is waiting
    rw->waiting_readers++;

    // waits until the following conditions are met before it proceeds
    // checks if there is already an active writer. If there is, waits as there shouldn't be any reading and writing happening simultaneously
    // checks who has priority. If writers have priority and there are writers that are waiting, wait until that job is completed
    // checks if priority is N-WAY, if the number of read operations since the last write has reached the n_way limit, and if there are writers wating. If there is, waits and allows write to do its job first
    while ((rw->active_writers > 0) || (rw->priority == WRITERS && rw->waiting_writers > 0)
           || (rw->priority == N_WAY && rw->readers >= rw->n_way && rw->waiting_writers > 0)) {
        pthread_cond_wait(&rw->check, &rw->lock);
    }

    // after the above conditions have been satisfied, the waiting readers can begin their job
    // decrements waiting_readers to show one less reader is waiting
    rw->waiting_readers--;

    // increments active_readers to show one more reader is active and is accessing shared resource
    rw->active_readers++;

    // increments reader count
    rw->readers++;

    // unlocks and allows other threads to begin
    pthread_mutex_unlock(&rw->lock);
}

// reader thread has completed
void reader_unlock(rwlock_t *rw) {
    // locks the mutex so no other threads can access simultaneously
    pthread_mutex_lock(&rw->lock);

    // decrements active_readers to show one less reader is active and is accessing shared resource
    rw->active_readers--;

    // signals other waiting threads
    pthread_cond_broadcast(&rw->check);

    // unlocks and allow other threads to begin
    pthread_mutex_unlock(&rw->lock);
}

void writer_lock(rwlock_t *rw) {
    // locks the mutex so no other threads can access simultaneously
    pthread_mutex_lock(&rw->lock);

    // increments waiting_readers to show one writer is waiting
    rw->waiting_writers++;

    // waits until the following conditions are met before it proceeds
    // checks if there is already an active reader. If there is, waits until that job is completed to ensure no reading and writing happen simultaneously
    // checks if there is already an active writer. If there is, waits until that job is completed before another write. Mutual exclusion.
    // checks if priority is N-WAY, if the number of read operations since the last write has reached the n_way limit, and if there are writers wating. If there is, waits and allows write to do its job first
    while ((rw->active_readers > 0) || (rw->active_writers > 0)
           || (rw->priority == N_WAY && rw->readers < rw->n_way && rw->waiting_readers > 0)) {
        pthread_cond_wait(&rw->check, &rw->lock);
    }

    // after the above conditions are met, the waiting writers  can begin their job
    // decrements waiting_writers to show one less writer is waiting
    rw->waiting_writers--;

    // increments active_readers to show one more reader is active and is accessing shared resource
    rw->active_writers++;

    // increments writer count
    rw->writers++;

    // unlocks and allow other threads to begin
    pthread_mutex_unlock(&rw->lock);
}

void writer_unlock(rwlock_t *rw) {
    // locks the mutex so no other threads can access simultaneously
    pthread_mutex_lock(&rw->lock);

    // decrements active_writerss to show one less writer is active and is accessing shared resource
    rw->active_writers--;

    // signals other waiting threads
    pthread_cond_broadcast(&rw->check);

    // unlocks and allow other threads to begin
    pthread_mutex_unlock(&rw->lock);
}